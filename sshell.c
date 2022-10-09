#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define CMDLINE_MAX 512
#define MAX_ARG 16
#define MAX_LENGTH 32

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

        /* Remove trailing newline from command line */
        nl = strchr(cmd, '\n');
        if (nl)
            *nl = '\0';

        int currentLetter = 0;
        int argumentCount = 0;
        int cmdLength = strlen(cmd);

        char **arg = malloc(MAX_ARG * sizeof(char *));
        for (int i = 0; i < MAX_ARG; ++i)
        {
            arg[i] = malloc(MAX_LENGTH * sizeof(char));
        }

        for (int i = 0; i < cmdLength; i++)
        {
            if (cmd[i] == ' ' || NULL)
            { // if cmd[i] is a space

                currentLetter = 0; // resets back to 0 for next word's first character
                argumentCount++;   // move to the next argument
            }
            else
            { // if cmd[i] is NOT a space

                arg[argumentCount][currentLetter] = cmd[i]; // add letter to the current argument
                currentLetter++;                            // move to the next letter
            }
        }

        // Assigns "NULL" to remaining argument slots
        if (argumentCount < MAX_ARG)
        {
            for (int i = argumentCount + 1; i <= (MAX_ARG - argumentCount); i++)
            {
                arg[i] = NULL;
            }
        }
        // prints current word
        for (int i = 0; i <= MAX_ARG; i++)
        {
            printf("Argument %d: %s\n", i, arg[i]);
        }
        /* Print command line if stdin is not provided by terminal */
        if (!isatty(STDIN_FILENO))
        {
            printf("%s", cmd);
            fflush(stdout);
        }

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
        pid_t sshellPid = fork(); // forking to keep the sshell running
        if (sshellPid == 0)       // child process to wait for its child to run command and return it back to display on terminal
        {
            pid_t pid = fork(); // forking to run the command
            if (pid == 0)
            {
                printf("pid = %d\n", pid);
                int evpReturn = execvp(arg[0], arg);

                // free argument variable
                for (int i = 0; i < 16; ++i)
                {
                    free(arg[i]);
                }
                free(arg);
                if (evpReturn < 0)
                {
                    perror("execvp");
                    exit(1);
                }
                else
                {
                    exit(0);
                    // exit(EXIT_FAILURE);
                }
            }
            else if (pid > 0)
            {
                printf("pid = %d\n", pid);
                int status;
                waitpid(pid, &status, 0);
                fprintf(stderr, "+ completed '%s' [%d]\n", cmd, WEXITSTATUS(status));
            }
            else
            {
                perror("fork");
                exit(0);
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
            exit(0);
        }
    }

    return EXIT_SUCCESS;
}
