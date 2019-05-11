#include "error_checker.h"

int check_ops_args_size(int size)
{
    if(size != 0) {
        printf("Error: This operation does not require any argument.\n");
        return 1;
    }

    return 0;
}

int check_account_id(uint32_t account_id)
{
    if( account_id < MIN_BANK_ACCOUNTS) {
        printf("Error: account_id must be greater or equal to %d.\n", (int) MIN_BANK_ACCOUNTS);
        return 1;
    }
    else if(account_id >= MAX_BANK_ACCOUNTS) {
        printf("Error: account_id must be less than %d.\n", (int) MAX_BANK_ACCOUNTS);
        return 1;
    }

    return 0;
}

int check_balance(uint32_t balance)
{
    if(balance < MIN_BALANCE) {
        printf("Error: balance must be greater than %d.\n", (int) MIN_BALANCE);
        return 1;
    }
    else if(balance > MAX_BALANCE) {
        printf("Error: balance must be less than %d.\n", (int) MAX_BALANCE);
        return 1;
    }

    return 0;
}

int check_password_length(int password_length)
{
    if(password_length < MIN_PASSWORD_LEN) {
        printf("Error: password length must be greater than %d.\n", (int) MIN_PASSWORD_LEN);
        return 1;
    }
    else if(password_length > MAX_PASSWORD_LEN) {
        printf("Error: password length must be less than %d.\n", (int) MAX_PASSWORD_LEN);
        return 1;
    }

    return 0;
}

int check_op_delay(uint32_t op_delay_ms)
{
    if(op_delay_ms < MIN_OP_DEALY_MS) {
        printf("Error: operation delay must be greater than %d.\n", (int) MIN_OP_DEALY_MS);
        return 1;
    }
    else if(op_delay_ms > MAX_OP_DELAY_MS) {
        printf("Error: operation delay must be less than %d.\n", (int) MAX_OP_DELAY_MS);
        return 1;
    }

    return 0;
}

