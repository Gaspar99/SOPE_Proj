#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

#include "constants.h"
#include "types.h"
#include "sope.h"

#include "bank_office.h"
#include "requests_queue.h"
#include "error_checker.h"


int create_admin_account(char* password);
void *process_order(void* arg);
bool check_waiting_requests(int bank_office_id);

