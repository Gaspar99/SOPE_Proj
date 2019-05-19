#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#include "constants.h"
#include "types.h"
#include "sope.h"

#include "tlv_request_filler.h"

bool sigalarm = false;

void sigalarm_handler() {
    sigalarm = true;
}

int main(int argc, char* argv[])
{
    tlv_request_t tlv_request;
    tlv_reply_t tlv_reply;
    pid_t user_pid = getpid();
    int server_fifo_fd, user_fifo_fd, log_file_fd;
    char user_fifo_path[USER_FIFO_PATH_LEN];
    
    if(argc != 6)
    {
        printf("Usage: %s <account_id> <password> <op_delay> <op_code> <op_args>\n", argv[0]);
        return 1;
    }

    if( (log_file_fd = open(USER_LOGFILE, O_WRONLY | O_CREAT | O_APPEND | O_TRUNC, LOG_FILE_PERMISSIONS)) == -1 ) {
        printf("Error: could not open user log file.\n");
        return 1;
    }

    sprintf(user_fifo_path, "%s%d", USER_FIFO_PATH_PREFIX, user_pid);
    if( mkfifo(user_fifo_path, OPEN_FIFO_PERMISSIONS) ) {
        printf("Error creating user fifo.\n");
        return 1;
    }

    if(get_tvl_request(argv, &tlv_request)) return 1;
    logRequest(log_file_fd, user_pid, &tlv_request);

    struct sigaction action;
    action.sa_handler = sigalarm_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGALRM, &action, NULL);

    //Open server fifo to write tlv request
    if ( (server_fifo_fd = open(SERVER_FIFO_PATH, O_WRONLY)) == -1) { //Server down
        tlv_reply.type = tlv_request.type;
        tlv_reply.value.header.account_id = tlv_request.value.header.account_id;
        tlv_reply.value.header.ret_code = RC_SRV_DOWN;
        tlv_reply.length = sizeof(tlv_reply.value.header);

        switch(tlv_reply.type) {
            case OP_BALANCE: {
                tlv_reply.value.balance.balance = 0;
                break;
            }
            case OP_TRANSFER: {
                tlv_reply.value.transfer.balance = 0;
                break;
            }
            case OP_SHUTDOWN: {
                tlv_reply.value.shutdown.active_offices = 0;
                break;
            }
            default: {
                break;
            }
        }
    }
    else {
        write(server_fifo_fd, &tlv_request.type, sizeof(op_type_t));
        write(server_fifo_fd, &tlv_request.length, sizeof(uint32_t));
        write(server_fifo_fd, &tlv_request.value, tlv_request.length);

        close(server_fifo_fd);

        alarm(FIFO_TIMEOUT_SECS);

        //Open user fifo to read response
        if ( (user_fifo_fd = open(user_fifo_path, O_RDONLY)) == -1) {
            if(sigalarm) {
                tlv_reply.type = tlv_request.type;
                tlv_reply.value.header.account_id = tlv_request.value.header.account_id;
                tlv_reply.value.header.ret_code = RC_SRV_TIMEOUT;
                tlv_reply.length = sizeof(tlv_reply.value.header);
                goto EXIT;
            }
            else {
                printf("Error opening user fifo.\n");
                return 1;
            }
        }
        alarm(0);
        read(user_fifo_fd, &tlv_reply.type, sizeof(op_type_t));
        read(user_fifo_fd, &tlv_reply.length, sizeof(uint32_t));
        read(user_fifo_fd, &tlv_reply.value, tlv_reply.length);
    }

    EXIT: logReply(log_file_fd, user_pid, &tlv_reply);

    close(log_file_fd);
    close(user_fifo_fd);
    unlink(user_fifo_path);
    
    return 0;
}


