#pragma once

#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

struct log_info {
    int stdout_copy;
    char* output_file_name;
};

int set_log_info(int stdout_copy, char* output_file_name);
int start_time();
int register_log(int log_file_des, pid_t pid, char* act);

