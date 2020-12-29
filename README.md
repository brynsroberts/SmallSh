# small shell

small shell will implement a subset of features of well-known shells, such as bash.

* Provide a prompt for running commands
* Handle blank lines and comments, which are lines beginning with the # character
* Provide expansion for the variable $$
* Execute 3 commands exit, cd, and status via code built into the shell
* Execute other commands by creating new processes using a function from the exec family of functions
* Support input and output redirection
* Support running commands in foreground and background processes
* Implement custom handlers for 2 signals, SIGINT and SIGTSTP

## Compile
    gcc --std=gnu99 -o smallsh main.c functions.c

## Run
    ./smallsh

## Using Test Script
To run the script, place it in the same directory as your compiled shell, chmod it (chmod +x ./p3testscript) and run this command from a bash prompt
    
    ./p3testscript > mytestresults 2>&1 

## Program Functionality
### 1. The Command Prompt

Use the colon : symbol as a prompt for each command line. 

The general syntax of a command line is:

    command [arg1 arg2 ...] [< input_file] [> output_file] [&]

…where items in square brackets are optional.

The command is made up of words separated by spaces.
The special symbols <, > and & are recognized, but they must be surrounded by spaces like other words.
If the command is to be executed in the background, the last word must be &. If the & character appears anywhere else, just treat it as normal text.
If standard input or output is to be redirected, the > or < words followed by a filename word must appear after all the arguments. Input redirection can appear before or after output redirection.
The shell does not support any quoting; so arguments with spaces inside them are not possible.
The shell supports command lines with a maximum length of 2048 characters, and a maximum of 512 arguments.

### 2. Comments & Blank Lines

Any line that begins with the # character is a comment line and should be ignored. Mid-line comments, such as the C-style //, are not supported.
A blank line (one without any commands) should also do nothing.
The shell will re-prompt for another command when it receives either a blank line or a comment line.

### 3. Expansion of Variable $$

The program will expand any instance of "$$" in a command into the process ID of the smallsh itself. The shell does not otherwise perform variable expansion. 

### 4. Built-in Commands

The shell will support three built-in commands: exit, cd, and status. These three built-in commands are the only ones that the shell will handle itself - all others are simply passed on to a member of the exec() family of functions.

These do not have to support input/output redirection for these built in commands
These commands do not have to set any exit status.
If the user tries to run one of these built-in commands in the background with the & option, it will ignore that option and run the command in the foreground anyway.

#### exit

The exit command exits your shell. It takes no arguments. 

#### cd

The cd command changes the working directory of smallsh.

By itself - with no arguments - it changes to the directory specified in the HOME environment variable
This is typically not the location where smallsh was executed from, unless the shell executable is located in the HOME directory, in which case these are the same.
This command can also take one argument: the path of a directory to change to. The cd command supports both absolute and relative paths.

#### status

The status command prints out either the exit status or the terminating signal of the last foreground process ran by the shell.

If this command is run before any foreground command is run, then it should simply return the exit status 0.
The three built-in shell commands do not count as foreground processes for the purposes of this built-in command - i.e., status should ignore built-in commands.

### 5. Executing Other Commands

The shell will execute any commands other than the 3 built-in command by using fork(), exec() and waitpid()

Whenever a non-built in command is received, the parent (i.e., smallsh) will fork off a child.
The child will use a function from the exec() family of functions to run the command.
The shell will use the PATH variable to look for non-built in commands, and it will allow shell scripts to be executed.
If a command fails because the shell could not find the command to run, then the shell will print an error message and set the exit status to 1.
A child process must terminate after running a command (whether the command is successful or it fails).

### 6. Input & Output Redirection

An input file redirected via stdin should be opened for reading only; if the shell cannot open the file for reading, it will print an error message and set the exit status to 1 (but don't exit the shell).
Similarly, an output file redirected via stdout will be opened for writing only; it will be truncated if it already exists or created if it does not exist. If the shell cannot open the output file it will print an error message and set the exit status to 1 (but don't exit the shell).
Both stdin and stdout for a command can be redirected at the same time.

### 7. Executing Commands in Foreground & Background

#### Foreground Commands

Any command without an & at the end must be run as a foreground command and the shell must wait for the completion of the command before prompting for the next command. For such commands, the parent shell does NOT return command line access and control to the user until the child terminates.

#### Background Commands

Any non built-in command with an & at the end must be run as a background command and the shell must not wait for such a command to complete. For such commands, the parent must return command line access and control to the user immediately after forking off the child.

The shell will print the process id of a background process when it begins.
When a background process terminates, a message showing the process id and exit status will be printed. This message will be printed just before the prompt for a new command is displayed.
If the user doesn't redirect the standard input for a background command, then standard input should be redirected to /dev/null
If the user doesn't redirect the standard output for a background command, then standard output should be redirected to /dev/null

### 8. Signals SIGINT & SIGTSTP

#### SIGINT

The shell, i.e., the parent process, will ignore SIGINT
Any children running as background processes will ignore SIGINT
A child running as a foreground process will terminate itself when it receives SIGINT

#### SIGTSTP

A child, if any, running as a foreground process will ignore SIGTSTP.
Any children running as background process will ignore SIGTSTP.
When the parent process running the shell receives SIGTSTP, the shell must display an informative message (see below) immediately if it's sitting at the prompt, or immediately after any currently running foreground process has terminated.
The shell then enters a state where subsequent commands can no longer be run in the background.
In this state, the & operator should simply be ignored, i.e., all such commands are run as if they were foreground processes.
If the user sends SIGTSTP again, then your shell will display another informative message immediately after any currently running foreground process terminates.
The shell then returns back to the normal condition where the & operator is once again honored for subsequent commands, allowing them to be executed in the background.

## Sources

* CS 344 - Operating Systems Portfolio Project

## Authors

* Bryan Roberts

