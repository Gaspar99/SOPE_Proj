#include "print_info.h"

int print(const char* file_path, struct commands *cmds)
{
    struct stat file_stat;
    char file_info[400];

    //File name
    char* file_name;
    file_name = (char*) malloc(sizeof(char) * 100);
    strcpy(file_name, file_path);
    while( *file_name++ != '/');

    //File type
    char file_type[50];
    get_file_type(file_path, file_type, cmds);

    lstat(file_path, &file_stat);

    //File size
    int file_size = (int) file_stat.st_size;

    //File access
    char file_access[5];
    int j = 0;
    if(file_stat.st_mode & S_IRUSR) file_access[j++] = 'r';
    if(file_stat.st_mode & S_IWUSR) file_access[j++] = 'w';
    if(file_stat.st_mode & S_IXUSR) file_access[j++] = 'x';
    file_access[j] = '\0';

    //File modified time
    char file_modified_time[30];
    format_date(file_stat.st_mtime, file_modified_time);

    //File accessed time
    char file_accessed_time[30];
    format_date(file_stat.st_atime, file_accessed_time);

    sprintf(file_info, "%s,%s,%d,%s,%s,%s", file_name, file_type, file_size, file_access, file_modified_time, file_accessed_time);

    //Hash functions
    if(cmds->hash_commands != NULL)
    {
        char** hash_codes;
        hash_codes = (char**) malloc(sizeof(char) * 3);
        for(int i = 0; i < 3; i++)
            hash_codes[i] = (char*) malloc(sizeof(char) * 80);

        int functions = get_hash_codes(file_path, hash_codes, cmds);

        for(int i = 0; i < functions; i++)
            strcat(file_info, hash_codes[i]);

        for(int i = 0; i < functions; i++)
            free(hash_codes[i]);

        free(hash_codes); 
    }

    //Register event: finished analyse of file
    if (cmds->log_file_des != -1)
    {
        char act[] = "ANALYZED ";
        strcat(act, file_path);
        register_log(cmds->log_file_des, getpid(), act);
    }

    strcat(file_info, "\n");
    write(STDOUT_FILENO, file_info, strlen(file_info));

    //Register event: printed file info
    if (cmds->log_file_des != -1)
    {
        char act[] = "PRINTED ";
        strcat(act, file_path);
        register_log(cmds->log_file_des, getpid(), act);
    }

    if(sigint_received()) {
        if(cmds->output_file_des != -1) close_output_file(cmds);
        if(cmds->log_file_des != -1) close_logs_file(cmds->log_file_des, cmds->log_file_name);
        exit(0);
    } 

    return 0;
}

int get_file_type(const char* file_path, char* file_type, struct commands *cmds)
{
    int tmp_file_des, stdout_copy;
    pid_t pid;
    char tmp_file_name[100];
    sprintf(tmp_file_name, "file_type%d.txt", file_hash(file_path));

    tmp_file_des = open(tmp_file_name, O_RDWR | O_CREAT, 0750);
    stdout_copy = dup(STDOUT_FILENO);
    dup2(tmp_file_des, STDOUT_FILENO);

    pid = fork();
    if(pid == 0) { //Child
        
        //Register event
        if(cmds->log_file_des != -1)
        {
            char act[] = "COMMAND file ";
            strcat(act, file_path);
            register_log(cmds->log_file_des, getpid(), act);
        }
            
        execlp("file", "file", file_path, NULL);
        perror("Error executing file command.");
        exit(1);
    }
    else if(pid > 0) {
        int status;
        wait(&status);
        if(WEXITSTATUS(status) == 1) exit(1);

        dup2(stdout_copy, STDOUT_FILENO);
        close(stdout_copy);
        lseek(tmp_file_des, 0, SEEK_SET);
    }
    else perror("Fork error.");
    
    char ch;
    int i = 0;
    bool reading_file_type = false;
    while( read(tmp_file_des, &ch, 1) == 1) {
        if (ch == '\n' || ch == ',') {
            file_type[i] = '\0';
            reading_file_type = false;
            break;
        }

        if (ch == ' ' && !reading_file_type) {
            reading_file_type = true;
            continue;
        }
        if(reading_file_type) {
            file_type[i] = ch;
            i++;
        }
    }

    close(tmp_file_des);
    unlink(tmp_file_name);

    if(sigint_received()) {
        if(cmds->output_file_des != -1) close_output_file(cmds);
        if(cmds->log_file_des != -1) close_logs_file(cmds->log_file_des, cmds->log_file_name);
        exit(0);
    } 

    return 0;
}

int format_date(time_t time, char* formated_date)
{
    struct tm* date_time;
    date_time = localtime(&time);

    sprintf(formated_date, 
    "%d-%d-%dT%02d:%02d:%02d", 
    date_time->tm_year + 1900, date_time->tm_mon + 1, date_time->tm_mday, 
    date_time->tm_hour, date_time->tm_min, date_time->tm_sec);

    return 0;
}

int get_hash_codes(const char* file_path, char **hash_codes, struct commands *cmds)
{
    pid_t pid;
    int tmp_file_des, stdout_copy;
    char tmp_file_name[100];
    sprintf(tmp_file_name, "hash_codes%d.txt", file_hash(file_path));

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
            if(WEXITSTATUS(status) == 1) exit(1); 
        }
    }

    dup2(stdout_copy, STDOUT_FILENO);
    close(stdout_copy);
    lseek(tmp_file_des, 0, SEEK_SET);

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

    if(sigint_received()) {
        if(cmds->output_file_des != -1) close_output_file(cmds);
        if(cmds->log_file_des != -1) close_logs_file(cmds->log_file_des, cmds->log_file_name);
        exit(0);
    } 

    return functions;
}

int file_hash(const char* file_path)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *file_path++))
        hash = ((hash << 5) + hash) + c; 

    return hash;
}



