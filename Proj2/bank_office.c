#include "bank_office.h"

bank_account_t bank_accounts[MAX_BANK_ACCOUNTS];
int current_num_accounts = 0;

void *process_order(void* arg)
{
    (void) arg;
    int server_fifo_fd;
    tlv_request_t tlv_request;
    tlv_reply_t tlv_reply;

    if ( (server_fifo_fd = open(SERVER_FIFO_PATH, O_RDONLY)) == -1) {
        printf("Error opening server fifo.\n");
        //return
    }

    read(server_fifo_fd, &tlv_request, sizeof(tlv_request));

    tlv_reply.type = tlv_request.type;
    tlv_reply.value.header.account_id = tlv_request.value.header.account_id;

    if( authenthicate_user(tlv_request.value.header)) {
        tlv_reply.value.header.ret_code = RC_LOGIN_FAIL;
        tlv_reply.length = sizeof(tlv_reply.value.header);
        write_response(tlv_request.value.header.pid, tlv_reply); 
    }
    
    switch(tlv_request.type) {
        case OP_CREATE_ACCOUNT: {
            if(tlv_request.value.header.account_id != ADMIN_ACCOUNT_ID) {
                tlv_reply.value.header.ret_code = RC_OP_NALLOW;
                tlv_reply.length = sizeof(tlv_reply.value.header);
                break;
            }
            tlv_reply.value.header.ret_code = create_account(tlv_request.value.create);
            break;
        }
        case OP_BALANCE: {
            if(tlv_request.value.header.account_id == ADMIN_ACCOUNT_ID) {
                tlv_reply.value.header.ret_code = RC_OP_NALLOW;
                tlv_reply.length = sizeof(tlv_reply.value.header); 
                break;
            }
            tlv_reply.value.header.ret_code = balance_inquiry(tlv_request.value.header, &tlv_reply.value.balance);
            break;
        }
        case OP_TRANSFER: {
            if(tlv_request.value.header.account_id == ADMIN_ACCOUNT_ID) {
                tlv_reply.value.header.ret_code = RC_OP_NALLOW;
                tlv_reply.length = sizeof(tlv_reply.value.header); 
                break;
            }
            tlv_reply.value.header.ret_code = transfer(tlv_request.value.transfer, &tlv_reply.value.transfer);
            break;
        }
        case OP_SHUTDOWN: {
            if(tlv_request.value.header.account_id != ADMIN_ACCOUNT_ID) {
                tlv_reply.value.header.ret_code = RC_OP_NALLOW;
                tlv_reply.length = sizeof(tlv_reply.value.header);
                break;
            }
            tlv_reply.value.header.ret_code = shutdown(&tlv_reply.value.shutdown);
            break;
        }
    }

    write_response(tlv_request.value.header.pid, tlv_reply);
    return NULL;
}

int authenthicate_user(req_header_t req_header)
{
    //return 1; if login failed

    return 0;
}

ret_code_t create_account(req_create_account_t req_create_account)
{
    bank_accounts[current_num_accounts].account_id = req_create_account.account_id;
    bank_accounts[current_num_accounts].balance = req_create_account.balance;
    
    if (getHash(req_create_account.password, bank_accounts[current_num_accounts].hash)) return 1;

    //Get salt

    current_num_accounts++;
    return 0;
}

int getHash(char* password, char* hash)
{
    int fd[2], n;
    pid_t pid;

    pipe(fd);

    pid = fork();

    if (pid > 0) {
        close(fd[1]);
        n = read(fd[0], hash, 64);
    }
    else {
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        execl("./sha256sum", "sha256sum", password);
        return 1;
    }

}

ret_code_t balance_inquiry(req_header_t req_header, rep_balance_t *rep_balance)
{

}

ret_code_t transfer(req_transfer_t req_transfer, rep_transfer_t *rep_transfer)
{

}

ret_code_t shutdown(rep_shutdown_t *rep_shutdown)
{

}

int write_response(pid_t user_pid, tlv_reply_t tlv_reply)
{
    char user_fifo_path[USER_FIFO_PATH_LEN];
    int fifo_fd;

    sprintf(user_fifo_path, "%s%d", USER_FIFO_PATH_PREFIX, (int) user_pid);
    mkfifo(user_fifo_path, OPEN_FIFO_PERMISSIONS);

    if ( (fifo_fd = open(user_fifo_path, O_WRONLY)) == -1) {
        printf("Error opening user fifo.\n");
        return 1;
    }

    write(fifo_fd, &tlv_reply, tlv_reply.length);

    return 0;
}

