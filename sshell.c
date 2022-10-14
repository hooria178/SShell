#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


#define CMDLINE_MAX 512
#define MAX_ARG 16
#define MAX_LENGTH 32
#define NUM_PIPES 4

/* ERROR MACROS */
enum errornum
{
    TOO_MANY_ARGS,
    MISSING_CMD,
    NO_OUTPUT_FILE,
    UNABLE_TO_OPEN_FILE,
    MIS_OUT_REDIR,
    DIR_NOT_EXIST,
    CMD_NOT_FOUND,
    NO_INPUT_FILE,
    STDIN_FILE_NOT_EXIST,
    MIS_IN_REDIR
};

/* FUNCTION TO CHECK ERROR STATUS */
void error_checking(enum errornum error_number)
{
    switch (error_number)
    {
    case TOO_MANY_ARGS:
        fprintf(stderr, "Error: too many process arguments\n");
        break;
    case MISSING_CMD:
        fprintf(stderr, "Error: missing command\n");
        break;
    case NO_OUTPUT_FILE:
        fprintf(stderr, "Error: no output file\n");
        break;
    case UNABLE_TO_OPEN_FILE:
        fprintf(stderr, "Error: cannot open output file\n");
        break;
    case MIS_OUT_REDIR:
        fprintf(stderr, "Error: mislocated output redirection\n");
        break;
    case DIR_NOT_EXIST:
        fprintf(stderr, "Error: cannot cd into directory\n");
        break;
    case CMD_NOT_FOUND:
        fprintf(stderr, "Error: command not found\n");
        break;
    case NO_INPUT_FILE:
        fprintf(stderr, "Error: no input file\n");
        break;
    case STDIN_FILE_NOT_EXIST:
        fprintf(stderr, "Error: cannot open input file\n");
        break;
    case MIS_IN_REDIR:
        fprintf(stderr, "Error: mislocated input redirection\n");
        break;
    }
}

/* STRUCT TO CONTAIN COMMAND LINE VARIABLES */
struct CommandLine
{
    char **arg;
    char *cmd;
    char *fileName;
};

/* PARSING FUNCTION */
void parseCommandLine(struct CommandLine *cl)
{

    int currentLetter = 0;
    int argumentCount = 0;
    int cmdLength = strlen(cl->cmd);
    int lengthSoFar = 0;

    /* GOES THROUGH THE COMMAND LINE BY EACH CHARACTER TO EXTRACT ARGUMENTS AND IF APPLICABLE A FILENAME */
    for (int i = 0; i < cmdLength; i++)
    {

        if (cl->cmd[i] == ' ' || NULL)
        {
            while (i < cmdLength - 1 && cl->cmd[i + 1] == ' ')
            {
                i++;
            }
            currentLetter = 0; // resets back to 0 for next word's first character
            argumentCount++;   // move to the next argument
        }
        /* IF CMDLINE CONTAINS ">" OR "<", THEN GETS A FILENAME*/
        else if (cl->cmd[i] == '>' || cl->cmd[i] == '<')
        {
            if (cl->cmd[i - 1] != ' ')
            {
                argumentCount++;
            }
            if (cl->cmd[i + 1] != ' ')
            {
                argumentCount++;
            }
            /*
                take fileName from the argument list
                then later assign NULL to any arguments after '>'
            */
            lengthSoFar = i;
            if (lengthSoFar != cmdLength && (strstr(cl->cmd, ">") || strstr(cl->cmd, "<")))
            {
                int outputArg = argumentCount + 1;
                currentLetter = 0;
                for (int j = lengthSoFar + 1; j < cmdLength; j++)
                {
                    while (cl->cmd[j] == ' ') // if cmd[i] is a space
                    {
                        j++;
                    }
                    cl->arg[outputArg][currentLetter] = cl->cmd[j];
                    currentLetter++;
                }
                cl->fileName = cl->arg[outputArg]; 
            }
        }
        else
        {                                                       // if cmd[i] is NOT a space
            cl->arg[argumentCount][currentLetter] = cl->cmd[i]; // add letter to the current argument
            currentLetter++;                                    // move to the next letter
        }
    }

    //  Assigns "NULL" to remaining argument slots
    if (argumentCount < MAX_ARG && (strstr(cl->cmd, ">") || strstr(cl->cmd, "<")))
    {
        for (int i = argumentCount - 1; i <= (MAX_ARG - argumentCount); i++)
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
    char cmdln[CMDLINE_MAX];

    while (1)
    {
        char *nl;

        /* Print prompt */
        printf("sshell$ ");
        fflush(stdout);

        /* Get command line */
        fgets(cmdln, CMDLINE_MAX, stdin);

        /* Print command line if stdin is not provided by terminal */
        if (!isatty(STDIN_FILENO))
        {
            printf("%s", cmdln);
            fflush(stdout);
        }

        /* Remove trailing newline from command line */
        nl = strchr(cmdln, '\n');
        if (nl)
            *nl = '\0';

        /* Allocate space for the maximum number of pipes */
        struct CommandLine *cl[NUM_PIPES];
        for (int i = 0; i < NUM_PIPES; i++)
        {
            cl[i] = malloc(sizeof(struct CommandLine));
            cl[i]->arg = malloc(MAX_ARG * sizeof(char *));
            for (int j = 0; j < MAX_ARG; ++j)
            {
                cl[i]->arg[j] = malloc(MAX_LENGTH * sizeof(char));
            }
            cl[i]->cmd = malloc(CMDLINE_MAX * sizeof(char));
            cl[i]->fileName = malloc(MAX_LENGTH * sizeof(char));
            cl[i]->fileName = NULL;
        }

        // Separate the command line
        char *token;
        token = strtok(cmdln, "|");
        int numCommands = 0;
        while (token)
        {
            cl[numCommands]->cmd = token;
            numCommands++;
            token = strtok(NULL, "|");
        }

        /* CALLS FUNCTION TO PARSE THE COMMAND LINE */
        for (int i = 0; i < numCommands; i++)
        {
            parseCommandLine(cl[i]);
        }

        /* Builtin command */
        if (!strcmp(cmdln, "exit")) /* EXIT COMMAND */
        {
            fprintf(stderr, "Bye...\n+ completed '%s' [%d]\n", cmdln, 0); 
            break;
        }
        else if (!strcmp(cmdln, "pwd")) /* PWD COMMAND */
        {
            char *current_directory = get_current_dir_name();
            printf("%s\n", current_directory);
            fprintf(stderr, "+ completed '%s' [%d]\n", cmdln, 0);
            continue;
        }
        else if (strstr(cmdln, "cd")) /* CD COMMAND */
        {
            char *new_directory = cl[0]->arg[1];
            chdir(new_directory);
            fprintf(stderr, "+ completed '%s' [%d]\n", cmdln, 0); 
            continue;
        }

        /* Regular command */
        pid_t sshellPid = fork(); // forking to keep the sshell running
        if (sshellPid == 0)       // child process to wait for its child to run command and return it back to display on terminal
        {
            if (numCommands == 2)
            {
                int fd[2];
                pipe(fd);
                pid_t pid1 = fork();
                if (pid1 == 0)
                {
                    close(fd[0]);
                    dup2(fd[1], STDOUT_FILENO);
                    close(fd[1]);
                    int evpReturn = execvp(cl[0]->arg[0], cl[0]->arg);
                    if (evpReturn < 0)
                    {
                        perror("execvp");
                        exit(1);
                    }
                    else
                    {
                        exit(0);
                    }
                }
                else
                {
                    int status;
                    waitpid(pid1, &status, 0);
                    fprintf(stderr, "+ completed '%s' [%d]\n", cmdln, WEXITSTATUS(status));
                }
                pid_t pid2 = fork();
                if (pid2 == 0)
                {
                    close(fd[1]);
                    dup2(fd[0], STDIN_FILENO);
                    close(fd[0]);
                    int evpReturn = execvp(cl[1]->arg[0], cl[1]->arg);
                    if (evpReturn < 0)
                    {
                        perror("execvp");
                        exit(1);
                    }
                    else
                    {
                        exit(0);
                    }
                }
                else
                {
                    int status;
                    waitpid(pid2, &status, 0);
                    fprintf(stderr, "+ completed '%s' [%d]\n", cmdln, WEXITSTATUS(status));
                }
            }
            else
            {
                pid_t pid = fork();
                if (pid == 0)
                {
                    if (cl[numCommands - 1]->fileName != NULL && strstr(cmdln, ">"))
                    {
                        int fd = open(cl[numCommands - 1]->fileName, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        dup2(fd, STDOUT_FILENO);
                        close(fd);
                    }

                    if (cl[numCommands - 1]->fileName != NULL && strstr(cmdln, "<"))
                    {
                        int fd = open(cl[numCommands - 1]->fileName, O_RDONLY, 0644);
                        dup2(fd, STDIN_FILENO);
                        close(fd);
                    }
                    if (strstr(cmdln, ">") && cl[numCommands - 1]->fileName == NULL)
                    {
                        error_checking(NO_OUTPUT_FILE);
                        break;
                    }
                    if (strstr(cmdln, "<") && cl[numCommands - 1]->fileName == NULL)
                    {
                        error_checking(NO_INPUT_FILE);
                        break;
                    }
                    int evpReturn = execvp(cl[0]->arg[0], cl[0]->arg);

                    /* FREE ARGUMENT VARIABLES */
                    for (int j = 0; j < NUM_PIPES; j++)
                    {
                        for (int i = 0; i < MAX_ARG; ++i)
                        {
                            free(cl[j]->arg[i]);
                        }
                        free(cl[j]->arg);
                        free(cl[j]->fileName);
                        free(cl[j]);
                    }

                    if (evpReturn < 0)
                    {
                        perror("execvp");
                        exit(1);
                    }
                    else
                    {
                        exit(0);
                    }
                }
                else if (pid > 0)
                {
                    int status;
                    waitpid(pid, &status, 0);
                    fprintf(stderr, "+ completed '%s' [%d]\n", cmdln, WEXITSTATUS(status));
                }
                else
                {
                    perror("fork");
                    exit(0);
                }
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
