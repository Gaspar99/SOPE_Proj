#include "bank_operations.h"

bank_account_t bank_accounts[MAX_BANK_ACCOUNTS];
int current_num_accounts = 0;
pthread_mutex_t bank_accounts_lock = PTHREAD_MUTEX_INITIALIZER;

int check_permissions(tlv_request_t tlv_request)
{
    switch(tlv_request.type) {
        case OP_CREATE_ACCOUNT: 
            return (tlv_request.value.header.account_id != ADMIN_ACCOUNT_ID);
        case OP_BALANCE: 
            return (tlv_request.value.header.account_id == ADMIN_ACCOUNT_ID);
        case OP_TRANSFER: 
            return (tlv_request.value.header.account_id == ADMIN_ACCOUNT_ID);
        case OP_SHUTDOWN:
            return (tlv_request.value.header.account_id != ADMIN_ACCOUNT_ID);
        default:
            return 1;
    }
}

ret_code_t authenthicate_user(req_header_t req_header)
{
    int account_index;
    char hash[HASH_LEN + 1];
    ret_code_t ret_code = RC_OK;

    account_index = get_account_index(req_header.account_id);

    if(getHash(req_header.password, bank_accounts[account_index].salt, hash)) ret_code = RC_OTHER;
    if( account_index == -1) ret_code = RC_ID_NOT_FOUND;
    if(strcmp(hash, bank_accounts[account_index].hash)) ret_code = RC_LOGIN_FAIL;

    return ret_code;
}

ret_code_t create_account(req_create_account_t req_create_account, int bank_office_id, uint32_t op_delay)
{
    bank_account_t bank_account;
    char hash[HASH_LEN + 1];
    char salt[SALT_LEN + 1];
    ret_code_t ret_code = RC_OK;

    logSyncMech(get_log_file_des(), bank_office_id, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_ACCOUNT, req_create_account.account_id);
    pthread_mutex_lock(&bank_accounts_lock);

    logSyncDelay(get_log_file_des(), bank_office_id, req_create_account.account_id, op_delay);
    usleep(op_delay * 1000);

    getSalt(salt);
    if (getHash(req_create_account.password, salt, hash)) ret_code = RC_OTHER;
    if(get_account_index(req_create_account.account_id) != -1) ret_code = RC_ID_IN_USE;

    if(ret_code == RC_OK) {
        bank_account.account_id = req_create_account.account_id;
        bank_account.balance = req_create_account.balance;
        strcpy(bank_account.hash, hash);
        strcpy(bank_account.salt, salt);

        bank_accounts[current_num_accounts++] = bank_account;
        logAccountCreation(get_log_file_des(), bank_office_id, &bank_account);
    }

    pthread_mutex_unlock(&bank_accounts_lock);
    logSyncMech(get_log_file_des(), bank_office_id, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_ACCOUNT, req_create_account.account_id);

    return ret_code;
}

ret_code_t balance_inquiry(req_header_t req_header, rep_balance_t *rep_balance, int bank_office_id)
{
    ret_code_t ret_code = RC_OK;
    int account_index;

    logSyncMech(get_log_file_des(), bank_office_id, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_ACCOUNT, req_header.account_id);
    pthread_mutex_lock(&bank_accounts_lock);

    logSyncDelay(get_log_file_des(), bank_office_id, req_header.account_id, req_header.op_delay_ms);
    usleep(req_header.op_delay_ms * 1000);

    account_index = get_account_index(req_header.account_id);
    if(account_index == -1) ret_code = RC_ID_NOT_FOUND;
    rep_balance->balance = bank_accounts[account_index].balance;

    pthread_mutex_unlock(&bank_accounts_lock);
    logSyncMech(get_log_file_des(), bank_office_id, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_ACCOUNT, req_header.account_id);

    return ret_code;
}

ret_code_t transfer(req_header_t req_header, req_transfer_t req_transfer, rep_transfer_t *rep_transfer, int bank_office_id)
{   
    ret_code_t ret_code = RC_OK;
    int account1_index, account2_index;
    int balance1, balance2;

    logSyncMech(get_log_file_des(), bank_office_id, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_ACCOUNT, req_header.account_id);
    pthread_mutex_lock(&bank_accounts_lock);

    logSyncDelay(get_log_file_des(), bank_office_id, req_header.account_id, req_header.op_delay_ms);
    usleep(req_header.op_delay_ms * 1000);

    if(req_header.account_id == req_transfer.account_id) ret_code = RC_SAME_ID;
    if( (account1_index = get_account_index(req_header.account_id)) == -1) ret_code = RC_ID_NOT_FOUND;
    if( (account2_index = get_account_index(req_transfer.account_id)) == -1) ret_code = RC_ID_NOT_FOUND;

    balance1 = bank_accounts[account1_index].balance - req_transfer.amount;
    balance2 = bank_accounts[account2_index].balance + req_transfer.amount;

    if(balance2 > (int) MAX_BALANCE) ret_code = RC_TOO_HIGH;
    if(balance1 < (int) MIN_BALANCE) ret_code = RC_NO_FUNDS;

    if(ret_code == RC_OK) {
        bank_accounts[account1_index].balance = balance1;
        bank_accounts[account2_index].balance = balance2;
        rep_transfer->balance = balance1;
    }
    else {
        rep_transfer->balance = req_transfer.amount;
    }
    
    pthread_mutex_unlock(&bank_accounts_lock);
    logSyncMech(get_log_file_des(), bank_office_id, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_ACCOUNT, req_header.account_id);
    
    return ret_code;
}

ret_code_t shutdown(rep_shutdown_t *rep_shutdown, int active_offices, int bank_office_id, int server_fifo, uint32_t op_delay)
{
    logSyncDelay(get_log_file_des(), bank_office_id, MAIN_THREAD_ID, op_delay);
    usleep(op_delay * 1000);

    if( chmod(SERVER_FIFO_PATH, READ_ONLY_PERMISSIONS)) return RC_OTHER;

    //Change server fifo flags to not block
    int flags = fcntl(server_fifo, F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(server_fifo, F_SETFL, flags);

    kill(getpid(), SIGUSR1); //To unblock main thread from read

    rep_shutdown->active_offices = active_offices;
    return RC_OK;
}

int write_response(pid_t user_pid, tlv_reply_t tlv_reply, uint32_t bank_office_id)
{
    char user_fifo_path[USER_FIFO_PATH_LEN];
    int fifo_fd;

    sprintf(user_fifo_path, "%s%d", USER_FIFO_PATH_PREFIX, (int) user_pid);

    if ( (fifo_fd = open(user_fifo_path, O_WRONLY)) == -1) {
        tlv_reply.value.header.ret_code = RC_USR_DOWN;
    }
    else {
        write(fifo_fd, &tlv_reply.type, sizeof(op_type_t));
        write(fifo_fd, &tlv_reply.length, sizeof(uint32_t));
        write(fifo_fd, &tlv_reply.value,tlv_reply.length); 
    }

    logReply(get_log_file_des(), bank_office_id, &tlv_reply);

    return 0;
}

void getSalt(char* salt)
{
    char str[] = "0123456789abcdef";

    for(int i = 0; i < SALT_LEN; i++) {
        salt[i] = str[rand() % 16];
    }

    salt[SALT_LEN] = '\0';
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
    if(pid1 == 0) {
        close(fd1[READ]);
        dup2(fd1[WRITE], STDOUT_FILENO);
        execlp("echo", "echo", "-n", stringToHash, NULL);
        perror("Error executing echo command.\n");
        exit(1);
    }
    else if (pid1 > 0) {
        close(fd1[WRITE]);

        wait(&status);
        if(WEXITSTATUS(status) == 1) return 1;

        pid2 = fork();
        if(pid2 == 0) {
            close(fd2[READ]);
            dup2(fd2[WRITE], STDOUT_FILENO);
            dup2(fd1[READ], STDIN_FILENO);
            execlp("sha256sum", "sha256sum", NULL);
            perror("Error executing sha256sum command.\n");
            exit(1);
        }
        else if(pid2 > 0) {
            close(fd2[WRITE]);

            wait(&status);
            if(WEXITSTATUS(status) == 1) return 1;

            dup2(stdout_copy, STDOUT_FILENO);
            dup2(stdin_copy, STDIN_FILENO);

            if (read(fd2[READ], hash, HASH_LEN) == 0) {
                perror("Error reading hash");
                return 1;    
            }

            hash[HASH_LEN] = '\0';
            
            close(stdout_copy);
            close(stdin_copy);

            return 0;
        }
        else {
            perror("Error doing fork.\n");
            return 1;
        }      
    }
    else {
        perror("Error doing fork.\n");
        return 1;
    }
}

int get_account_index(uint32_t account_id)
{
    for (int i = 0; i < current_num_accounts; i++) {
        if (bank_accounts[i].account_id == account_id)
            return i;
    }

    return -1;
}

void destroy_bank_accounts_lock()
{
    pthread_mutex_destroy(&bank_accounts_lock);
}
