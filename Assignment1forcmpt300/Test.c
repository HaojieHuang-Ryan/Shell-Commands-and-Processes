//// Shell starter file
//// You may make any changes to any part of this file.
//
//#include <stdio.h>
//#include <stdbool.h>
//#include <stdlib.h>
//#include <string.h>
//#include <unistd.h>
//#include <errno.h>
//#include <sys/types.h>
//#include <sys/wait.h>
//
//#define COMMAND_LENGTH 1024
//#define NUM_TOKENS (COMMAND_LENGTH / 2 + 1)
//#define HISTORY_DEPTH 10
//char history[HISTORY_DEPTH][COMMAND_LENGTH];
//
///**
// * Command Input and Processing
// */
//
//void print_prompt()
//{
//    char tempPath[1024];
//    char *directory=getcwd(tempPath,1024);
//    if (directory==NULL)
//    {
//        write(STDOUT_FILENO, "Unable to get directory.\n", strlen("Unable to get directory.\n"));
//    }
//    write(STDOUT_FILENO, directory, strlen(directory));
//    write(STDOUT_FILENO, " $ ", strlen(" $ "));
//}
//
///*
// * Tokenize the string in 'buff' into 'tokens'.
// * buff: Character array containing string to tokenize.
// *       Will be modified: all whitespace replaced with '\0'
// * tokens: array of pointers of size at least COMMAND_LENGTH/2 + 1.
// *       Will be modified so tokens[i] points to the i'th token
// *       in the string buff. All returned tokens will be non-empty.
// *       NOTE: pointers in tokens[] will all point into buff!
// *       Ends with a null pointer.
// * returns: number of tokens.
// */
//int tokenize_command(char *buff, char *tokens[])
//{
//    int token_count = 0;
//    _Bool in_token = false;
//    int num_chars = strnlen(buff, COMMAND_LENGTH);
//    for (int i = 0; i < num_chars; i++) {
//        switch (buff[i]) {
//        // Handle token delimiters (ends):
//        case ' ':
//        case '\t':
//        case '\n':
//            buff[i] = '\0';
//            in_token = false;
//            break;
//
//        // Handle other characters (may be start)
//        default:
//            if (!in_token) {
//                tokens[token_count] = &buff[i];
//                token_count++;
//                in_token = true;
//            }
//        }
//    }
//    tokens[token_count] = NULL;
//    return token_count;
//}
//
///**
// * Read a command from the keyboard into the buffer 'buff' and tokenize it
// * such that 'tokens[i]' points into 'buff' to the i'th token in the command.
// * buff: Buffer allocated by the calling code. Must be at least
// *       COMMAND_LENGTH bytes long.
// * tokens[]: Array of character pointers which point into 'buff'. Must be at
// *       least NUM_TOKENS long. Will strip out up to one final '&' token.
// *       tokens will be NULL terminated (a NULL pointer indicates end of tokens).
// * in_background: pointer to a boolean variable. Set to true if user entered
// *       an & as their last token; otherwise set to false.
// */
//int num_history=0;
//int index_history=0;
//void read_command(char *buff, char *tokens[], _Bool *in_background)
//{
//    *in_background = false;
//
//    // Read input
//    int length = read(STDIN_FILENO, buff, COMMAND_LENGTH-1);
//
//    if (length < 0) {
//        perror("Unable to read command from keyboard. Terminating.\n");
//        exit(-1);
//    }
//
//    // Null terminate and strip \n.
//    buff[length] = '\0';
//    if (buff[strlen(buff) - 1] == '\n') {
//        buff[strlen(buff) - 1] = '\0';
//    }
//
//    // Add to history
//    if (num_history==HISTORY_DEPTH)
//    {
//        for (int i = 0; i < num_history-1; i++)
//        {
//            strcpy(history[i],history[i+1]);
//        }
//        num_history--;
//    }
//    strcpy(history[num_history],buff);
//    num_history++;
//    index_history++;
//
//    // Tokenize (saving original command string)
//    int token_count = tokenize_command(buff, tokens);
//    if (token_count == 0) {
//        return;
//    }
//
//    // Extract if running in background:
//    if (token_count > 0 && strcmp(tokens[token_count - 1], "&") == 0) {
//        *in_background = true;
//        tokens[token_count - 1] = 0;
//    }
//}
//
///**
// * Main and Execute Commands
// */
//int main(int argc, char* argv[])
//{
//    char input_buffer[COMMAND_LENGTH];
//    char *tokens[NUM_TOKENS];
//    while (true) {
//
//        // Get command
//        // Use write because we need to use read() to work with
//        // signals, and read() is incompatible with printf().
//        print_prompt();
//        _Bool in_background = false;
//        read_command(input_buffer, tokens, &in_background);
//
//        //Internal commands
//        if (strcmp(tokens[0], "exit")==0)
//        {
//            if (tokens[1]!=NULL)
//            {
//                write(STDOUT_FILENO, "exit error.", strlen("exit error."));
//            }
//            exit(0);
//        }
//        if (strcmp(tokens[0], "pwd")==0)
//        {
//            if (tokens[1]!=NULL)
//            {
//                write(STDOUT_FILENO, "pwd error.", strlen("pwd error."));
//            }
//            else
//            {
//                char tempPath[1024];
//                char *directory=getcwd(tempPath,1024);
//                if (directory==NULL)
//                {
//                    write(STDOUT_FILENO, "Unable to get directory.\n", strlen("Unable to get directory.\n"));
//                }
//                write(STDOUT_FILENO, "The current directory: ", strlen("The current directory: "));
//                write(STDOUT_FILENO, directory, strlen(directory));
//                write(STDOUT_FILENO, "\n", strlen("\n"));
//            }
//            continue;
//        }
//        if (strcmp(tokens[0], "cd")==0)
//        {
//            if (tokens[1]==NULL||tokens[2]!=NULL)
//            {
//                write(STDOUT_FILENO, "cd command error.\n", strlen("cd error.\n"));
//            }
//            else if (chdir(tokens[1])<0)
//            {
//                write(STDOUT_FILENO, "cd: no such file or directory:", strlen("cd: no such file or directory:"));
//                write(STDOUT_FILENO, tokens[1], strlen(tokens[1]));
//                write(STDOUT_FILENO, "\n", strlen("\n"));
//            }
//            continue;
//        }
//        if (strcmp(tokens[0], "help")==0)
//        {
//            if (tokens[1]==NULL)
//            {
//                write(STDOUT_FILENO, "cd:Changing the current working directory.\n", strlen("cd:Changing the current working directory.\n"));
//                write(STDOUT_FILENO, "exit:Exit the shell program.\n", strlen("exit:Exit the shell program.\n"));
//                write(STDOUT_FILENO, "pwd:Display the current working directory.\n", strlen("pwd:Display the current working directory.\n"));
//                write(STDOUT_FILENO, "history:Display the 10 most recent commands.\n", strlen("history:Display the 10 most recent commands.\n"));
//                write(STDOUT_FILENO, "help:Display help information on internal commands.\n", strlen("help:Display help information on internal commands.\n"));
//            }
//            else if (tokens[2]==NULL)
//            {
//                if (strcmp(tokens[1],"cd")==0)
//                {
//                    write(STDOUT_FILENO, "cd:is a builtin command for changing the current working directory.\n", strlen("cd:is a builtin command for changing the current working directory.\n"));
//                }
//                else if (strcmp(tokens[1],"exit")==0)
//                {
//                    write(STDOUT_FILENO, "exit:is builtin command for exiting the shell program.\n", strlen("exit:is builtin command for exiting the shell program.\n"));
//                }
//                else if (strcmp(tokens[1],"pwd")==0)
//                {
//                    write(STDOUT_FILENO, "pwd:is builtin command for display the current working directory.\n", strlen("pwd:is builtin command for display the current working directory.\n"));
//                }
//                else
//                {
//                    write(STDOUT_FILENO, tokens[1], strlen(tokens[1]));
//                    write(STDOUT_FILENO, " is an external command or application.\n", strlen("'%s' is an external command or application.\n"));
//                }
//            }
//            else
//            {
//                write(STDOUT_FILENO, "help error.\n", strlen("help error.\n"));
//            }
//            continue;
//        }
//        if (strcmp(tokens[0], "history")==0)
//        {
//            if (tokens[1]!=NULL)
//            {
//                write(STDOUT_FILENO, "history error.\n", strlen("history error.\n"));
//            }
//            else
//            {
//                for (int i = num_history; i > 0; i--)
//                {
//                    write(STDOUT_FILENO, history[i], strlen(history[i]));
//                    write(STDOUT_FILENO, "\n", strlen("\n"));
//                }
//            }
//            continue;
//        }
//
//
//
//
//
//
//        // DEBUG: Dump out arguments:
//        // for (int i = 0; tokens[i] != NULL; i++) {
//        //     write(STDOUT_FILENO, "   Token: ", strlen("   Token: "));
//        //     write(STDOUT_FILENO, tokens[i], strlen(tokens[i]));
//        //     write(STDOUT_FILENO, "\n", strlen("\n"));
//        // }
//        if (in_background) {
//            write(STDOUT_FILENO, "Run in background.", strlen("Run in background."));
//        }
//
//        /**
//         * Steps For Basic Shell:
//         * 1. Fork a child process
//         * 2. Child process invokes execvp() using results in token array.
//         * 3. If in_background is false, parent waits for
//         *    child to finish. Otherwise, parent loops back to
//         *    read_command() again immediately.
//         */
//        pid_t pid=fork();
//        if (pid<0)
//        {
//            write(STDOUT_FILENO, "Child fail.\n", strlen("Child fail.\n"));
//            exit(-1);
//        }
//        else if (pid==0)
//        {
//            if (execvp(tokens[0],tokens)==-1)
//            {
//                write(STDOUT_FILENO, "Error:Unkown command.\n", strlen("Error:Unkown command.\n"));
//                exit(-1);
//            }
//        }
//        else
//        {
//            if (!in_background)
//            {
//                waitpid(pid,NULL,0);
//            }
//            else
//            {
//                while (waitpid(-1,NULL,WNOHANG)>0)
//                {
//                    ;
//                }
//            }
//        }
//    }
//    return 0;
//}
