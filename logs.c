#include "logs.h"

clock_t initial_time;

int start_time()
{
    initial_time = clock();

    return 0;
}

int register_log(int log_file_des, pid_t pid, char* act)
{
    char log[400];
    //clock_t current_time = clock();
    
    //Inst
    clock_t running_time = clock() - initial_time;
    double elapsed = ((double) (running_time / CLOCKS_PER_SEC ) * 1000 );

    sprintf(log, "%.2lf  -  %d  -  %s\n", elapsed, (int) pid, act);

    write(log_file_des, log, strlen(log));

    return 0;
}

int close_logs_file(int log_file_des, char* log_file_name)
{
    close(log_file_des);

    char msg[50];
    sprintf(msg, "Execution records saved on file %s\n", log_file_name);
    write(STDOUT_FILENO, msg, strlen(msg));

    return 0;
}

