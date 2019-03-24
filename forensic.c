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
    {
        //Checks if file exists in current directory
        if (open(argv[argc - 1], O_RDONLY) == -1)
        {
            perror("File does not exist.");
            exit(1);
        }

        print(argv[argc - 1], &cmds);
    }
}

