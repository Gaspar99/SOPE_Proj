#pragma once

#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include "logs.h"
#include "args_handler.h"

int print(const char* file_path, struct commands *cmds);
int get_file_type(const char* file_path, char* file_type, int log_file_des);
int format_date(time_t time, char* formated_date);
int get_hash_codes(const char* file_path, char **hash_codes, struct commands *cmds);

