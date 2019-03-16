#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

int get_file_type(char* file_name, char* file_type);
int format_date(time_t time, char* formated_date);

int main(int argc, char *argv[], char *envp[])
{
    struct stat file_stat;

    //File name
    char file_name[25];
    strcpy(file_name, argv[1]);
    write(STDOUT_FILENO, file_name, strlen(file_name)); //File name

    //File type
    char file_type[20];
    get_file_type(argv[1], file_type);
    write(STDOUT_FILENO, file_type, strlen(file_type)); 

    stat(argv[1], &file_stat);

    //File size
    char file_size[11] = ",";
    char size[10];
    sprintf(size, "%d", (int) file_stat.st_size);
    strcat(file_size, size);
    write(STDOUT_FILENO, file_size, strlen(file_size));

    //File access
    char file_access[5];
    int j = 0;
    file_access[j++] = ',';
    if(file_stat.st_mode & S_IRUSR) file_access[j++] = 'r';
    if(file_stat.st_mode & S_IWUSR) file_access[j++] = 'w';
    if(file_stat.st_mode & S_IXUSR) file_access[j++] = 'x';
    file_access[j] = '\0';
    write(STDOUT_FILENO, file_access, strlen(file_access));

    //File changes permissions time
    char file_creation_time[30];
    format_date(file_stat.st_ctime, file_creation_time);
    write(STDOUT_FILENO, file_creation_time, strlen(file_creation_time));

    //File modified time
    char file_modified_time[30];
    format_date(file_stat.st_mtime, file_modified_time);
    write(STDOUT_FILENO, file_modified_time, strlen(file_modified_time));

    write(STDOUT_FILENO, "\n", 1);
    exit(0);
}

int get_file_type(char* file_name, char* file_type)
{
    int tmp_file_des, stdout_copy;
    pid_t pid;
    char tmp_file_name[] = "file_type.txt";

    tmp_file_des = open(tmp_file_name, O_WRONLY | O_CREAT, 0750);
    stdout_copy = dup(STDOUT_FILENO);
    dup2(tmp_file_des, STDOUT_FILENO);

    pid = fork();
    if(pid == 0) { //Child
        execlp("file", "file", file_name, NULL);
        perror("Error executing file command.");
        exit(1);
    }
    else {
        wait(NULL);
        dup2(stdout_copy, STDOUT_FILENO);
        close(stdout_copy);
        close(tmp_file_des);
    }

    tmp_file_des = open(tmp_file_name, O_RDONLY);
    char ch;
    int i = 0;
    bool reading_file_type = false;
    while( read(tmp_file_des, &ch, 1) == 1) {
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

    file_type[0] = ',';

    close(tmp_file_des);
    unlink(tmp_file_name);

    return 0;
}

int format_date(time_t time, char* formated_date)
{
    struct tm* date_time;

    date_time = localtime(&time);

    char year[5];
    sprintf(year, "%d", date_time->tm_year + 1900);

    char month[3];
    sprintf(month, "%d", date_time->tm_mon + 1);

    char day[3];
    sprintf(day, "%d", date_time->tm_mday);

    char hour[3];
    sprintf(hour, "%02d", date_time->tm_hour);

    char min[3];
    sprintf(min, "%02d", date_time->tm_min);

    char sec[3];
    sprintf(sec, "%02d", date_time->tm_sec);

    strcpy(formated_date, ",");
    strcat(formated_date, year);
    strcat(formated_date, "-");
    strcat(formated_date, month);
    strcat(formated_date, "-");
    strcat(formated_date, day);
    strcat(formated_date, "T");

    strcat(formated_date, hour);
    strcat(formated_date, ":");
    strcat(formated_date, min);
    strcat(formated_date, ":");
    strcat(formated_date, sec);

    return 0;
}

