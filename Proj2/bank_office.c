#include "bank_office.h"

bank_account_t bank_accounts[MAX_BANK_ACCOUNTS];
int current_num_accounts = 0;

int log_file_des;
int nr_bank_offices_open;
bool down = false;

pthread_mutex_t nr_bank_offices_open_lock = PTHREAD_MUTEX_INITIALIZER;

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

    if( (account_index = get_account_index(req_header.account_id)) == -1) return RC_ID_NOT_FOUND;
    if(getHash(req_header.password, bank_accounts[account_index].salt, hash)) return RC_OTHER;
    if(strcmp(hash, bank_accounts[account_index].hash)) return RC_LOGIN_FAIL;

    return RC_OK;
}

ret_code_t create_account(req_create_account_t req_create_account, int bank_office_id)
{
    char hash[HASH_LEN + 1];
    char salt[SALT_LEN + 1];

    if(get_account_index(req_create_account.account_id) != -1) return RC_ID_IN_USE;

    getSalt(salt);
    if (getHash(req_create_account.password, salt, hash)) return RC_OTHER;

    bank_accounts[current_num_accounts].account_id = req_create_account.account_id;
    bank_accounts[current_num_accounts].balance = req_create_account.balance;
    strcpy(bank_accounts[current_num_accounts].hash, hash);
    strcpy(bank_accounts[current_num_accounts].salt, salt);

    logAccountCreation(log_file_des, bank_office_id, &bank_accounts[current_num_accounts]);

    current_num_accounts++;
    return RC_OK;
}

ret_code_t balance_inquiry(req_header_t req_header, rep_balance_t *rep_balance)
{
    int index = get_account_index(req_header.account_id);
    if(index==-1)
    return RC_ID_NOT_FOUND;
    else
    {
         rep_balance->balance=bank_accounts[index].balance;
        return RC_OK;
    }
    
}

ret_code_t transfer(req_header_t req_header, req_transfer_t req_transfer, rep_transfer_t *rep_transfer)
{
    int index1= get_account_index(req_transfer.account_id);
    if(index1==-1)
    return RC_ID_NOT_FOUND;

    int balance= bank_accounts[index1].balance;
    if(balance+req_transfer.amount>MAX_BALANCE)
    return RC_TOO_HIGH;

    int index2=get_account_index(req_header.account_id);
    if(index2==-1)
    return RC_ID_NOT_FOUND;

    balance=bank_accounts[index2].balance;
    if(balance-req_transfer.amount<1)
    return RC_NO_FUNDS;
    
    bank_accounts[index2].balance=bank_accounts[index2].balance-req_transfer.amount;
    bank_accounts[index1].balance=bank_accounts[index1].balance+req_transfer.amount;
    rep_transfer->balance=bank_accounts[index1].balance;

    return RC_OK;
}

ret_code_t shutdown(rep_shutdown_t *rep_shutdown)
{
    turnDown();

    if( chmod(SERVER_FIFO_PATH, READ_ONLY_PERMISSIONS)) return RC_OTHER;

    rep_shutdown->active_offices = nr_bank_offices_open;

    return RC_OK;
}

int write_response(pid_t user_pid, tlv_reply_t tlv_reply, int bank_office_id)
{
    char user_fifo_path[USER_FIFO_PATH_LEN];
    int fifo_fd;

    sprintf(user_fifo_path, "%s%d", USER_FIFO_PATH_PREFIX, (int) user_pid);

    if ( (fifo_fd = open(user_fifo_path, O_WRONLY)) == -1) {
        printf("Error opening user fifo.\n");
        return 1;
    }

    write(fifo_fd, &tlv_reply, sizeof(tlv_reply));
    logReply(log_file_des, bank_office_id, &tlv_reply);

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

bool isDown()
{
    return down;
}

void turnDown()
{
    down = true;
}

void inc_nr_bank_offices_open()
{
    nr_bank_offices_open++;
}

void dec_nr_bank_offices_open(int bank_office_id, pid_t user_pid)
{
    logSyncMech(get_log_file_des(), bank_office_id, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_PRODUCER, user_pid);
    pthread_mutex_lock(&nr_bank_offices_open_lock);

    nr_bank_offices_open--;

    pthread_mutex_unlock(&nr_bank_offices_open_lock);
    logSyncMech(get_log_file_des(), bank_office_id, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_PRODUCER, user_pid);
}

bool check_nr_bank_offices()
{
    bool result;

    logSyncMech(get_log_file_des(), MAIN_THREAD_ID, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_PRODUCER, UNKNOWN_ID);
    pthread_mutex_lock(&nr_bank_offices_open_lock);

    result = (nr_bank_offices_open > 0);

    pthread_mutex_unlock(&nr_bank_offices_open_lock);
    logSyncMech(get_log_file_des(), MAIN_THREAD_ID, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_PRODUCER, UNKNOWN_ID);

    return result;
}

int open_log_file()
{
    if( (log_file_des = open(SERVER_LOGFILE, O_WRONLY | O_CREAT | O_APPEND | O_TRUNC, LOG_FILE_PERMISSIONS)) == -1 ) {
        printf("Error: could not open server log file.\n");
        return 1;
    }

    return 0;
}

int get_log_file_des()
{
    return log_file_des;
}

void close_log_file()
{
    close(log_file_des);
}
