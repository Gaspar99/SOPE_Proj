#include "signals_handler.h"

void set_signal_handlers()
{
    struct sigaction num_files_action;
    /*
    sigint_action.sa_handler = sigint_handler;
    sigemptyset(&sigint_action.sa_mask);
    sigint_action.sa_flags = 0;

    sigaction(SIGINT, &sigint_action, NULL);*/

    num_files_action.sa_handler = num_files_handler;
    sigemptyset(&num_files_action.sa_mask);
    num_files_action.sa_flags = 0;

    sigaction(SIGUSR1, &num_files_action, NULL);
    sigaction(SIGUSR2, &num_files_action, NULL);

    signal(SIGCHLD, SIG_IGN);
}

void num_files_handler(int sig_no)
{
    static int num_files = 0;
    static int num_directories = 0;

    struct output_info output_file_info;

    get_output_info(&output_file_info);

    if(sig_no == SIGUSR1)
    {
        num_directories++;

        char msg[70];
        sprintf(msg, "New directory: %d/%d directories/files at this time.\n", num_directories, num_files);
        write(output_file_info.stdout_copy, msg, strlen(msg));
    }
    else if(sig_no == SIGUSR2)
        num_files++;
}


