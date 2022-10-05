#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define CMDLINE_MAX 512

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
        // printf("%s\n", cmd);
        // char cmdcopy[CMDLINE_MAX] = cmd;
        // char *arguments;
        // arguments = strtok(cmd, " ");
        //  char arguments[16][32];
        //  char *token = strtok(cmd," ");
        //  int tokencounter = 0;
        //  while(!token){
        //          *arguments[tokencounter] = *token;
        //          token = strtok(NULL," ");
        //          tokencounter++;
        //  }
        //  for(int i = 0; i < tokencounter; i++){
        //          printf("%s\n", arguments[i]);
        //  }

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
                char *argu[] = {"ECS150", NULL};
                // int execvp(const char *file, char *const argv[]);
                // int execv(const char *path, char *const argv[]);
                execvp(cmd, argu); // execvp: No such file or directory
                                   //   + completed 'echo ECS150' [1]
                perror("execvp");  // CHANGE THIS
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
        Phase 1:
        pid_t sshellPid = fork(); // forking to keep the sshell running
        if (sshellPid == 0) // child process to wait for its child to run command and return it back to display on terminal
        {
                pid_t pid = fork(); // forking to run the command
                if (pid == 0)
                {
                        execvp(cmd, "ECS150");
                        perror("execvp"); //CHANGE THIS
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
                        exit(2)
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
                exit(3)
        }




        pid_t pid = fork();
        if (pid == 0) {
        execv(cmd, args);
        perror("execv");
        exit(1);
        } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        printf("Child returned %d\n",
               WEXITSTATUS(status));
        }
        else {
        perror("fork");
        exit(1);
        }
*/
