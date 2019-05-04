#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include "constants.h"

struct account {
    int balance;
    char* hashPassword;
};

struct account bank_accounts[MAX_BANK_ACCOUNTS];


int creat_account(int account_id, int balance, char* password)
{
    bank_accounts[account_id].balance = balance;
    if(getHashPassword(password, bank_accounts[account_id].hashPassword)) return 1;

    return 0;
}

int getHashPassword(char* password, char* hashPassword)
{



    return 0;
}

int main(int argc, char* argv[])
{
    int nr_bank_offices;

    if(argc != 3) {
        printf("Usage: %s <nr_bank_offices> <password>\n", argv[0]);
        return 1;
    }

    nr_bank_offices = atoi(argv[1]);
    if(nr_bank_offices > MAX_BANK_OFFICES) {
        printf("Error: max amount of bank offices: %d\n", MAX_BANK_OFFICES);
        return 1;
    }

    creat_account(ADMIN_ACCOUNT_ID, ADMIN_ACCOUNT_BALANCE, argv[2]);

    pthread_t tidf[nr_bank_offices];

    //make fifo

    //create threads




}


