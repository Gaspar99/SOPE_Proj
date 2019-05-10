#include "bank_office.h"

bank_account_t bank_accounts[MAX_BANK_ACCOUNTS];
int current_num_accounts = 0;

extern int log_file_des;

void *process_order(void* arg)
{
    (void) arg;
    int server_fifo_fd;
    tlv_request_t tlv_request;
    tlv_reply_t tlv_reply;

    get_request(&tlv_request);

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
    }

    if(write_response(tlv_request.value.header.pid, tlv_reply)) return (void*) 1;


    return NULL;
}

int authenthicate_user(req_header_t req_header)
{
    int account_index;
    char* hash;

    if(account_index = get_account_index(req_header.account_id) == -1) return 1;
    if(getHash(req_header.password, bank_accounts[account_index].salt, hash)) return 1;

    if(hash == bank_accounts[account_index].hash)
        return 0;
    else
        return 1;
}

ret_code_t create_account(req_create_account_t req_create_account)
{
    if(get_account_index(req_create_account.account_id) != -1) return 1;

    bank_accounts[current_num_accounts].account_id = req_create_account.account_id;
    bank_accounts[current_num_accounts].balance = req_create_account.balance;

    if(getSalt(bank_accounts[current_num_accounts].salt)) return 1;
    
    if(getHash(req_create_account.password, bank_accounts[current_num_accounts].hash, bank_accounts[current_num_accounts].salt)) return 1;

    current_num_accounts++;
    return 0;
}

int getHash(char* password, char* salt, char* hash)
{
    int fd[2], stdout_copy, status;
    pid_t pid;
    char stringToHash[strlen(password) + 1];
    sprintf(stringToHash, "%s%s", password, salt);

    if (pipe(fd) != 0) {
        perror("Error creating pipe");
        return 1;
    }

    stdout_copy = dup(STDOUT_FILENO);

    pid = fork();
    if (pid > 0) {
        close(fd[WRITE]);
        
        if (read(fd[READ], hash, HASH_LEN) == 0) {
            perror("Error reading hash");
            return 1;
        }

        wait(&status);
        if(WEXITSTATUS(status) == 1) return 1;
    }
    else if (pid == 0) {
        close(fd[READ]);
        dup2(fd[WRITE], STDOUT_FILENO);
        execlp("sha256sum", "sha256sum", stringToHash, NULL);
        perror("Error obtaining hash");
        exit(1);
    }
    else {
        perror("Error doing fork");
        return 1;
    }

    dup2(stdout_copy, STDOUT_FILENO);
    close(stdout_copy);

    return 0;
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

    if ( (fifo_fd = open(user_fifo_path, O_WRONLY)) == -1) {
        printf("Error opening user fifo.\n");
        return 1;
    }

    write(fifo_fd, &tlv_reply, tlv_reply.length);

    return 0;
}

int getSalt(char* salt)
{
    int salt_num = 0x0;

    for(int i = 0; i < SALT_LEN; i += 8) {
        salt_num |= rand() & i;
    }

    return sprintf(salt, "%d", salt_num);
}

int get_account_index(uint32_t account_id)
{
    for (int i = 0; i < current_num_accounts; i++) {
        if (bank_accounts[i].account_id == account_id)
            return i;
    }

    return -1;
}
