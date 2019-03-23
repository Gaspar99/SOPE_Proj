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
        sprintf(file, "%s/%s", path, direntp->d_name);
        
        if (is_directory(file)) {
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
        else
            print(file, cmds);
    }

    return 0;
}

int is_directory(const char *path)
{
    struct stat file_stat;
    if (lstat(path, &file_stat) != 0)
    {
        perror("Error reading file info.");
        return -1;
    }

    return S_ISDIR(file_stat.st_mode);
}