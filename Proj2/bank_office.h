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

#include "error_checker.h"

void* process_order(void* arg);
ret_code_t authenthicate_user(req_header_t req_header);

ret_code_t create_account(req_create_account_t req_create_account);
ret_code_t balance_inquiry(req_header_t req_header);
ret_code_t transfer(req_transfer_t req_transfer);
ret_code_t shutdown();

int getHash(char* password, char* hash);

int write_response(pid_t user_pid, ret_code_t ret_code);

