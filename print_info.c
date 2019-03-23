#include "print_info.h"

int print(const char* file_path, struct commands *cmds)
{
    struct stat file_stat;

    //File name
    write(STDOUT_FILENO, file_path, strlen(file_path)); //File name

    //File type
    char file_type[20];
    get_file_type(file_path, file_type, cmds->log_file_des);
    write(STDOUT_FILENO, file_type, strlen(file_type)); 

    lstat(file_path, &file_stat);

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

    //File modified time
    char file_modified_time[30];
    format_date(file_stat.st_mtime, file_modified_time);
    write(STDOUT_FILENO, file_modified_time, strlen(file_modified_time));

    //File accessed time
    char file_accessed_time[30];
    format_date(file_stat.st_atime, file_accessed_time);
    write(STDOUT_FILENO, file_accessed_time, strlen(file_accessed_time));

    //Hash functions
    if(cmds->hash_commands != NULL)
    {
        char** hash_codes;
        hash_codes = (char**) malloc(sizeof(char) * 3);
        for(int i = 0; i < 3; i++)
            hash_codes[i] = (char*) malloc(sizeof(char) * 80);

        int functions = get_hash_codes(file_path, hash_codes, cmds);

        for(int i = 0; i < functions; i++)
            write(STDOUT_FILENO, hash_codes[i], strlen(hash_codes[i]));
    }

    write(STDOUT_FILENO, "\n", 1);


    //Register event
    if (cmds->log_file_des != -1)
    {
        char act[] = "ANALYZED ";
        strcat(act, file_path);
        register_log(cmds->log_file_des, getpid(), act);
    }

    return 0;
}

int get_file_type(const char* file_path, char* file_type, int log_file_des)
{
    int tmp_file_des, stdout_copy;
    pid_t pid;
    char tmp_file_name[] = "file_type.txt";

    tmp_file_des = open(tmp_file_name, O_WRONLY | O_CREAT, 0750);
    stdout_copy = dup(STDOUT_FILENO);
    dup2(tmp_file_des, STDOUT_FILENO);

    pid = fork();
    if(pid == 0) { //Child
        
        //Register event
        if(log_file_des != -1)
        {
            char act[] = "COMMAND file ";
            strcat(act, file_path);
            register_log(log_file_des, getpid(), act);
        }
            
        execlp("file", "file", file_path, NULL);
        perror("Error executing file command.");
        exit(1);
    }
    else {
        int status;
        wait(&status);
        if(WEXITSTATUS(status) != 0) exit(1);

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

int get_hash_codes(const char* file_path, char **hash_codes, struct commands *cmds)
{
    pid_t pid;
    int tmp_file_des, stdout_copy;
    char tmp_file_name[] = "hash_codes.txt";

    char hash_functions[3][8];
    int functions = 0;
    int j = 0;
    for(int i = 0; cmds->hash_commands[i] != '\0'; i++, j++)
    {
        if(cmds->hash_commands[i] == ',')
        {
            hash_functions[functions][j] = '\0';
            functions++;
            j = -1;
        }
        else {
            hash_functions[functions][j] = cmds->hash_commands[i];
        }
    }

    hash_functions[functions][j] = '\0';
    functions++;

    tmp_file_des = open(tmp_file_name, O_RDWR | O_CREAT, 0750);
    stdout_copy = dup(STDOUT_FILENO);
    dup2(tmp_file_des, STDOUT_FILENO);

    for(int i = 0; i < functions; i++)
    {
        pid = fork();
        if( pid == 0)
        {
            if(cmds->log_file_des != -1)
            {   
                //Register event
                char act[] = "COMMAND ";
                strcat(act, hash_functions[i]);
                strcat(act, " ");
                strcat(act, file_path);
            }

            strcat(hash_functions[i], "sum");
            execlp(hash_functions[i], hash_functions[i], file_path, NULL);
            perror("Error executing hash command.");
            exit(1);
        }
        else {
            int status;
            wait(&status);
            if(WEXITSTATUS(status) != 0) exit(1); 
        }
    }

    dup2(stdout_copy, STDOUT_FILENO);
    close(stdout_copy);
    close(tmp_file_des); 

    tmp_file_des = open(tmp_file_name, O_RDONLY);
    char ch;
    int i = 0, code = 0;
    bool reading_hash = true;

    while( read(tmp_file_des, &ch, 1) == 1) {
        if(ch == ' ') {
            reading_hash = false;
            hash_codes[code][i] = '\0';
        }
        else if(ch == '\n') {
            reading_hash = true;
            i = 0;
            code++;
        }

        else if(reading_hash)
        {
            if(i == 0) {
                hash_codes[code][i] = ',';
                i++;
            }
            hash_codes[code][i] = ch;
            i++;
        }
    }

    close(tmp_file_des);
    unlink(tmp_file_name);

    return functions;
}



