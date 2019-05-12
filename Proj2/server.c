#include "server.h"

int waiting_requests = 0;
int slots = MAX_BANK_ACCOUNTS;

pthread_cond_t slots_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t requests_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t slots_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t requests_lock = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char* argv[])
{    
    int nr_bank_offices;
    int server_fifo_fd;
    tlv_request_t tlv_request;
    srand((unsigned int) time(0) + getpid());

    if(argc != 3) {
        printf("Usage: %s <nr_bank_offices> <admin_password>\n", argv[0]);
        return 1;
    }

    nr_bank_offices = atoi(argv[1]);
    if(nr_bank_offices > MAX_BANK_OFFICES) {
        printf("Error: max amount of bank offices: %d\n", MAX_BANK_OFFICES);
        return 1;
    }

    if(open_log_file()) return 1;

    if(create_admin_account(argv[2])) {
        printf("Error: Unable to create admin account.\n");
        return 1; 
    }

    pthread_t bank_offices_tid[nr_bank_offices];
    int bank_office_id[nr_bank_offices];

    while(true) {
        if (mkfifo(SERVER_FIFO_PATH, OPEN_FIFO_PERMISSIONS)) {
            if(errno == EEXIST) {
                unlink(SERVER_FIFO_PATH);
                continue;
            }   
            else {
                printf("Error creating server fifo.\n");
                return 1;
            }
        }
        else break;
    }
    
    for(int i = 1; i <= nr_bank_offices; i++) {

        bank_office_id[i] = i;
        if(pthread_create(&bank_offices_tid[i], NULL, process_order, (void*) &bank_office_id[i])) {
            printf("Error creating bank office thread.\n");
            return 1;
        }
        logBankOfficeOpen(get_log_file_des(), bank_office_id[i], bank_offices_tid[i]);
        inc_nr_bank_offices_open();
    }

    if ( (server_fifo_fd = open(SERVER_FIFO_PATH, O_RDONLY)) == -1) {
        printf("Error opening server fifo.\n");
        return 1;
    }

    while(check_nr_bank_offices()) {

        if(read(server_fifo_fd, &tlv_request, sizeof(tlv_request)) == 0) {
            sleep(1);
            continue;
        } 

        logSyncMech(get_log_file_des(), MAIN_THREAD_ID, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_PRODUCER, tlv_request.value.header.pid);
        pthread_mutex_lock(&slots_lock);

        while (!(slots > 0)) {
            logSyncMech(get_log_file_des(), MAIN_THREAD_ID, SYNC_OP_COND_WAIT, SYNC_ROLE_PRODUCER, tlv_request.value.header.pid);
            pthread_cond_wait (&slots_cond, &slots_lock);
        }

        slots--;

        pthread_mutex_unlock(&slots_lock);
        logSyncMech(get_log_file_des(), MAIN_THREAD_ID, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_PRODUCER, tlv_request.value.header.pid);

        put_request(tlv_request);
        logRequest(get_log_file_des(), MAIN_THREAD_ID, &tlv_request);

        logSyncMech(get_log_file_des(), MAIN_THREAD_ID, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_PRODUCER, tlv_request.value.header.pid);
        pthread_mutex_lock(&requests_lock);

        waiting_requests++;

        logSyncMech(get_log_file_des(), MAIN_THREAD_ID, SYNC_OP_COND_SIGNAL, SYNC_ROLE_PRODUCER, tlv_request.value.header.pid);
        pthread_cond_signal(&requests_cond);

        pthread_mutex_unlock(&requests_lock);
        logSyncMech(get_log_file_des(), MAIN_THREAD_ID, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_PRODUCER, tlv_request.value.header.pid);
    }

    close_log_file();
    close(server_fifo_fd);
    unlink(SERVER_FIFO_PATH);

    return 0;
}

int create_admin_account(char* password)
{
    req_create_account_t admin_account;

    if(check_password_length(strlen(password))) return 1;

    admin_account.account_id = ADMIN_ACCOUNT_ID;
    admin_account.balance = ADMIN_ACCOUNT_BALANCE;
    strcpy(admin_account.password, password);
    if(create_account(admin_account, MAIN_THREAD_ID) != RC_OK) return 1; 

    return 0;
}

void *process_order(void* arg)
{
    int thread_id = *(int *) arg;
    tlv_request_t tlv_request;
    tlv_reply_t tlv_reply;

    while(waiting_requests != 0 || !isDown() ) {

        logSyncMech(get_log_file_des(), thread_id, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_CONSUMER, UNKNOWN_ID);
        pthread_mutex_lock(&requests_lock);

        while(!(waiting_requests > 0)) {
            logSyncMech(get_log_file_des(), thread_id, SYNC_OP_COND_WAIT, SYNC_ROLE_CONSUMER, UNKNOWN_ID);
            pthread_cond_wait(&requests_cond, &requests_lock);
        }
        
        waiting_requests--;

        pthread_mutex_unlock(&requests_lock);
        logSyncMech(get_log_file_des(), thread_id, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_CONSUMER, UNKNOWN_ID);

        get_request(&tlv_request, thread_id);
        logRequest(get_log_file_des(), thread_id, &tlv_request);

        logSyncMech(get_log_file_des(), thread_id, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_CONSUMER, tlv_request.value.header.pid);
        pthread_mutex_lock(&slots_lock);

        slots++;

        pthread_cond_signal(&slots_cond);
        logSyncMech(get_log_file_des(), thread_id, SYNC_OP_COND_SIGNAL, SYNC_ROLE_CONSUMER, tlv_request.value.header.pid);

        pthread_mutex_unlock(&slots_lock);
        logSyncMech(get_log_file_des(), thread_id, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_CONSUMER, tlv_request.value.header.pid);

        tlv_reply.type = tlv_request.type;
        tlv_reply.value.header.account_id = tlv_request.value.header.account_id;
        tlv_reply.length = sizeof(tlv_reply.value.header);

        if(check_permissions(tlv_request)) {
            tlv_reply.value.header.ret_code = RC_OP_NALLOW;
            write_response(tlv_request.value.header.pid, tlv_reply, thread_id);
            continue;
        }   

        if( (tlv_reply.value.header.ret_code = (int) authenthicate_user(tlv_request.value.header)) != RC_OK) {
            write_response(tlv_request.value.header.pid, tlv_reply, thread_id);
            continue; 
        }

        switch(tlv_request.type) {
            case OP_CREATE_ACCOUNT: {
                tlv_reply.value.header.ret_code = create_account(tlv_request.value.create, thread_id);
                break;
            }
            case OP_BALANCE: {
                tlv_reply.value.header.ret_code = balance_inquiry(tlv_request.value.header, &tlv_reply.value.balance);
                tlv_reply.length += sizeof(tlv_reply.value.balance);
                break;
            }
            case OP_TRANSFER: {
                tlv_reply.value.header.ret_code = transfer(tlv_request.value.transfer, &tlv_reply.value.transfer);
                tlv_reply.length += sizeof(tlv_reply.value.transfer);
                break;
            }
            case OP_SHUTDOWN: {
                tlv_reply.value.header.ret_code = shutdown(&tlv_reply.value.shutdown);
                tlv_reply.length += sizeof(tlv_reply.value.shutdown);
                break;
            }
            default: {
                tlv_reply.value.header.ret_code = RC_OTHER;
                break;
            }
        }

        write_response(tlv_request.value.header.pid, tlv_reply, thread_id);
    }
    
    dec_nr_bank_offices_open(thread_id, tlv_request.value.header.pid);
    logBankOfficeClose(get_log_file_des(), thread_id, pthread_self());

    return NULL;
}

bool check_waiting_requests(int bank_office_id)
{
    bool result;

    logSyncMech(get_log_file_des(), bank_office_id, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_CONSUMER, UNKNOWN_ID);
    pthread_mutex_lock(&requests_lock);

    result = (waiting_requests != 0);

    logSyncMech(get_log_file_des(), bank_office_id, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_CONSUMER, UNKNOWN_ID);
    pthread_mutex_unlock(&requests_lock);

    return result;
}

