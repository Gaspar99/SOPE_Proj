#include "bank_office.h"

bank_account_t bank_accounts[MAX_BANK_ACCOUNTS];
int current_num_accounts = 0;

void *process_order(void* arg)
{
    (void) arg;
    int server_fifo_fd;
    tlv_request_t tlv_request;
    ret_code_t ret_code;

    if ( (server_fifo_fd = open(SERVER_FIFO_PATH, O_RDONLY)) == -1) {
        printf("Error opening server fifo.\n");
        //return
    }

    read(server_fifo_fd, &tlv_request, sizeof(tlv_request));

    ret_code = authenthicate_user(tlv_request.value.header);
    
    switch(tlv_request.type) {
        case OP_CREATE_ACCOUNT: {
            if(tlv_request.value.header.account_id != ADMIN_ACCOUNT_ID) {
                ret_code = RC_OP_NALLOW;
                break;
            }
            ret_code = create_account(tlv_request.value.create);
            break;
        }
        case OP_BALANCE: {
            if(tlv_request.value.header.account_id == ADMIN_ACCOUNT_ID) {
                ret_code = RC_OP_NALLOW; 
                break;
            }
            ret_code = balance_inquiry(tlv_request.value.header);
            break;
        }
        case OP_TRANSFER: {
            if(tlv_request.value.header.account_id == ADMIN_ACCOUNT_ID) {
                ret_code = RC_OP_NALLOW;
                break;
            }
            ret_code = transfer(tlv_request.value.transfer);
            break;
        }
        case OP_SHUTDOWN: {
            if(tlv_request.value.header.account_id != ADMIN_ACCOUNT_ID) {
                ret_code = RC_OP_NALLOW;
                break;
            }
            ret_code = shutdown();
            break;
        }
    }

    write_response(tlv_request.value.header.pid, ret_code);
    return NULL;
}

ret_code_t authenthicate_user(req_header_t req_header)
{

}

ret_code_t create_account(req_create_account_t req_create_account)
{
    bank_accounts[current_num_accounts].account_id = req_create_account.account_id;
    bank_accounts[current_num_accounts].balance = req_create_account.balance;
    
    if (getHash(req_create_account.password, bank_accounts[current_num_accounts].hash)) return 1;

    //Get salt

    current_num_accounts++;
    return 0;
}

int getHash(char* password, char* hash)
{
    (void) password;
    (void) hash;
    return 0;
}

ret_code_t balance_inquiry(req_header_t req_header)
{

}

ret_code_t transfer(req_transfer_t req_transfer)
{

}

ret_code_t shutdown()
{

}

int write_response(pid_t user_pid, ret_code_t ret_code)
{
    char user_fifo_path[USER_FIFO_PATH_LEN];
    int fifo_fd;

    sprintf(user_fifo_path, "%s%d", USER_FIFO_PATH_PREFIX, (int) user_pid);
    mkfifo(user_fifo_path, OPEN_FIFO_PERMISSIONS);

    if ( (fifo_fd = open(user_fifo_path, O_WRONLY)) == -1) {
        printf("Error opening user fifo.\n");
        return 1;
    }

    write(fifo_fd, &ret_code, sizeof(ret_code));

    return 0;
}
