#include "server_log_file.h"

int log_file_des;

int open_log_file()
{
    if( (log_file_des = open(SERVER_LOGFILE, O_WRONLY | O_CREAT | O_APPEND | O_TRUNC, LOG_FILE_PERMISSIONS)) == -1 ) {
        printf("Error: could not open server log file.\n");
        return 1;
    }

    return 0;
}

int get_log_file_des()
{
    return log_file_des;
}

void close_log_file()
{
    close(log_file_des);
}