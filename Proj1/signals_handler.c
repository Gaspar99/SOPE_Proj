#include "signals_handler.h"

static int num_files;
static int num_directories;

void set_signal_handlers()
{
    struct sigaction num_files_action, sigint_action;

    num_files = 0;
    num_directories = 0;
    
    sigint_action.sa_handler = sigint_handler;
    sigemptyset(&sigint_action.sa_mask);
    sigint_action.sa_flags = 0;

    sigaction(SIGINT, &sigint_action, NULL);

    num_files_action.sa_handler = num_files_handler;
    sigemptyset(&num_files_action.sa_mask);
    num_files_action.sa_flags = 0;

    sigaction(SIGUSR1, &num_files_action, NULL);
    sigaction(SIGUSR2, &num_files_action, NULL);

    signal(SIGCHLD, SIG_IGN);
}

void num_files_handler(int sig_no)
{
    struct output_info output_file_info;

    get_output_info(&output_file_info);

    if(sig_no == SIGUSR2)
        num_files++;
    else if(sig_no == SIGUSR1)
    {
        num_directories++;

        char msg[100];
        sprintf(msg, "New directory: %d/%d directories/files at this time.\n", num_directories, num_files);
        write(output_file_info.stdout_copy, msg, strlen(msg));
    }
}

bool sigint_called = false;
void sigint_handler()
{
    sigint_called = true;
}

bool sigint_received()
{
    return sigint_called;
}



