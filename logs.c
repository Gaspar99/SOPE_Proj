#include "logs.h"

static clock_t initial_time;

int start_time()
{
    initial_time = clock();

    return 0;
}

int register_log(int log_file_des, pid_t pid, char* act)
{
    char log[50];
    clock_t current_time = clock();
    
    //Inst
    char inst[10];
    double elapsed = ( ((double) (current_time - initial_time)) / CLOCKS_PER_SEC ) * 1000;
    sprintf(inst, "%f", elapsed);
    if(inst[4] >= 5) inst[3]++;
    inst[4] = '\0';

    sprintf(log, "%s  -  %d  -  %s\n", inst, (int) pid, act);

    write(log_file_des, log, strlen(log));

    return 0;
}

