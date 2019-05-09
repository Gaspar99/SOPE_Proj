#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "constants.h"
#include "types.h"
#include "sope.h"

#include "requests_queue.h"
#include "error_checker.h"

void* process_order(void* arg);
int authenthicate_user(req_header_t req_header);

ret_code_t create_account(req_create_account_t req_create_account);
ret_code_t balance_inquiry(req_header_t req_header, rep_balance_t *rep_balance);
ret_code_t transfer(req_transfer_t req_transfer, rep_transfer_t *rep_transfer);
ret_code_t shutdown(rep_shutdown_t *rep_shutdown);

int getHash(char* password, char* hash);
int getSalt(char* salt);

int write_response(pid_t user_pid, tlv_reply_t tlv_reply);

