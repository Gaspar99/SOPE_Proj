#pragma once

#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logs.h"

struct commands {
    bool read_sub_dirs;
    char* hash_commands;
    int log_file_des;
    char* log_file_name;
    int output_file_des;
};

struct output_info {
    int stdout_copy;
    char* output_file_name;
};

int set_output_info(int stdout_copy, char* output_file_name);
int get_output_info(struct output_info *output_file_info);
int close_output_file(struct commands *cmds);

int commands_handler(int argc, char* argv[], struct commands *cmds);
int check_command(int argc, char *argv[], char command[]);