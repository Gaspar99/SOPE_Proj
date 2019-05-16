#include "requests_queue.h"

tlv_request_t request_queue[MAX_REQUESTS];
int front = 0;
int rear = -1;
pthread_mutex_t buffer_lock = PTHREAD_MUTEX_INITIALIZER;

void put_request(tlv_request_t tlv_request)
{
    logSyncMech(get_log_file_des(), MAIN_THREAD_ID, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_PRODUCER, tlv_request.value.header.pid);
    pthread_mutex_lock(&buffer_lock);

    if(rear == MAX_REQUESTS - 1) {
        rear = -1;
    }
            
    request_queue[++rear] = tlv_request;

    pthread_mutex_unlock(&buffer_lock);
    logSyncMech(get_log_file_des(), MAIN_THREAD_ID, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_PRODUCER, tlv_request.value.header.pid);
}

void get_request(tlv_request_t *tlv_request, int bank_office_id)
{
    logSyncMech(get_log_file_des(), bank_office_id, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_CONSUMER, UNKNOWN_ID);
    pthread_mutex_lock(&buffer_lock);
    
    *tlv_request = request_queue[front++];

    if(front == MAX_REQUESTS) {
        front = 0;
    }

    pthread_mutex_unlock(&buffer_lock);
    logSyncMech(get_log_file_des(), bank_office_id, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_CONSUMER, tlv_request->value.header.pid);
}

void destroy_buffer_lock()
{
    pthread_mutex_destroy(&buffer_lock);
}

