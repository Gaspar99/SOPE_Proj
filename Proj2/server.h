#pragma once

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <semaphore.h>
#include <signal.h>

#include "constants.h"
#include "types.h"
#include "sope.h"

#include "bank_operations.h"
#include "requests_queue.h"
#include "error_checker.h"
#include "server_log_file.h"

int create_admin_account(char* password);
void *bank_office(void* arg);
void process_request(tlv_request_t tlv_request, tlv_reply_t *tlv_reply, int thread_id);
void sigusr1_handler();

