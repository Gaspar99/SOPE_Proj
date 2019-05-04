#include "dir_handler.h"

int process_dir(const char *path, struct commands *cmds)
{
    pid_t pid;
    DIR *dirp;
    struct dirent *direntp;
    char file[300];

    if ((dirp = opendir(path)) == NULL)
    {
        perror(path);
        return 1;
    }

    while ((direntp = readdir(dirp)) != NULL)
    {
        if(sigint_received()) {
            if(cmds->output_file_des != -1) close_output_file(cmds);
            if(cmds->log_file_des != -1) close_logs_file(cmds->log_file_des, cmds->log_file_name);
            exit(0);
        }

        if(!strcmp(direntp->d_name, ".") || !strcmp(direntp->d_name, "..")) //Ignore parent folders
			continue;

        sprintf(file, "%s/%s", path, direntp->d_name);
        
        if (is_directory(file)) {

            if(cmds->output_file_des != -1){
                raise(SIGUSR1);

                //Register event
                if(cmds->log_file_des != -1) 
                    register_log(cmds->log_file_des, getpid(), "SIGNAL USR1");
            }

            if(cmds->read_sub_dirs)
            {
                pid = fork();
                if (pid == 0) //Child
                { 
                    process_dir(file, cmds);
                    exit(0);
                }
            }
        }
        else {

            if(cmds->output_file_des != -1){
                raise(SIGUSR2);
        

                //Register event
                if(cmds->log_file_des != -1) 
                    register_log(cmds->log_file_des, getpid(), "SIGNAL USR2");
            }

            print(file, cmds);
        }         
    }

    wait(NULL);
    return 0;
}

bool is_directory(const char *path)
{
    struct stat file_stat;
    if (lstat(path, &file_stat) != 0)
    {
        //if(ERRNO() == ENOENT)
        perror("Error reading file info.");
        return -1;
    }

    return S_ISDIR(file_stat.st_mode);
}
