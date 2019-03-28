#include <stdio.h>
#include <stdlib.h>

#include "print_info.h"
#include "logs.h"
#include "args_handler.h"
#include "dir_handler.h"
#include "signals_handler.h"

int main(int argc, char *argv[])
{    
    start_time();
    set_signal_handlers();
    
    struct commands cmds;
    commands_handler(argc, argv, &cmds);

    if(is_directory(argv[argc - 1]))
        process_dir(argv[argc - 1], &cmds); 
    else 
        print(argv[argc - 1], &cmds);
    
    if(cmds.output_file_des != -1) close_output_file(&cmds);
    if(cmds.log_file_des != -1) close_logs_file(cmds.log_file_des, cmds.log_file_name);

    return 0;
}

