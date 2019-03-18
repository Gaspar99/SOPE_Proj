#pragma once

#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

#include "logs.h"

int print(char* file_name, char* hash_commands, int log_file_des);
int get_file_type(char* file_name, char* file_type, int log_file_des);
int format_date(time_t time, char* formated_date);
int get_hash_codes(char* file, char* hash_commands, char **hash_codes, int log_file_des);

