#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define CMDLINE_MAX 512
#define MAX_ARG 16
#define MAX_LENGTH 32

struct CommandLine
{
    char **arg;
    char *fileName;
};

void parseCommandLine(struct CommandLine *cl, char *cmd)
{

    int currentLetter = 0;
    int argumentCount = 0;
    int cmdLength = strlen(cmd);
    int lengthSoFar = 0;

    for (int i = 0; i < cmdLength; i++)
    {
        if (cmd[i] == ' ' || NULL)
        { // if cmd[i] is a space
            while (i < cmdLength - 1 && cmd[i + 1] == ' ')
            {
                i++;
            }
            currentLetter = 0; // resets back to 0 for next word's first character
            argumentCount++;   // move to the next argument
        }
        else if (cmd[i] == '>' || cmd[i] == '<')
        {
            if (cmd[i - 1] != ' ')
            {
                argumentCount++;
            }
            if (cmd[i + 1] != ' ')
            {
                argumentCount++;
            }
            /*
                take fileName from the argument list
                then later assign NULL to any arguments after '>'
            */
            lengthSoFar = i;
            if (lengthSoFar != cmdLength && (strstr(cmd, ">") || strstr(cmd, "<")))
            {
                int outputArg = argumentCount + 1;
                currentLetter = 0;
                for (int j = lengthSoFar + 1; j < cmdLength; j++)
                {
                    while (cmd[j] == ' ') // if cmd[i] is a space
                    {
                        j++;
                    }
                    cl->arg[outputArg][currentLetter] = cmd[j];
                    currentLetter++;
                }
                cl->fileName = cl->arg[outputArg];
            }
        }
        else
        {                                                   // if cmd[i] is NOT a space
            cl->arg[argumentCount][currentLetter] = cmd[i]; // add letter to the current argument
            currentLetter++;                                // move to the next letter
        }
    }
    // printf("Argument Count is: %d\n", argumentCount);
    //  Assigns "NULL" to remaining argument slots
    /* THIS PART MAY BE A PROBLEM IF WE ARE DEALING WITH ARGUMENTS AFTER FILENAME*/
    if (argumentCount < MAX_ARG && (strstr(cmd, ">") || strstr(cmd, "<")))
    {
        for (int i = argumentCount - 1; i <= (MAX_ARG - argumentCount); i++) // FIX THE LAST NULL ARGUMENTS ASSIGNING
        {
            cl->arg[i] = NULL;
        }
    }

    if (argumentCount < MAX_ARG)
    {
        for (int i = argumentCount + 1; i <= (MAX_ARG - argumentCount); i++)
        {
            cl->arg[i] = NULL;
        }
    }
}

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

        struct CommandLine *cl = malloc(sizeof(struct CommandLine));
        cl->arg = malloc(MAX_ARG * sizeof(char *));
        for (int i = 0; i < MAX_ARG; ++i)
        {
            cl->arg[i] = malloc(MAX_LENGTH * sizeof(char));
        }
        cl->fileName = malloc(MAX_LENGTH * sizeof(char *));
        cl->fileName = NULL;
        parseCommandLine(cl, cmd);
        // printf("File Name: %s\n", cl->fileName);

        // // prints current word
        // for (int i = 0; i < MAX_ARG; i++)
        // {
        //     printf("Argument %d: %s\n", i, cl->arg[i]);
        // }

        /* Print command line if stdin is not provided by terminal */
        if (!isatty(STDIN_FILENO))
        {
            printf("%s", cmd);
            fflush(stdout);
        }

        /* Builtin command */
        if (!strcmp(cmd, "exit"))
        {
            fprintf(stderr, "Bye...\n+ completed '%s' [%d]\n", cmd, 0); // EXITSTATUS NEEDED
            break;
        }
        else if (!strcmp(cmd, "pwd"))
        {
            char *current_directory = get_current_dir_name();
            printf("%s\n", current_directory);
            fprintf(stderr, "+ completed '%s' [%d]\n", cmd, 0); // EXITSTATUS NEEDED
            continue;
        }
        else if (strstr(cmd, "cd"))
        {
            char *new_directory = cl->arg[1];
            chdir(new_directory);
            perror("cd");
            fprintf(stderr, "+ completed '%s' [%d]\n", cmd, 0); // EXITSTATUS NEEDED

            continue;
        }

        /* Regular command */
        pid_t sshellPid = fork(); // forking to keep the sshell running
        if (sshellPid == 0)       // child process to wait for its child to run command and return it back to display on terminal
        {
            pid_t pid = fork(); // forking to run the command
            if (pid == 0)
            {
                /* OUTPUT REDIRECTION */
                if (cl->fileName != NULL && strstr(cmd, ">"))
                {
                    int fd = open(cl->fileName, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                }
                /* INPUT REDIRECTION */
                if (cl->fileName != NULL && strstr(cmd, "<"))
                {
                    int fd = open(cl->fileName, O_RDONLY, 0644);
                    dup2(fd, STDIN_FILENO);
                    close(fd);
                }
                int evpReturn = execvp(cl->arg[0], cl->arg);

                // free argument variable
                for (int i = 0; i < 16; ++i)
                {
                    free(cl->arg[i]);
                }
                free(cl->arg);
                free(cl->fileName);
                free(cl);

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
                // printf("pid = %d\n", pid);
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
