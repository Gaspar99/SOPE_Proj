#include "logs.h"

struct timeval initial_time;

int start_time()
{
    gettimeofday(&initial_time, NULL);

    return 0;
}

int register_log(int log_file_des, pid_t pid, char* act)
{
    char log[400];
    struct timeval current_time;

    //Inst
    gettimeofday(&current_time, NULL);
    float elapsed_time = (float) ( (current_time.tv_usec - initial_time.tv_usec) / 1000);

    char inst[10];
    sprintf(inst, "%.2f", elapsed_time);
    int inst_length = strlen(inst);
    while(inst_length < 6) inst[inst_length++] = ' ';
    inst[inst_length] = '\0';

    //pid
    char pid_string[10];
    sprintf(pid_string, "%d", (int) pid);
    int pid_length = strlen(pid_string);
    while(pid_length < 7) pid_string[pid_length++] = ' ';
    pid_string[pid_length] = '\0';

    sprintf(log, "%s  -  %s  -  %s\n", inst, pid_string, act);

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

