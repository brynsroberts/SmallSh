/*****************************************************************************************
 * Bryan Roberts
 * CS344 - Fall
 * Assignment 3: small shell
 * functions.c
 ****************************************************************************************/

#include "functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

/*****************************************************************************************
 * global variables
 ****************************************************************************************/
struct sigaction SIGINT_action = {0};
struct sigaction SIGTSTP_action = {0};
int noBackgroundExecution = 0;

/*****************************************************************************************
 * cd command no arguments 
 ****************************************************************************************/
void cdNoPath()
{

    // change directory to HOME environment directory
    chdir(getenv("HOME"));
}

/*****************************************************************************************
 * cd command with path argument 
 ****************************************************************************************/
void cdPath(char *path)
{

    // change directory to path specified by user
    chdir(path);
}

/*****************************************************************************************
 * replace $$ with smallsh pid in input string
 ****************************************************************************************/
void variableExpansion(char *str1, char *str2)
{

    int i = 0;
    char pid[7];
    sprintf(pid, "%d", getpid());

    while (i < strlen(str1))
    {
        if (i + 1 < strlen(str1) && str1[i] == '$' && str1[i + 1] == '$')
        {
            strcat(str2, pid);
            i += 2;
        }
        else
        {
            char c = str1[i];
            strncat(str2, &c, 1);
            i++;
        }
    }
}

/*****************************************************************************************
 * status command 
******************************************************************************************/
void statusCommand(int status, int foregroundProcessRun)
{

    // if not foreground process has been run - print default
    if (foregroundProcessRun == 0)
    {
        printf("exit value 0\n");
    }
    else
    {

        // if child was terminated normally - print status
        if (WIFEXITED(status))
        {
            printf("exit value %d\n", WEXITSTATUS(status));
        }

        // if child was terminated abnoramlly - print signal number
        else if (WIFSIGNALED(status))
        {
            printf("terminated by signal %d\n", WTERMSIG(status));
        }
    }

    // flush output buffer
    fflush(stdout);
}

/*****************************************************************************************
 * get command prompt
 * get a single line of input from the user up to 2048 characters
 ****************************************************************************************/
void getCommandPrompt(char *buffer, char *returnStr)
{

    // new prompt start with colon
    printf(": ");
    fflush(stdout);

    // get line of input from user, up to 2048 characters
    // referencing "c-for-dummies.com/blog?p=1112"
    // char buffer[2048];
    char *input = buffer;
    size_t bufferSize = 2048;
    getline(&input, &bufferSize, stdin);
    variableExpansion(buffer, returnStr);
}

/*****************************************************************************************
 * execute foregound command 
 ****************************************************************************************/
void executeCommandForeground(struct prompt *prompt, int *status)
{
    char *args[prompt->argCount + 2];

    // copy command into first element
    args[0] = calloc(strlen(prompt->command) + 1, sizeof(char));
    strcpy(args[0], prompt->command);

    // copy the rest of the arguments into the remaining spots
    for (int i = 1; i < prompt->argCount + 1; i++)
    {
        args[i] = calloc(strlen(prompt->argument[i - 1]) + 1, sizeof(char));
        strcpy(args[i], prompt->argument[i - 1]);
    }

    // final element is NULL
    args[prompt->argCount + 1] = NULL;

    // code modified from "Exploration: Process API - Executing a New Program"
    // run the program given
    pid_t spawnPid = fork();

    switch (spawnPid)
    {

    case (-1):
        // perror("fork()\n");
        *status = 1;
        perror("");
        exit(*status);
        break;

    // in child process
    case (0):

        // make SIGINT do default action
        SIGINT_action.sa_handler = SIG_DFL;
        SIGINT_action.sa_flags = 0;
        sigaction(SIGINT, &SIGINT_action, NULL);

        // setup input/output redirection in child process
        // source from example in "Exploration: Process and I/O"
        if (prompt->inputFile)
        {

            // open file for read only and check for error
            int inFile = open(prompt->inputFile, O_RDONLY);
            if (inFile == -1)
            {
                perror("");
                *status = 1;
                exit(1);
            }

            // redirect stdin to inFile and check for error
            int redirect = dup2(inFile, 0);
            if (redirect == -1)
            {
                perror("");
                *status = 2;
                exit(2);
            }
        }

        if (prompt->outputFile)
        {

            // open file for output redirection and check for error
            int outFile = open(prompt->outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (outFile == -1)
            {
                perror("");
                *status = -1;
                exit(1);
            }

            // redirect stdout to outFile and check for error
            int redirect = dup2(outFile, 1);
            if (redirect == -1)
            {
                perror("");
                *status = 2;
                exit(2);
            }
        }

        // execute command
        execvp(args[0], args);

        // if execvp errors - perform following
        perror("");
        fflush(stdout);
        *status = 2;
        exit(*status);
        break;

        // in parent process - wait for childs termination
    default:

        spawnPid = waitpid(spawnPid, status, 0);

        // if terminated by SIGINT
        if (WIFSIGNALED(*status))
        {
            statusCommand(*status, 1);
        }

        break;
    }

    for (int i = 0; i < prompt->argCount + 2; i++)
    {
        free(args[i]);
    }
}

/*****************************************************************************************
 * execute background command 
 ****************************************************************************************/
void executeCommandBackground(struct prompt *prompt)
{

    char *args[prompt->argCount + 2];

    // copy command into first element
    args[0] = calloc(strlen(prompt->command) + 1, sizeof(char));
    strcpy(args[0], prompt->command);

    // copy the rest of the arguments into the remaining spots
    for (int i = 1; i < prompt->argCount + 1; i++)
    {
        args[i] = calloc(strlen(prompt->argument[i - 1]) + 1, sizeof(char));
        strcpy(args[i], prompt->argument[i - 1]);
    }

    // final element is NULL
    args[prompt->argCount + 1] = NULL;

    // code modified from "Exploration: Process API - Executing a New Program"
    // run the program given

    int status;
    pid_t spawnPid = fork();

    switch (spawnPid)
    {

    case (-1):
        // perror("fork()\n");
        perror("");
        exit(1);
        break;

    // in child process
    case (0):

        // setup input/output redirection in child process
        // source from example in "Exploration: Process and I/O"
        if (prompt->inputFile)
        {

            // open file for read only and check for error
            int inFile = open(prompt->inputFile, O_RDONLY);
            if (inFile == -1)
            {
                perror("");
                exit(1);
            }

            // redirect stdin to inFile and check for error
            int redirect = dup2(inFile, 0);
            if (redirect == -1)
            {
                perror("");
                exit(2);
            }
        }

        // user did not redirect stdin - send to /dev/null
        else
        {

            // open file for read only and check for error
            int inFile = open("/dev/null", O_RDONLY);
            if (inFile == -1)
            {
                perror("");
                exit(1);
            }

            // redirect stdin to inFile and check for error
            int redirect = dup2(inFile, 0);
            if (redirect == -1)
            {
                perror("");
                exit(2);
            }
        }

        if (prompt->outputFile)
        {

            // open file for output redirection and check for error
            int outFile = open(prompt->outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (outFile == -1)
            {
                perror("");
                // *status = -1;
                exit(1);
            }

            // redirect stdout to outFile and check for error
            int redirect = dup2(outFile, 1);
            if (redirect == -1)
            {
                perror("");
                exit(2);
            }
        }

        // user did not redirect stdout - send to /dev/null
        else
        {
            // open file for output redirection and check for error
            int outFile = open("/dev/null", O_WRONLY, 00600);
            if (outFile == -1)
            {
                perror("");
                exit(1);
            }

            // redirect stdout to outFile and check for error
            int redirect = dup2(outFile, 1);
            if (redirect == -1)
            {
                perror("");
                exit(2);
            }
        }

        // execute command
        if (execvp(args[0], args))
        {

            // if execvp errors - perform following
            perror("");
            fflush(stdout);
            exit(2);
        }

        fflush(stdout);
        break;

    // in parent process - child process in background
    default:

        printf("background pid is %d\n", spawnPid);
        fflush(stdout);
        spawnPid = waitpid(spawnPid, &status, WNOHANG);

        break;
    }

    for (int i = 0; i < prompt->argCount + 2; i++)
    {
        free(args[i]);
    }
}

/*****************************************************************************************
 * parse out command prompt into prompt structure
 * sytax of a prompt will be - command [arg 1 arg2 ...] [< input_file] [> output_file] [&]
 ****************************************************************************************/
struct prompt *parsePrompt(char *promptLine)
{

    struct prompt *currPrompt = malloc(sizeof(struct prompt));

    // initialize pointers to NULL
    currPrompt->command = NULL;
    currPrompt->inputFile = NULL;
    currPrompt->outputFile = NULL;
    for (int i = 0; i < 512; i++)
    {
        currPrompt->argument[i] = NULL;
    }

    // set default values for backgroundExecution, comment, and argCount
    currPrompt->backgroundExecution = 0;
    currPrompt->comment = 0;
    currPrompt->argCount = 0;

    // check if prompt is a comment, if so make comment true and return prompt struct
    if (promptLine[0] == '#')
    {
        currPrompt->comment = 1;
        return currPrompt;
    }

    // for use with strtok_r
    char *savePtr;

    // get command for prompt
    char *token = strtok_r(promptLine, " \n", &savePtr);
    currPrompt->command = calloc(strlen(token) + 1, sizeof(char));
    strcpy(currPrompt->command, token);

    // if input_file or output_file found, make bool true to know what part of
    // input string we are parsing
    char *inputFileChar = "<";
    char *outputFileChar = ">";
    char *backgroundProcessChar = "&";

    // parse rest of prompt for arguments for space separated tokens
    while (token != NULL)
    {

        // get the next token
        token = strtok_r(NULL, " \n", &savePtr);

        if (token != NULL)
        {

            // if inputFile found
            if (strcmp(token, "<") == 0)
            {

                // get next token as inputFile
                token = strtok_r(NULL, " \n", &savePtr);
                currPrompt->inputFile = calloc(strlen(token) + 1, sizeof(char));
                strcpy(currPrompt->inputFile, token);

                // if outputFile found
            }
            else if (strcmp(token, ">") == 0)
            {

                // get next token as outputFile
                token = strtok_r(NULL, " \n", &savePtr);
                currPrompt->outputFile = calloc(strlen(token) + 1, sizeof(char));
                strcpy(currPrompt->outputFile, token);

                // if background process character found
            }
            else if (strcmp(token, "&") == 0)
            {
                currPrompt->backgroundExecution = 1;
                return currPrompt;

                // argument has been found
            }
            else
            {
                int count = currPrompt->argCount;
                currPrompt->argument[count] = calloc(strlen(token) + 1, sizeof(char));
                strcpy(currPrompt->argument[count], token);
                currPrompt->argCount++;
            }
        }
    }

    return currPrompt;
}

/*****************************************************************************************
 * checks background pids to see if they have finished executing 
 ****************************************************************************************/
void freeCurrentPrompt(struct prompt *currentPrompt)
{
    if (currentPrompt->command != NULL)
    {
        free(currentPrompt->command);
    }
    if (currentPrompt->inputFile != NULL)
    {
        free(currentPrompt->inputFile);
    }
    if (currentPrompt->outputFile != NULL)
    {
        free(currentPrompt->outputFile);
    }
    for (int i = 0; i < 512; i++)
    {
        if (currentPrompt->argument[i] != NULL)
        {
            free(currentPrompt->argument[i]);
        }
    }
}

/*****************************************************************************************
 * checks background pids to see if they have finished executing 
 ****************************************************************************************/
void checkBackgroundProcesses()
{

    // check for all child pid that have finished and print message to user
    int checkStatus;
    pid_t result;
    while ((result = waitpid(-1, &checkStatus, WNOHANG)) > 0)
    {
        printf("background pid %d is done: ", result);
        statusCommand(checkStatus, 1);
    }
}

/*****************************************************************************************
 * print values in prompt 
 ****************************************************************************************/
void printPrompt(struct prompt *prompt)
{
    printf("-------------------------------\n");
    printf("command: %s\n", prompt->command);
    for (int i = 0; i < prompt->argCount; i++)
    {
        printf("arg%d: %s\n", i + 1, prompt->argument[i]);
    }
    printf("input file: %s.\n", prompt->inputFile);
    printf("output file: %s.\n", prompt->outputFile);
    printf("background: %d\n", prompt->backgroundExecution);
    printf("-------------------------------\n");
}

/*****************************************************************************************
 * handle SIGTSTP function 
 ****************************************************************************************/
void handle_SIGTSTP(int signalNumber)
{

    // if noBackgroundExecution global set to 0 - set it 1 and print value
    if (noBackgroundExecution == 0)
    {
        char *printMessage = "\nEntering foreground-only mode (& is now ignored)\n";
        write(1, printMessage, 50);
        fflush(stdout);
        noBackgroundExecution = 1;
    }

    // if noBackgroundExecution global set to 1 - set to 0 and print value
    else
    {
        char *printMessage = "\nExiting foreground-only mode\n";
        write(1, printMessage, 30);
        fflush(stdout);
        noBackgroundExecution = 0;
    }

    char *prompt = ": ";
    write(1, prompt, 3);
    fflush(stdout);
}

/*****************************************************************************************
 * control flow for entire program
 * initial setup of signal handlers
 * do infinite loop getting arguments from users and performing correct function
 ****************************************************************************************/
void smallShell()
{
    // set default actions for sig handlers
    SIGINT_action.sa_handler = SIG_IGN;
    sigaction(SIGINT, &SIGINT_action, NULL);

    // set SIGTSTP to perform custom function upon being called
    SIGTSTP_action.sa_handler = handle_SIGTSTP;
    sigfillset(&SIGTSTP_action.sa_mask);
    SIGTSTP_action.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);

    // status variable
    int status = 0;
    int foregroundProcessRun = 0; // false value

    // continue until exit command given
    while (1)
    {

        // check for background processes
        checkBackgroundProcesses();

        // create buffers for prompt
        char promptBuffer[2048] = {'\0'};   // will hold initial user input
        char expandedPrompt[2048] = {'\0'}; // will hold variable expanded input

        // get command prompt and print it
        getCommandPrompt(promptBuffer, expandedPrompt);

        // if len of prompt < 1 - continue - don't do any of the following processing
        if (strlen(expandedPrompt) == 1)
        {
            continue;
        }

        // parse out prompt input into prompt struct
        struct prompt *currPrompt = parsePrompt(expandedPrompt);

        // if comment - skip to next prompt
        if (currPrompt->comment)
        {
        }

        // if exit - exit program
        else if (strcmp(currPrompt->command, "exit") == 0)
        {
            freeCurrentPrompt(currPrompt);
            free(currPrompt);
            break;
        }

        // if cd with no arguments - call cdNoPath()
        else if (strcmp(currPrompt->command, "cd") == 0 && currPrompt->argCount < 1)
        {
            cdNoPath();
        }

        // if cd with one argument - call cdPath(path)
        else if (strcmp(currPrompt->command, "cd") == 0 && currPrompt->argCount > 0)
        {
            cdPath(currPrompt->argument[0]);
        }

        // if status
        else if (strcmp(currPrompt->command, "status") == 0)
        {
            statusCommand(status, foregroundProcessRun);
        }

        // execute command
        else
        {

            // execute command in background
            if (currPrompt->backgroundExecution == 1 && noBackgroundExecution == 0)
            {
                executeCommandBackground(currPrompt);
            }

            // execute command in foreground
            else
            {
                foregroundProcessRun = 1;
                executeCommandForeground(currPrompt, &status);
            }
        }

        freeCurrentPrompt(currPrompt);
        free(currPrompt);
    }
}
