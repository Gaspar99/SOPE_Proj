#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdbool.h> 
#include <time.h>

#include "constants.h"
#include "types.h"
#include "sope.h"

#include "requests_queue.h"
#include "error_checker.h"
#include "server_log_file.h"

int check_permissions(tlv_request_t tlv_request);
ret_code_t authenthicate_user(req_header_t req_header);
ret_code_t create_account(req_create_account_t req_create_account, int bank_office_id, uint32_t account_id, uint32_t op_delay);
ret_code_t balance_inquiry(req_header_t req_header, rep_balance_t *rep_balance, int bank_office_id);
ret_code_t transfer(req_header_t req_header, req_transfer_t req_transfer, rep_transfer_t *rep_transfer, int bank_office_id);
ret_code_t shutdown(rep_shutdown_t *rep_shutdown, int active_offices, int bank_office_id, uint32_t account_id, uint32_t op_delay);

int getHash(char* password, char* salt, char* hash);
void getSalt(char* salt);
int get_account_index(uint32_t account_id);

int write_response(pid_t user_pid, tlv_reply_t tlv_reply, uint32_t bank_office_id);

void destroy_bank_accounts_lock();
