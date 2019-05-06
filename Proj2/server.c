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

bank_account_t bank_accounts[MAX_BANK_ACCOUNTS];
int current_num_accounts = 0;

int getHashPassword(char* password, char* hashPassword);
int create_account(uint32_t account_id, uint32_t balance, char* password);
void* process_order(void* arg);

int create_account(uint32_t account_id, uint32_t balance, char* password)
{
    bank_accounts[current_num_accounts].account_id = account_id;
    bank_accounts[current_num_accounts].balance = balance;
    
    if (getHashPassword(password, bank_accounts[current_num_accounts].hash)) return 1;

    //Get salt

    current_num_accounts++;
    return 0;
}

int getHashPassword(char* password, char* hashPassword)
{
    hashPassword = (char*) malloc(sizeof(char) * HASH_LEN);

    (void) password;
    (void) hashPassword;
    return 0;
}

void *process_order(void* arg)
{
    (void) arg;

    //Open server fifo to read request

    
    return NULL;
}

int main(int argc, char* argv[])
{
    int nr_bank_offices;

    if(argc != 3) {
        printf("Usage: %s <nr_bank_offices> <admin_password>\n", argv[0]);
        return 1;
    }

    nr_bank_offices = atoi(argv[1]);
    if(nr_bank_offices > MAX_BANK_OFFICES) {
        printf("Error: max amount of bank offices: %d\n", MAX_BANK_OFFICES);
        return 1;
    }

    if(create_account(ADMIN_ACCOUNT_ID, ADMIN_ACCOUNT_BALANCE, argv[2])) return 1;

    pthread_t bank_offices_tid[nr_bank_offices];

    mkfifo(SERVER_FIFO_PATH, OPEN_FIFO_PERMISSIONS);

    for(int i = 1; i <= nr_bank_offices; i++) {
        pthread_create(&bank_offices_tid[i], NULL, process_order, NULL);
        logBankOfficeOpen(STDOUT_FILENO, i, bank_offices_tid[i]);
    }




}


