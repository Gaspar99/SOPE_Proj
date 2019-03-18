#include "logs.h"

clock_t initial_time;

int start_time()
{
    initial_time = clock();

    return 0;
}

int register_log(int log_file_des, pid_t pid, char* act)
{
    char log[50];
    
    //Inst
    double elapsed = (double) (clock() - initial_time) * 1000 / CLOCKS_PER_SEC;
    sprintf(log, "%.2lf", elapsed); 

    strcat(log, " - ");  

    //Pid
    char pid_string[30];
    sprintf(pid_string, "%d", (int) pid);
    strcat(log, pid_string);

    strcat(log, " - ");

    //Act
    strcat(log, act);
    strcat(log, "\n");

    write(log_file_des, log, strlen(log));

    return 0;
    
}

