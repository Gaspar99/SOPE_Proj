#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <stdbool.h>

int main(int argc, char *argv[], char *envp[])
{
    int tmp_file_des;
    struct stat file_stat;
    pid_t pid;
    
    tmp_file_des = open("file_type.txt", O_RDWR | O_CREAT, 0750);
    int stdout_copy = dup(STDOUT_FILENO);
    dup2(tmp_file_des, STDOUT_FILENO);

    pid = fork();
    if(pid == 0) { //Child
        execlp("file", "file", argv[1], NULL);
        perror("Error executing file command.");
        exit(1);
    }
    else {
        wait(NULL);
        dup2(stdout_copy, STDOUT_FILENO);
        close(stdout_copy);
    }

    char ch;
    char file_type[25];
    int i = 0;
    bool reading_file_type = false;
    while( read(tmp_file_des, &ch, 1) == 1) {
        printf("1");
        if (ch == '\n' || ch == ',' || ch == 0x0) {
            file_type[i] = '\0';
            reading_file_type = false;
            break;
        }

        if (ch == ':') {
            reading_file_type = true;
            continue;
        }
        if(reading_file_type) {
            file_type[i] = ch;
            i++;
        }
    }

    printf("%s,", argv[1]); //File name
    //printf("%s,", file_type); //File type

    stat(argv[1], &file_stat);
    printf("%d,", (int) file_stat.st_size); //Size

    //File access
    char file_access[5];
    int j = 0;
    if(file_stat.st_mode & S_IRUSR) { file_access[j] = 'r'; j++; }
    if(file_stat.st_mode & S_IWUSR) { file_access[j] = 'w'; j++; }
    if(file_stat.st_mode & S_IXUSR) { file_access[j] = 'x'; j++; }
    file_access[j] = ','; j++;
    file_access[j] = '\0';
    printf("%s", file_access);

    printf("%s,", ctime(&file_stat.st_ctime));//File created time??
    printf("%s,", ctime(&file_stat.st_mtime));//File modified time

    exit(0);
}

