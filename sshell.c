#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define CMDLINE_MAX 512
/*
char** parse(char* cmd){
        char **argu;
        int currentLetter = 0;
        int argumentCount = 0;

        for(int i = 0; i < CMDLINE_MAX; i++){
                if(cmd[i] == ' '){ //if cmd[i] is a space
                        //printf("currentword: %s\n", argu[i]);
                        currentLetter = 0; // resets back to 0 for next word's first character
                        argumentCount++; // move to the next argument
                }
                else{ // if cmd[i] is NOT a space
                        argu[argumentCount][currentLetter] = cmd[i]; // add letter to the current argument
                        currentLetter++; //move to the next letter
                }
        }
        return argu**;
}
*/

int main(void)
{
    char cmd[CMDLINE_MAX];

    while (1)
    {
        char *nl;
        // int retval;

        /* Print prompt */
        printf("sshell$ ");
        fflush(stdout);

        /* Get command line */
        fgets(cmd, CMDLINE_MAX, stdin);
        //char **argu = parse(cmd);
        char arg[16][32];
        int currentLetter = 0;
        int argumentCount = 0;
        int cmdLength = strlen(cmd);

        for(int i = 0; i < cmdLength; i++){
                if(cmd[i] == ' ')
                { //if cmd[i] is a space
                        // prints current word
                        for (int i = 0; i < currentLetter; i++) 
                        {     
                                printf("%c", arg[argumentCount][i]);

                        }
                        printf("\n");
                        currentLetter = 0; // resets back to 0 for next word's first character
                        argumentCount++; // move to the next argument
                }
                else{ // if cmd[i] is NOT a space
                        arg[argumentCount][currentLetter] = cmd[i]; // add letter to the current argument
                        currentLetter++; //move to the next letter
                       
                }
                // printf("argumentCount: %d\n", argumentCount);
                // printf("currentLetter: %d\n", currentLetter);
        }
         
        /* Print command line if stdin is not provided by terminal */
        if (!isatty(STDIN_FILENO))
        {
            printf("%s", cmd);
            fflush(stdout);
        }

        /* Remove trailing newline from command line */
        nl = strchr(cmd, '\n');
        if (nl)
            *nl = '\0';

        /* Builtin command */
        if (!strcmp(cmd, "exit"))
        {
            fprintf(stderr, "Bye...\n");
            break;
        }
        else if (!strcmp(cmd, "pwd"))
        {
        }
        else if (!strcmp(cmd, "cd"))
        {
        }

        /* Regular command */
        // retval = system(cmd);
        // fprintf(stdout, "Return status value for '%s': %d\n",
        //         cmd, retval);
        pid_t sshellPid = fork(); // forking to keep the sshell running
        if (sshellPid == 0)       // child process to wait for its child to run command and return it back to display on terminal
        {
            pid_t pid = fork(); // forking to run the command
            if (pid == 0)
            {
                //char *argu[] = {"ECS150", NULL};
                // int execvp(const char *file, char *const argv[]);
                printf("%s\n", "Ignore");
                //execvp(argu[0], NULL); // execvp: No such file or directory
                                   //   + completed 'echo ECS150' [1]
                //perror("execvp");  // CHANGE THIS
                exit(1);
            }
            else if (pid > 0)
            {
                int status;
                waitpid(pid, &status, 0);
                printf("+ completed '%s' [%d]\n", cmd, WEXITSTATUS(status));
            }
            else
            {
                perror("fork");
                exit(2);
            }
        }
        else if (sshellPid > 0)
        {
            int status;
            waitpid(sshellPid, &status, 0); // wait for its child to return back with value to print on terminal
            break;
        }
        else
        {
            perror("fork");
            exit(3);
        }
    }

    return EXIT_SUCCESS;
}
/*
        PHASE 1: Pseudocode
        1. 
*/
