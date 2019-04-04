#pragma once

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h> 
#include <string.h>

#include "args_handler.h"

void set_signal_handlers();
void sigint_handler();
void num_files_handler(int sig_no);
bool sigint_received();
void print_final_count();
