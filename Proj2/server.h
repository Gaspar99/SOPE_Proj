#pragma once

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <semaphore.h>
#include <signal.h>

#include "constants.h"
#include "types.h"
#include "sope.h"

#include "bank_office.h"
#include "requests_queue.h"
#include "error_checker.h"
#include "server_log_file.h"

int create_admin_account(char* password);
void *process_order(void* arg);
void sigusr1_handler();

