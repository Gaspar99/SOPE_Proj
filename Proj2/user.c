#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "types.h"
#include "sope.h"

int get_req_header(char* argv[], req_header_t* req_header)
{
    req_header->pid = getpid();
    req_header->account_id = atoi(argv[1]);
    strcpy(req_header->password, argv[2]);
    req_header->op_delay_ms = atoi(argv[3]);

    return 0;
}

int main(int argc, char* argv[])
{
    req_header_t req_header;

    if(argc != 6)
    {
        printf("Usage: %s <account_id> <password> <op_delay> <op_code> <op_args>\n", argv[0]);
        return 1;
    }

    get_req_header(argv, &req_header);
}