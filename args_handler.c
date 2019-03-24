#include "args_handler.h"

int commands_handler(int argc, char* argv[], struct commands *cmds)
{
    if (argc < 2 || argc > 9)
    {
        perror("Wrong number of possible arguments.");
        exit(1);
    }
    
    //Command: -r
    if (check_command(argc, argv, "-r") != -1)
        cmds->read_sub_dirs = true;
    else cmds->read_sub_dirs = false;

    //Command: -h
    cmds->hash_commands = (char*) malloc(sizeof(char) * 20);
    int hash_command_index = check_command(argc, argv, "-h");
    if (hash_command_index != -1)
        strcpy(cmds->hash_commands, argv[hash_command_index + 1]);
    else
        cmds->hash_commands = NULL;

    //Command: -o
    int output_command_index = check_command(argc, argv, "-o");
    if (output_command_index != -1)
    {
        char output_file_name[20];
        strcpy(output_file_name, argv[output_command_index + 1]);
        cmds->output_file_des = open(output_file_name, O_WRONLY | O_CREAT | O_APPEND, 0750);

        set_output_info(dup(STDOUT_FILENO), output_file_name);
        dup2(cmds->output_file_des, STDOUT_FILENO);
    }
    else {
        set_output_info(-1, NULL);
        cmds->output_file_des = -1;
    }

    //Command: -v
    int log_command_index = check_command(argc, argv, "-v");
    if (log_command_index != -1)
    {
        char *log_file_name = (char *)malloc(sizeof(char) * 80);
        log_file_name = getenv("LOGFILENAME");
        
        cmds->log_file_des = open(log_file_name, O_WRONLY | O_CREAT | O_APPEND, 0750);
    }
    else
        cmds->log_file_des = -1;

    return 0;
}

int check_command(int argc, char *argv[], char command[])
{
    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(command, argv[i]))
            return i;
    }

    return -1;
}


struct output_info out;

int set_output_info(int stdout_copy, char* output_file_name)
{
    if(output_file_name != NULL) {
        out.output_file_name = (char*) malloc(sizeof(char) * 50);
        strcpy(out.output_file_name, output_file_name);
    }
    else out.output_file_name = output_file_name;   
    out.stdout_copy = stdout_copy;

    return 0;
}

int get_output_info(struct output_info *output_file_info)
{
    output_file_info->output_file_name = (char*) malloc(sizeof(char) * 50);
    strcpy(output_file_info->output_file_name, out.output_file_name);
    output_file_info->stdout_copy = out.stdout_copy;

    return 0;
}

