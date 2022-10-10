#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

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
        else if (cmd[i] == '>')
        {
            lengthSoFar = i;
            break;
        }
        else
        {                                                   // if cmd[i] is NOT a space
            cl->arg[argumentCount][currentLetter] = cmd[i]; // add letter to the current argument
            currentLetter++;                                // move to the next letter
        }
    }
    // printf("Argument Count: %d\n", argumentCount);
    /*
        take fileName from the argument list
        then later assign NULL to any arguments after '>'
    */
    if (lengthSoFar != cmdLength && strstr(cmd, ">"))
    {
        int outputArg = argumentCount + 1;
        currentLetter = 0;
        // printf("cmdLine contains '>'\n");
        // printf("%c\n", cmd[lengthSoFar]);
        for (int i = lengthSoFar + 1; i < cmdLength; i++)
        {
            while (cmd[i] == ' ') // if cmd[i] is a space
            {
                i++;
            }
            // printf("%c\n", cmd[i]);
            cl->arg[outputArg][currentLetter] = cmd[i];
            currentLetter++;
        }
        // printf("%s\n", cl->arg[outputArg]);
        cl->fileName = cl->arg[outputArg];
        // printf("%s\n", cl->fileName);
    }

    // Assigns "NULL" to remaining argument slots
    if (argumentCount < MAX_ARG)
    {
        for (int i = argumentCount; i <= (MAX_ARG - argumentCount); i++)
        {
            cl->arg[i] = NULL;
        }
    }
}

/*
    function for output redirection:
    1. open file by the filename
    2. take the output of the command before ">"
    3. write the output to the file
    4. close the file
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

        parseCommandLine(cl, cmd);
        printf("File Name: %s\n", cl->fileName);
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
            // fprintf(stderr, "Bye...\n");
            fprintf(stderr, "Bye...\n+ completed '%s' [%d]\n", cmd, 0); // EXITSTATUS NEEDED
            break;
        }
        else if (!strcmp(cmd, "pwd"))
        {
            printf("IN BUILTIN\n");
            char *current_directory = get_current_dir_name();
            printf("%s\n", current_directory);
            fprintf(stderr, "Bye...\n+ completed '%s' [%d]\n", cmd, 0); // EXITSTATUS NEEDED
            continue;
        }
        else if (strstr(cmd, "cd"))
        {
            // int chdir(const char *filename)
            char *new_directory = cl->arg[1];
            chdir(new_directory);
            perror("cd");
            fprintf(stderr, "Bye...\n+ completed '%s' [%d]\n", cmd, 0); // EXITSTATUS NEEDED

            continue;
        }

        /* Regular command */
        pid_t sshellPid = fork(); // forking to keep the sshell running
        if (sshellPid == 0)       // child process to wait for its child to run command and return it back to display on terminal
        {
            pid_t pid = fork(); // forking to run the command
            if (pid == 0)
            {
                // printf("pid = %d\n", pid);
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
