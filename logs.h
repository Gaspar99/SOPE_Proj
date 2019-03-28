#pragma once

#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "signals_handler.h"

int start_time();
int register_log(int log_file_des, pid_t pid, char* act);
int close_logs_file(int log_file_des, char* log_file_name);

