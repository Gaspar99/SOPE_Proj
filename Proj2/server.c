#include "server.h"

bool mainThreadDown = false;
sem_t empty, full;
int active_offices = 0;
int server_fifo_fd;

int main(int argc, char* argv[])
{    
    int nr_bank_offices;
    tlv_request_t tlv_request;
    srand((unsigned int) time(0) + getpid());
    int empty_sem_value, full_sem_value;

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

    struct sigaction action;
    action.sa_handler = sigusr1_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGUSR1, &action, NULL);

    logSyncMechSem(get_log_file_des(), MAIN_THREAD_ID, SYNC_OP_SEM_INIT, SYNC_ROLE_PRODUCER, UNKNOWN_ID, nr_bank_offices);
    sem_init(&full, SHARED, nr_bank_offices);

    logSyncMechSem(get_log_file_des(), MAIN_THREAD_ID, SYNC_OP_SEM_INIT, SYNC_ROLE_PRODUCER, UNKNOWN_ID, EMPTY_SEM_INIT_VALUE);
    sem_init(&empty, SHARED, EMPTY_SEM_INIT_VALUE);

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
    }

    if(create_admin_account(argv[2])) {
        printf("Error: Unable to create admin account.\n");
        return 1; 
    }

    if( (server_fifo_fd = open(SERVER_FIFO_PATH, O_RDWR)) == -1 ) {
        printf("Error: Unable to open server fifo.\n");
        return 1;
    }

    while(true) {

        memset(&tlv_request, 0, sizeof(tlv_request)); //Clean the struct

        if( read(server_fifo_fd, &tlv_request.type, sizeof(op_type_t)) <= 0) {
            if(errno == EINTR) continue; //try again
            else if(errno == EAGAIN) break; //Fifo is empty
        }

        read(server_fifo_fd, &tlv_request.length, sizeof(uint32_t));
        read(server_fifo_fd, &tlv_request.value, tlv_request.length);

        logRequest(get_log_file_des(), MAIN_THREAD_ID, &tlv_request);

        sem_getvalue(&full, &full_sem_value);
        logSyncMechSem(get_log_file_des(), MAIN_THREAD_ID, SYNC_OP_SEM_WAIT, SYNC_ROLE_PRODUCER, tlv_request.value.header.pid, full_sem_value);
        sem_wait(&full);

        put_request(tlv_request);

        sem_post(&empty);
        sem_getvalue(&empty, &empty_sem_value);
        logSyncMechSem(get_log_file_des(), MAIN_THREAD_ID, SYNC_OP_SEM_POST, SYNC_ROLE_PRODUCER, tlv_request.value.header.pid, empty_sem_value);
    }

    mainThreadDown = true;

    for(int i = 1; i <= nr_bank_offices; i++) {
        sem_post(&empty);
        sem_getvalue(&empty, &empty_sem_value);
        logSyncMechSem(get_log_file_des(), MAIN_THREAD_ID, SYNC_OP_SEM_POST, SYNC_ROLE_PRODUCER, UNKNOWN_ID, empty_sem_value);
    }
    for(int i = 1; i <= nr_bank_offices; i++) {
        pthread_join(bank_offices_tid[i], NULL);
    }

    sem_destroy(&full);
    sem_destroy(&empty);
    destroy_bank_accounts_lock();
    destroy_buffer_lock();

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
    if(create_account(admin_account, MAIN_THREAD_ID, ADMIN_ACCOUNT_ID, 0) != RC_OK) return 1; 

    return 0;
}

void *process_order(void* arg)
{
    int thread_id = *(int *) arg;
    int sem_empty_value, sem_full_value;
    tlv_request_t tlv_request;
    tlv_reply_t tlv_reply;

    while(!mainThreadDown) {

        //Clean the structs
        memset(&tlv_request, 0, sizeof(tlv_request));
        memset(&tlv_reply, 0, sizeof(tlv_reply)); 

        sem_getvalue(&empty, &sem_empty_value);
        logSyncMechSem(get_log_file_des(), thread_id, SYNC_OP_SEM_WAIT, SYNC_ROLE_CONSUMER, UNKNOWN_ID, sem_empty_value);
        sem_wait(&empty);

        if(mainThreadDown) break;

        get_request(&tlv_request, thread_id);

        sem_post(&full);
        sem_getvalue(&full, &sem_full_value);
        logSyncMechSem(get_log_file_des(), thread_id, SYNC_OP_SEM_POST, SYNC_ROLE_CONSUMER, tlv_request.value.header.pid, sem_full_value);

        logRequest(get_log_file_des(), thread_id, &tlv_request);
        active_offices++;

        tlv_reply.type = tlv_request.type;
        tlv_reply.value.header.account_id = tlv_request.value.header.account_id;
        tlv_reply.length = sizeof(tlv_reply.value.header);

        if( (tlv_reply.value.header.ret_code = authenthicate_user(tlv_request.value.header)) != RC_OK) {
            write_response(tlv_request.value.header.pid, tlv_reply, thread_id);
            active_offices--;
            continue; 
        }

        if(check_permissions(tlv_request)) {
            tlv_reply.value.header.ret_code = RC_OP_NALLOW;
            write_response(tlv_request.value.header.pid, tlv_reply, thread_id);
            active_offices--;
            continue;
        }   

        switch(tlv_request.type) {
            case OP_CREATE_ACCOUNT: {
                tlv_reply.value.header.ret_code = create_account(tlv_request.value.create, thread_id, 
                                                tlv_request.value.header.account_id, tlv_request.value.header.op_delay_ms);
                break;
            }
            case OP_BALANCE: {
                tlv_reply.value.header.ret_code = balance_inquiry(tlv_request.value.header, &tlv_reply.value.balance, thread_id);
                tlv_reply.length += sizeof(tlv_reply.value.balance);
                break;
            }
            case OP_TRANSFER: {
                tlv_reply.value.header.ret_code = transfer(tlv_request.value.header, tlv_request.value.transfer, 
                                                &tlv_reply.value.transfer, thread_id);
                tlv_reply.length += sizeof(tlv_reply.value.transfer);
                break;
            }
            case OP_SHUTDOWN: {
                tlv_reply.value.header.ret_code = shutdown(&tlv_reply.value.shutdown, active_offices, thread_id,
                                                tlv_request.value.header.account_id, tlv_request.value.header.op_delay_ms);
                tlv_reply.length += sizeof(tlv_reply.value.shutdown);

                //Change server fifo flags to not block
                int flags = fcntl(server_fifo_fd, F_GETFL, 0);
                flags |= O_NONBLOCK;
                fcntl(server_fifo_fd, F_SETFL, flags);

                kill(getpid(), SIGUSR1); //To unblock main thread from read

                break;
            }
            default: {
                tlv_reply.value.header.ret_code = RC_OTHER;
                break;
            }
        }

        write_response(tlv_request.value.header.pid, tlv_reply, thread_id);

        active_offices--;
    }

    logBankOfficeClose(get_log_file_des(), thread_id, pthread_self());
    return NULL;
}

void sigusr1_handler() {}

