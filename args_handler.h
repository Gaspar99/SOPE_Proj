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
    int output_file_des;
};

int commands_handler(int argc, char* argv[], struct commands *cmds);
int check_command(int argc, char *argv[], char command[]);