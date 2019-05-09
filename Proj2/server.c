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

#include "bank_office.h"
#include "requests_queue.h"

int log_file_des;

int create_admin_account(char* password);

int main(int argc, char* argv[])
{
    int nr_bank_offices;
    int server_fifo_fd;
    tlv_request_t tlv_request;

    if(argc != 3) {
        printf("Usage: %s <nr_bank_offices> <admin_password>\n", argv[0]);
        return 1;
    }

    nr_bank_offices = atoi(argv[1]);
    if(nr_bank_offices > MAX_BANK_OFFICES) {
        printf("Error: max amount of bank offices: %d\n", MAX_BANK_OFFICES);
        return 1;
    }

    if( (log_file_des = open(SERVER_LOGFILE, O_WRONLY | O_CREAT | O_APPEND | O_TRUNC, LOG_FILE_PERMISSIONS)) == -1 ) {
        printf("Error: could not open server log file.\n");
        return 1;
    }

    if(create_admin_account(argv[2])) {
        printf("Error: Unable to create admin account.\n");
        return 1; 
    }

    pthread_t bank_offices_tid[nr_bank_offices];

    mkfifo(SERVER_FIFO_PATH, OPEN_FIFO_PERMISSIONS);

    for(int i = 1; i <= nr_bank_offices; i++) {
        pthread_create(&bank_offices_tid[i], NULL, process_order, NULL);
        logBankOfficeOpen(STDOUT_FILENO, i, bank_offices_tid[i]);
    }

    if ( (server_fifo_fd = open(SERVER_FIFO_PATH, O_RDONLY)) == -1) {
        printf("Error opening server fifo.\n");
        return 1;
    }

    while(true) {
        read(server_fifo_fd, &tlv_request, sizeof(tlv_request));
        put_request(tlv_request);
    }
    

}

int create_admin_account(char* password)
{
    req_create_account_t admin_account;
    admin_account.account_id = ADMIN_ACCOUNT_ID;
    admin_account.balance = ADMIN_ACCOUNT_BALANCE;
    strcpy(admin_account.password, password);
    if(create_account(admin_account) != RC_OK) return 1; 

    return 0;
}


