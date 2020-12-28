#ifndef FUNCTIONS_H
#define FUNCTIONS_H

/*****************************************************************************************
 * Bryan Roberts
 * CS344 - Fall
 * Assignment 3: small shell
 * functions.h
 ****************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

/*****************************************************************************************
 * global variables defined in functions.c
 ****************************************************************************************/
extern struct sigaction SIGINT_action;
extern struct sigaction SIGTSTP_action;
extern int noBackgroundExecution;

/*****************************************************************************************
 * struct prompt
 * used to hold parsed out values from single line of user input
 ****************************************************************************************/
struct prompt
{
    char *command;
    char *argument[512];
    char *inputFile;
    char *outputFile;
    int backgroundExecution; // 0 - not background process, 1 - background process
    int comment;             // 0 - not a comment, 1 - comment
    int argCount;
};

/*****************************************************************************************
 * function headers
 ****************************************************************************************/
void cdNoPath();
void cdPath(char *path);
void variableExpansion(char *str1, char *str2);
void statusCommand(int status, int foregroundProcessRun);
void getCommandPrompt(char *buffer, char *returnStr);
void executeCommandForeground(struct prompt *prompt, int *status);
void executeCommandBackground(struct prompt *prompt);
struct prompt *parsePrompt(char *promptLine);
void freeCurrentPrompt(struct prompt *currentPrompt);
void checkBackgroundProcesses();
void printPrompt(struct prompt *prompt);
void handle_SIGTSTP(int signalNumber);
void smallShell();

#endif /* FUNCTIONS_H */
