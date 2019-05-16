#pragma once

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#include "constants.h"

int open_log_file();
int get_log_file_des();
void close_log_file();