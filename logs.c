#include "logs.h"

int register_log(int log_file_des, pid_t pid, clock_t beginning, char* act)
{
    char log[50];

    double elapsed = (double) (clock() - beginning) * 1000 / CLOCKS_PER_SEC;


}