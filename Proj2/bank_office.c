#include "bank_office.h"

bank_account_t bank_accounts[MAX_BANK_ACCOUNTS];
int current_num_accounts = 0;

extern int log_file_des;

void *process_order(void* arg)
{
    (void) arg;
    tlv_request_t tlv_request;
    tlv_reply_t tlv_reply;

    get_request(&tlv_request);
    logRequest(log_file_des, pthread_self(), &tlv_request);

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
            tlv_reply.length = sizeof(tlv_reply.value.header);
            break;
        }
        case OP_BALANCE: {
            if(tlv_request.value.header.account_id == ADMIN_ACCOUNT_ID) {
                tlv_reply.value.header.ret_code = RC_OP_NALLOW;
                tlv_reply.length = sizeof(tlv_reply.value.header); 
                break;
            }
            tlv_reply.value.header.ret_code = balance_inquiry(tlv_request.value.header, &tlv_reply.value.balance);
            tlv_reply.length = sizeof(tlv_reply.value.header) + sizeof(tlv_reply.value.balance);
            break;
        }
        case OP_TRANSFER: {
            if(tlv_request.value.header.account_id == ADMIN_ACCOUNT_ID) {
                tlv_reply.value.header.ret_code = RC_OP_NALLOW;
                tlv_reply.length = sizeof(tlv_reply.value.header); 
                break;
            }
            tlv_reply.value.header.ret_code = transfer(tlv_request.value.transfer, &tlv_reply.value.transfer);
            tlv_reply.length = sizeof(tlv_reply.value.header) + sizeof(tlv_reply.value.transfer);
            break;
        }
        case OP_SHUTDOWN: {
            if(tlv_request.value.header.account_id != ADMIN_ACCOUNT_ID) {
                tlv_reply.value.header.ret_code = RC_OP_NALLOW;
                tlv_reply.length = sizeof(tlv_reply.value.header);
                break;
            }
            tlv_reply.value.header.ret_code = shutdown(&tlv_reply.value.shutdown);
            tlv_reply.length = sizeof(tlv_reply.value.header) + sizeof(tlv_reply.value.shutdown);
            break;
        }
        default: {

            break;
        }
    }

    if(write_response(tlv_request.value.header.pid, tlv_reply)) return (void*) 1;


    return NULL;
}

int authenthicate_user(req_header_t req_header)
{
    //return 1; if login failed

    (void) req_header;
    return 0;
}

ret_code_t create_account(req_create_account_t req_create_account)
{
    ret_code_t ret_code = RC_OK;

    bank_accounts[current_num_accounts].account_id = req_create_account.account_id;
    bank_accounts[current_num_accounts].balance = req_create_account.balance;

    getSalt(bank_accounts[current_num_accounts].salt);
    
    if (getHash(req_create_account.password, bank_accounts[current_num_accounts].salt, bank_accounts[current_num_accounts].hash)) return 1;

    current_num_accounts++;

    if(ret_code == RC_OK) {
        logAccountCreation(log_file_des, pthread_self(), &bank_accounts[current_num_accounts]);
    }
        
    return RC_OK;
}

int getHash(char* password, char* salt, char* hash)
{
    int fd1[2], fd2[2], stdout_copy, stdin_copy, status;
    pid_t pid1, pid2;
    char stringToHash[strlen(password) + SALT_LEN + 1];
    sprintf(stringToHash, "%s%s", password, salt);

    if (pipe(fd1) != 0) {
        perror("Error creating pipe1");
        return 1;
    }

    if (pipe(fd2) != 0) {
        perror("Error creating pipe2");
        return 1;
    }

    stdout_copy = dup(STDOUT_FILENO);
    stdin_copy = dup(STDIN_FILENO);

    pid1 = fork();
    if (pid1 > 0) {
        close(fd1[WRITE]);

        wait(&status);
        if(WEXITSTATUS(status) == 1) return 1;
    }
    else if (pid1 == 0) {
        close(fd1[READ]);

        pid2 = fork();
        if(pid2 > 0) {
            close(fd2[WRITE]);

            wait(&status);
            if(WEXITSTATUS(status) == 1) exit(1);
        }
        else if(pid2 == 0) {
            close(fd2[READ]);

            dup2(fd2[WRITE], STDOUT_FILENO);
            execlp("echo", "echo", "-n", stringToHash, NULL);
            perror("Error obtaining hash");
            exit(1);
        }
        else {
            perror("Error doing fork");
            exit(1);
        }
        dup2(fd1[WRITE], STDOUT_FILENO);
        dup2(fd2[READ], STDIN_FILENO);
        execlp("sha256sum", "sha256sum", NULL);
        perror("Error obtaining hash");
        exit(1);
    }
    else {
        perror("Error doing fork");
        return 1;
    }

    dup2(stdout_copy, STDOUT_FILENO);
    dup2(stdin_copy, STDIN_FILENO);
    close(stdout_copy);
    close(stdin_copy);

    if (read(fd1[READ], hash, HASH_LEN) == 0) {
        perror("Error reading hash");
        return 1;    
    }

    printf("%s", hash);

    return 0;
}

ret_code_t balance_inquiry(req_header_t req_header, rep_balance_t *rep_balance)
{
    (void) req_header;
    (void) rep_balance;


    return RC_OK;
}

ret_code_t transfer(req_transfer_t req_transfer, rep_transfer_t *rep_transfer)
{
    (void) req_transfer;
    (void) rep_transfer;

    
    return RC_OK;
}

ret_code_t shutdown(rep_shutdown_t *rep_shutdown)
{
    (void) rep_shutdown;


    
    return RC_OK;
}

int write_response(pid_t user_pid, tlv_reply_t tlv_reply)
{
    char user_fifo_path[USER_FIFO_PATH_LEN];
    int fifo_fd;

    sprintf(user_fifo_path, "%s%d", USER_FIFO_PATH_PREFIX, (int) user_pid);

    if ( (fifo_fd = open(user_fifo_path, O_WRONLY)) == -1) {
        printf("Error opening user fifo.\n");
        return 1;
    }

    write(fifo_fd, &tlv_reply, tlv_reply.length);
    logReply(log_file_des, pthread_self(), &tlv_reply);

    return 0;
}

void getSalt(char* salt)
{
    int salt_num = 0x0;

    for(int i = 0; i < SALT_LEN; i += 8) {
        salt_num |= rand() & i;
    }

    sprintf(salt, "%d", salt_num);
}
