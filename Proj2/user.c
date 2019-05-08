#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "constants.h"
#include "types.h"
#include "sope.h"

#include "tlv_request_filler.h"

int main(int argc, char* argv[])
{
    tlv_request_t tlv_request;
    char user_fifo_path[USER_FIFO_PATH_LEN];
    int server_fifo_fd;
    
    if(argc != 6)
    {
        printf("Usage: %s <account_id> <password> <op_delay> <op_code> <op_args>\n", argv[0]);
        return 1;
    }

    if(get_tvl_request(argv, &tlv_request)) return 1;

    sprintf(user_fifo_path, "%s%d", USER_FIFO_PATH_PREFIX, (int) getpid());
    mkfifo(user_fifo_path, OPEN_FIFO_PERMISSIONS);

    //Open server fifo to write tlv request
    if ( (server_fifo_fd = open(SERVER_FIFO_PATH, O_WRONLY)) == -1) {
        printf("Error opening server fifo.\n");
        return 1;
    }

    write(server_fifo_fd, &tlv_request, tlv_request.length);

    //Open user fifo to read response



    return 0;
}

