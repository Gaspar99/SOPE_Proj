#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>

int is_directory(const char *path);

int main(int argc, char *argv[], char *envp[])
{
    int file_des;
    pid_t pid;
    struct stat file_stat;

    if (argc < 2 || argc > 9) {
        printf("usage: %s dirname\n", argv[0]);
        exit(1);
    }

    if(!is_directory(argv[argc - 1])) {
        if( (file_des = open(argv[argc - 1], O_RDONLY) == -1) )  {
            perror("File does not exist.");
            exit(1);
        }

        pid = fork();
        if(pid == 0)  {//Child
            execlp("./print_info", "./print_info", argv[argc - 1], NULL);
            perror("Error!");
            exit(1);
        }
        else { //Parent
            wait(NULL);
            exit(0);
        }
    }
}

int is_directory(const char *path)
{
    struct stat file_stat;
    if (stat(path, &file_stat) != 0) {
        perror("Error reading file info.");
        return -1;
    }

    return S_ISDIR(file_stat.st_mode);
}

