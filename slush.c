#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define bufferSize 256
#define max_args 15

void sigintHandler(int sig_num)
{
    printf("\nslush> ");
} // signal handler for ctrl+c

// Functio`n to execute a program with arguments
void execute_command(char *command)
{

    if (strncmp(command, "cd ", 3) == 0)
    {
        // Extract the directory from the command
        char *directory = command + 3;
        // Change directory using chdir
        if (chdir(directory) != 0)
        {
            perror("chdir");
        }
        return;
    }

    char *args[max_args + 1];
    char *token;
    int i = 0;

    // Parse the command line into tokens
    token = strtok(command, " ");
    while (token != NULL && i < max_args)
    {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;
    //printf("Parsed command arguments:\n");
    for (int j = 0; j < i; j++)
    {

       // printf("Argument %d: %s\n", j, args[j]);
    }

    // Execute the command
    //      if (i > 0) {
    //              pid_t pid = fork();
    //              if (pid == 0) {
    if (execvp(args[0], args) == -1)
    {
        // If execvp fails, handle the error
        fprintf(stderr, "%s: Not Found\n", args[0]);
        perror("");
        exit(EXIT_FAILURE);
    }
    //                      exit(EXIT_SUCCESS); // This line will never be reached
    //                      if execvp succeeds
    //              } else if (pid > 0) {// exit(EXIT_SUCCESS); // This line will
    //              never be reached if execvp succeeds } else if (pid > 0) { }
    //              else {
    //                      perror("fork");
    //              }
    //      } else {
    // Handle invalid null command
    //              fprintf(stderr, "Invalid null command\n");
    //      }
    //      if( i == 0 ){
    //              int status;
    //              waitpid(pid, &status, 0);
    //
    //      }
}
int main()
{
    char *command;
    char buffer[bufferSize];
    char *commands[max_args];

    signal(SIGINT, sigintHandler);

    while (1)
    {
        printf("slush> ");

        if (fgets(buffer, bufferSize, stdin) == NULL)
        {
            printf("\n");
            break;
        }
        //printf("Received buffer: %s\n", buffer);

        int length = strlen(buffer);
        if (buffer[length - 1] == '\n')
        {
            buffer[length - 1] = '\0'; // Remove the newline character
        }

        command = strtok(buffer, "(");
        int num_commands = 0;
        while (command != NULL)
        {
            commands[num_commands++] = command;
            command = strtok(NULL, "(");
        }
      

        // Execute commands in sequence with pipes
        int pipefd[2];
        int oldpipe;
        pid_t pid;

        int input_fd = 0; // Input file descriptor for the next command
        for (int i = 0; i < num_commands; i++)
        {
            pipe(pipefd); // Create a pipe for each command
            //printf("Creating a pipe\n");

            if ((pid = fork()) == -1)
            {
                perror("fork");
                exit(EXIT_FAILURE);
            }
            else if (pid == 0)
            { // Child process
                if(i!=0) {
                    if(dup2(input_fd, STDIN_FILENO)==-1) {
                        perror("dup2");
                        exit(EXIT_FAILURE);
                    }
                    close(input_fd);
                }

                if(i!=num_commands - 1) {
                    if(dup2(pipefd[1], STDOUT_FILENO) == -1) {
                        perror("dup2");
                        exit(EXIT_FAILURE);
                    }
                }

                close(pipefd[0]);
                execute_command(commands[num_commands-i-1]);
                perror("execvp");
            }
            else
            {
                // Parent process
                //printf("Parent process waiting for child to finish\n");
                wait(NULL); // Wait for child process to finish
                if (i != 0)
                {
                    close(input_fd);      // Close write end of the pipe
                }
                input_fd = pipefd[0]; // Set input file descriptor for the next command
                close(pipefd[1]);
            }
        }
    }
    printf("shell exited\n");
    return 0;
}

