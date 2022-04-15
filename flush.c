#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


# define MAX_SIZE 300
# define MAX_STRING_LEN 10

void set_cwd(char* cwd)
{
    char cwd_array[200];

    if (getcwd(cwd_array, sizeof(cwd_array)) != NULL)
    {
        strcpy(cwd, cwd_array);
        // printf("%s \n", cwd);
    }
    else
    {
        printf("getcwd() error");
    }
}

void prompt_user(char * cwd, char * input)
{
    // bzero((char *)input, MAX_SIZE);
    printf("%s: ", cwd);
    
    if (fgets(input, MAX_SIZE, stdin) == NULL){
        printf("\n");
        exit(0);
    }
    //Remove newline character from input

    if (input[0] != '\n'){
        input[strcspn(input, "\n")] = 0;
    }
}

void redirection(char *args[], char *cwd){
    int fd;
    int i = 0;
    int redirect = 0;
    char absolute_path_to_file[MAX_SIZE];
    memset(absolute_path_to_file, '\0', sizeof(absolute_path_to_file));
    while (args[i] != NULL){
        if (strcmp(args[i],">") == 0 || strcmp(args[i],"<") == 0){
            strcat(absolute_path_to_file, cwd);
            strcat(absolute_path_to_file, "/");
            strcat(absolute_path_to_file, args[i+1]);
            redirect = 1;
            if (strcmp(args[i], ">") == 0){
                fd = open(absolute_path_to_file, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
                if(fd < 0){
                    perror("Noe gikk galt kan ikke aapne fil");
                }
                printf("%d\n", fd);
                dup2(fd, 1);
                close(fd);
                break;
            }
            if (strcmp(args[i], "<") == 0){
                // Tatt fra stackoverflow
                fd = open(absolute_path_to_file, O_RDONLY);
                if (fd < 0){
                    perror("Cannot open file");
                    exit(0);
                }
                dup2(fd, 0);
                close(fd);
                break;
            }
        }
        i++;
    }
    //Slicer slik at man faar inn riktig argumenter i execvp
    if (redirect){
        for (int j = i; j < MAX_STRING_LEN; j++){
            args[j] = '\0';
        }
    }
}


void split_string(char *str, char *args[], char *cwd)
{

    int counter = 1;
    char delim[] = " \t";
    char *ptr = strtok(str, delim);
    args[0] = ptr;
    while (ptr != NULL)
    {  
        ptr = strtok(NULL, delim);
        
        args[counter] = ptr;
        counter++;
    }
    if (strcmp(args[0], "cd") == 0)
    {
        chdir(args[1]);
    };
    
    // Maa ende paa null men ser ut til at det skjer uansett
    // args[counter] = NULL;
}


int flush()
{
    char* cwd = malloc(MAX_SIZE);
    char* input = malloc(MAX_SIZE);
    char* input_pointer_array[3];
    while(1)
    {
        set_cwd(cwd);
        prompt_user(cwd, input);
        char *args[MAX_STRING_LEN];
        split_string(input, args, cwd);
        int pid = fork();
        
        
        // child
        if (pid == 0){
            redirection(args, cwd);
            execvp(args[0], args);
            exit(0);
        }

        // parent waiting for child
        else{
            int status;
            waitpid(pid, &status, 0);
            printf("Parent: %d \n", pid);
            if(WIFEXITED(status)){
                int exit_status = WEXITSTATUS(status);
                printf("Exit status = %d \n", exit_status);
            }
        }
    }
}