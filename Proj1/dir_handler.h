#pragma once

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>

#include "args_handler.h"
#include "print_info.h"

int process_dir(const char *path, struct commands *cmds);
bool is_directory(const char *path);


