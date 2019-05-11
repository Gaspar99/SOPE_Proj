#include "tlv_request_filler.h"
#include "error_checker.h"

int get_tvl_request(char* argv[], tlv_request_t* tlv_request)
{
    tlv_request->type = (op_type_t) atoi(argv[4]);
    if( get_req_header(argv, &tlv_request->value.header)) return 1;
    tlv_request->length = sizeof(tlv_request->value.header);

    int ops_args_size = strlen(argv[5]);

    switch(tlv_request->type) {
        case OP_CREATE_ACCOUNT: {
            if(get_req_create_account(argv[5], &tlv_request->value.create)) return 1;
            tlv_request->length += sizeof(tlv_request->value.create);
            break;
        }
        case OP_BALANCE: {
            if(check_ops_args_size(ops_args_size)) return 1;
            break;
        }
        case OP_TRANSFER: {
            if(get_req_transfer(argv[5], &tlv_request->value.transfer)) return 1;
            tlv_request->length += sizeof(tlv_request->value.transfer);
            break;
        }
        case OP_SHUTDOWN: {
            if(check_ops_args_size(ops_args_size)) return 1;
            break;
        }
        default: {
            printf("Error: max number of operations is %d.\n", (int) __OP_MAX_NUMBER);
            return 1;
        } 
    }
        
    return 0;
}

int get_req_header(char* argv[], req_header_t* req_header)
{
    uint32_t account_id = (uint32_t) atoi(argv[1]);
    uint32_t op_dealy_ms = atoi(argv[3]);

    if(check_op_delay(op_dealy_ms)) return 1;
    if(check_password_length(strlen(argv[2]))) return 1;

    req_header->pid = getpid();
    req_header->account_id = account_id;
    strcpy(req_header->password, argv[2]);
    req_header->op_delay_ms = op_dealy_ms;

    return 0;
}

int get_req_create_account(char* op_args, req_create_account_t* req_create_account)
{
    char** args;
    args = (char**) malloc(sizeof(char*) * 5);
    for(int i = 0; i < 5; i++) {
        args[i] = (char*) malloc(sizeof(char) * MAX_PASSWORD_LEN);
    }

    int argsSize = split_string(op_args, args);

    if(argsSize != 3) {
        printf("Error: Wrong number of operation arguments.\n");
        return 1;
    }

    uint32_t account_id = (uint32_t) atoi(args[0]);
    uint32_t balance = (uint32_t) atoi(args[1]);

    if(check_account_id(account_id)) return 1;
    if(check_balance(balance)) return 1;
    if(check_password_length(strlen(args[2]))) return 1;

    req_create_account->account_id = account_id;
    req_create_account->balance = balance;
    strcpy(req_create_account->password, args[2]);
    return 0;
}

int get_req_transfer(char* op_args, req_transfer_t* req_transfer)
{
    char** args;
    args = (char**) malloc(sizeof(char*) * 5);
    for(int i = 0; i < 5; i++) {
        args[i] = (char*) malloc(sizeof(char) * MAX_PASSWORD_LEN);
    }
        
    int argsSize = split_string(op_args, args);

    if(argsSize != 2) {
        printf("Error: Wrong number of operation arguments.\n");
        return 1;
    }

    uint32_t account_id = (uint32_t) atoi(args[0]);
    uint32_t amount = (uint32_t) atoi(args[1]);

    if(check_account_id(account_id)) return 1;
    if(check_balance(amount)) return 1;

    req_transfer->account_id = account_id;
    req_transfer->amount = amount;

    return 0;
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



