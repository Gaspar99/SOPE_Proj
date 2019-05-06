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

int get_tvl_request(char* argv[], tlv_request_t* tlv_request)
{
    op_type_t op_type = (op_type_t) atoi(argv[4]);
    get_req_header(argv, &tlv_request->value.header);

    switch(op_type) {
        case OP_CREATE_ACCOUNT: {
            if(tlv_request->value.header.account_id != ADMIN_ACCOUNT_ID) {
                printf("Error: Only the admin can create new bank accounts.\n");
                return 1;
            }

            get_req_create_account(argv[5], &tlv_request->value.create);
            break;
        }
        case OP_BALANCE: {
            if( strlen(argv[5]) > 0) {
                printf("Error: This operation does not require any argument.\n");
                return 1;
            }
            break;
        }
        case OP_TRANSFER: {
            get_req_transfer(argv[5], &tlv_request->value.transfer);
            break;
        }
        case OP_SHUTDOWN: {
            if(tlv_request->value.header.account_id != ADMIN_ACCOUNT_ID) {
                printf("Error: Only the admin can create new bank accounts.\n");
                return 1;
            }
            if(strlen(argv[5]) > 0) {
                printf("Error: This operation does not require any argument.\n");
                return 1;
            }
            break;
        }
            
    }

    tlv_request->type = op_type;

    get_req_header(argv, &tlv_request->value.header);
}

int get_req_header(char* argv[], req_header_t* req_header)
{
    req_header->pid = getpid();
    req_header->account_id = atoi(argv[1]);
    strcpy(req_header->password, argv[2]);
    req_header->op_delay_ms = atoi(argv[3]);

    return 0;
}

int get_req_create_account(char* op_args, req_create_account_t* req_create_account)
{
    char* args[MAX_PASSWORD_LEN];
    for(int i = 0; i < MAX_PASSWORD_LEN; i++)
        args[i] = (char*) malloc(sizeof(char) * 21);

    int argsSize = split_string(op_args, &args);

    if(argsSize != 3) {
        printf("Error: Wrong number of operation arguments.\n");
        return 1;
    }

    //Put the arguments in the right place of the tlv request

}

int get_req_transfer(char* op_args, req_transfer_t* req_transfer)
{
    char* args[MAX_PASSWORD_LEN];
    for(int i = 0; i < MAX_PASSWORD_LEN; i++)
        args[i] = (char*) malloc(sizeof(char) * 21);
        
    int argsSize = split_string(op_args, &args);

    if(argsSize != 2) {
        printf("Error: Wrong number of operation arguments.\n");
        return 1;
    }

    //Put the arguments in the right place of the tlv request

}

int split_string(char* string, char* array[])
{
    int array_size = 0;

    char *p = strtok(string, " ");

    while(p != NULL) {
        array[array_size++] = p;
        p = strtok(NULL, " ");
    }

    return array_size;
}

int main(int argc, char* argv[])
{
    tlv_request_t tlv_request;
    char fifo_path[USER_FIFO_PATH_LEN];
    
    if(argc != 6)
    {
        printf("Usage: %s <account_id> <password> <op_delay> <op_code> <op_args>\n", argv[0]);
        return 1;
    }

    if(get_tvl_request(argv, &tlv_request)) return 1;

    sprintf(fifo_path, "%s%d", USER_FIFO_PATH_PREFIX, (int) getpid());
    mkfifo(fifo_path, OPEN_FIFO_PERMISSIONS);

    //Open server fifo to write tlv request

    //Open user fifo to read response
}

