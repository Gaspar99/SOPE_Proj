#include "requests_queue.h"

tlv_request_t request_queue[MAX_REQUESTS];
int waiting_requests = 0;
int front = 0;
int rear = -1;
pthread_mutex_t buffer_lock = PTHREAD_MUTEX_INITIALIZER;

void put_request(tlv_request_t tlv_request)
{
    pthread_mutex_lock(&buffer_lock);
    if(waiting_requests != MAX_REQUESTS) 
    {
        if(rear == MAX_REQUESTS - 1) {
            rear = -1;
        }
            
        request_queue[++rear] = tlv_request;
        waiting_requests++;
    }
    pthread_mutex_unlock(&buffer_lock);
}

void get_request(tlv_request_t *tlv_request)
{
    pthread_mutex_lock(&buffer_lock);
    *tlv_request = request_queue[front++];

    if(front == MAX_REQUESTS) {
        front = 0;
    }

    waiting_requests--;
    pthread_mutex_unlock(&buffer_lock);
}

