#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "constants.h"
#include "types.h"
#include "sope.h"

int get_tvl_request(char* argv[], tlv_request_t* tlv_request);
int get_req_header(char* argv[], req_header_t* req_header);
int get_req_create_account(char* op_args, req_create_account_t* req_create_account);
int get_req_transfer(char* op_args, req_transfer_t* req_transfer);

int split_string(char* string, char* array[]);

