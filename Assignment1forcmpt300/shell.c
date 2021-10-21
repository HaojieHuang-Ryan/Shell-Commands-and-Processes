// Shell starter file
// You may make any changes to any part of this file.

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>

#define COMMAND_LENGTH 1024
#define NUM_TOKENS (COMMAND_LENGTH / 2 + 1)
#define BUFFER_SIZE 1024
static char buffer[BUFFER_SIZE];

#define HISTORY_DEPTH 10
char history[HISTORY_DEPTH][COMMAND_LENGTH];
int n_history = 0;
int num_history = 0;

void add_history(char* cmd);
void write_msg(int fd, const char* msg);

/**
 * Command Input and Processing
 */

/*
 * Tokenize the string in 'buff' into 'tokens'.
 * buff: Character array containing string to tokenize.
 *       Will be modified: all whitespace replaced with '\0'
 * tokens: array of pointers of size at least COMMAND_LENGTH/2 + 1.
 *       Will be modified so tokens[i] points to the i'th token
 *       in the string buff. All returned tokens will be non-empty.
 *       NOTE: pointers in tokens[] will all point into buff!
 *       Ends with a null pointer.
 * returns: number of tokens.
 */
int tokenize_command(char *buff, char *tokens[])
{
    int token_count = 0;
    _Bool in_token = false;
    int num_chars = strnlen(buff, COMMAND_LENGTH);
    for (int i = 0; i < num_chars; i++) {
        switch (buff[i]) {
                // Handle token delimiters (ends):
            case ' ':
            case '\t':
            case '\n':
                buff[i] = '\0';
                in_token = false;
                break;

                // Handle other characters (may be start)
            default:
                if (!in_token) {
                    tokens[token_count] = &buff[i];
                    token_count++;
                    in_token = true;
                }
        }
    }
    tokens[token_count] = NULL;
    return token_count;
}

bool expand_history(char* buff) {
    _Bool flag = false;
    if (strcmp(buff, "!!") == 0) {
        if (n_history == 0) {
            return false;
        }
        strcpy(buff, history[n_history-1]);
        flag = true;
    } else if (buff[0] == '!') {
        char* endptr = NULL;
        int n = strtol(buff+1, &endptr, 10);
        if (*endptr != '\0' || endptr == buff + 1) {
            return false;
        }
        if (!(n >= num_history - 10 && n < num_history)) {
            return false;
        }
        if (num_history >= 10) {
            strcpy(buff, history[10 - num_history + n -1]);
        } else {
            strcpy(buff, history[n - 1]);
        }
        flag = true;
    }

    if (flag) {
        write_msg(STDOUT_FILENO, buff);
        write_msg(STDOUT_FILENO, "\n");
    }
    return true;
}
char pre_dir[COMMAND_LENGTH];

void save_dir()
{
    char* result = getcwd(buffer, BUFFER_SIZE);
    if (result == NULL)
    {
        write_msg(STDOUT_FILENO, "Unable to get current directory\n");
        exit(-1);
    }
    strcpy(pre_dir, buffer);
}

/**
 * Read a command from the keyboard into the buffer 'buff' and tokenize it
 * such that 'tokens[i]' points into 'buff' to the i'th token in the command.
 * buff: Buffer allocated by the calling code. Must be at least
 *       COMMAND_LENGTH bytes long.
 * tokens[]: Array of character pointers which point into 'buff'. Must be at
 *       least NUM_TOKENS long. Will strip out up to one final '&' token.
 *       tokens will be NULL terminated (a NULL pointer indicates end of tokens).
 * in_background: pointer to a boolean variable. Set to true if user entered
 *       an & as their last token; otherwise set to false.
 */
void read_command(char *buff, char *tokens[], _Bool *in_background)
{
    *in_background = false;

    // Read input
    int length = read(STDIN_FILENO, buff, COMMAND_LENGTH-1);

    if (length < 0 && (errno != EINTR)) {
        perror("Unable to read command from keyboard. Terminating.\n");
        exit(-1);
    }

    // Null terminate and strip \n.
    buff[length] = '\0';
    if (buff[strlen(buff) - 1] == '\n') {
        buff[strlen(buff) - 1] = '\0';
    }

    if (!expand_history(buff)) {
        write_msg(STDOUT_FILENO, "Invalid History command.\n");
        tokens[0] = NULL;
        return;
    } else {
    }

    add_history(buff);

    // Tokenize (saving original command string)
    int token_count = tokenize_command(buff, tokens);
    if (token_count == 0) {
        return;
    }

    // Extract if running in background:
    if (token_count > 0 && strcmp(tokens[token_count - 1], "&") == 0) {
        *in_background = true;
        tokens[token_count - 1] = 0;
    }
}

void write_msg(int fd, const char* msg) {
    write(fd, msg, strlen(msg));
}

void handle_SIGINT(int sig) {
    if (sig == SIGINT) {
        char msg[BUFFER_SIZE];
        snprintf(msg, BUFFER_SIZE,
                 "\n"
                 "cd:   Changing the current working directory\n"
                 "exit: Exit the shell program\n"
                 "pwd:  Display the current working directory\n"
                 "history: Display the 10 most recent commands\n"
                 "help: Display help information on internal commands\n$ ");
        write_msg(STDOUT_FILENO, msg);
    }
}

void add_history(char* cmd) {
    int i = 0;
    if (n_history == HISTORY_DEPTH) {
        for (i = 1; i < n_history; ++i) {
            strcpy(history[i-1], history[i]);
        }
        n_history -= 1;
    }
    strcpy(history[n_history], cmd);
    n_history++;
    num_history++;
}

void display_history() {
    int i = 0;
    char msg[BUFFER_SIZE];
    for (i = 0; i < n_history; ++i) {
        snprintf(msg, BUFFER_SIZE,
                 "%d\t%s\n",
                 num_history - i,
                 history[n_history-i-1]);
        write_msg(STDOUT_FILENO, msg);
    }
}

/**
 * Main and Execute Commands
 */
int main(int argc, char* argv[])
{
    /* set up the signal handler */
    struct sigaction handler;
    memset(&handler, 0, sizeof(handler));
    handler.sa_handler = handle_SIGINT;
    handler.sa_flags = 0;
    //sigemptyset(&handler.sa_mask);
    sigaction(SIGINT, &handler, NULL);

    char input_buffer[COMMAND_LENGTH];
    char *tokens[NUM_TOKENS];
    save_dir();

    while (true) {
        // Get command
        // Use write because we need to use read() to work with
        // signals, and read() is incompatible with printf().
        char temp[COMMAND_LENGTH];
        if (getcwd(temp, sizeof(temp)) != NULL)
        {
            write(STDOUT_FILENO, temp, strlen(temp));
            write(STDOUT_FILENO, " $ ", strlen(" $ "));
        }
        _Bool in_background = false;
        read_command(input_buffer, tokens, &in_background);

        // DEBUG: Dump out arguments:
        // for (int i = 0; tokens[i] != NULL; i++) {
        //     write(STDOUT_FILENO, "   Token: ", strlen("   Token: "));
        //     write(STDOUT_FILENO, tokens[i], strlen(tokens[i]));
        //     write(STDOUT_FILENO, "\n", strlen("\n"));
        // }
        if (in_background) {
            write(STDOUT_FILENO, "Run in background.", strlen("Run in background."));
        }

        if (tokens[0] == NULL) {
            continue;
        }

        //Internal commands
        if (tokens[0] != NULL) {
            if (strcmp(tokens[0], "exit") == 0) {
                if (tokens[1] != NULL) {
                    write_msg(STDOUT_FILENO, "exit error: arguments number\n");
                    continue;
                }
                exit(0);
            } else if (strcmp(tokens[0], "cd") == 0) {
                char *dir=NULL;
                if (tokens[2] != NULL) {
                    write_msg(STDOUT_FILENO, "cd error: arguments number\n");
                    continue;
                }
                if (tokens[1] == NULL || strcmp(tokens[1], "~") == 0)
                {
                    struct passwd *pwd = getpwuid(getuid());
                    dir=pwd->pw_dir;
                    save_dir();
                    chdir(dir);
                    continue;
                }
                if (strcmp(tokens[1], "-") == 0)
                {
                    dir=getcwd(buffer,BUFFER_SIZE);
                    if (strcmp(dir,pre_dir)== 0)
                    {
                        write_msg(STDOUT_FILENO, "cd: OLDPWD not set.\n");
                    }
                    else
                    {
                        strcpy(dir,pre_dir);

                        write_msg(STDOUT_FILENO, dir);
                        write_msg(STDOUT_FILENO, "\n");
                        chdir(dir);
                    }
                    continue;
                }
                save_dir();
                if (chdir(tokens[1]) < 0) {
                    snprintf(buffer,
                             BUFFER_SIZE-1,
                             "Error %s %s: %s\n",
                             tokens[0], tokens[1],
                             strerror(errno));
                    write(STDOUT_FILENO, buffer, strlen(buffer));
                }
                continue;
            } else if (strcmp(tokens[0], "pwd") == 0) {
                if (tokens[1] != NULL) {
                    write_msg(STDOUT_FILENO, "pwd error: arguments number\n");
                } else {
                    getcwd(buffer, BUFFER_SIZE);
                    int n = strlen(buffer);
                    buffer[n] = '\n';
                    buffer[n+1] = '\0';
                    write_msg(STDOUT_FILENO, buffer);
                }
                continue;
            } else if (strcmp(tokens[0], "history") == 0) {
                if (tokens[1] != NULL) {
                    write_msg(STDOUT_FILENO, "history error: arguments number\n");
                } else {
                    display_history();
                }
                continue;
            }
            else if (strcmp(tokens[0], "help") == 0) {
                if (tokens[1] == NULL) {
                    snprintf(buffer, BUFFER_SIZE,
                             "cd:   Changing the current working directory\n"
                             "exit: Exit the shell program\n"
                             "pwd:  Display the current working directory\n"
                             "history: Display the 10 most recent commands\n"
                             "help: Display help information on internal commands\n");
                } else if (tokens[2] == NULL) {
                    if (strcmp(tokens[1], "exit") == 0) {
                        snprintf(buffer, BUFFER_SIZE, "'exit' is builtin command for exiting the shell program\n");
                    } else if (strcmp(tokens[1], "cd") == 0) {
                        snprintf(buffer, BUFFER_SIZE, "'cd' is builtin command for changing the current working directory\n");
                    } else if (strcmp(tokens[1], "pwd") == 0) {
                        snprintf(buffer, BUFFER_SIZE, "'pwd' is builtin command for displaying the current working directory\n");

                    } else {
                        snprintf(buffer, BUFFER_SIZE, "'%s' is an external command or application\n", tokens[1]);
                    }
                } else {
                    snprintf(buffer, BUFFER_SIZE, "help error: arguments number\n");
                }
                write_msg(STDOUT_FILENO, buffer);
                continue;
            }
        }

        /**
         * Steps For Basic Shell:
         * 1. Fork a child process
         * 2. Child process invokes execvp() using results in token array.
         * 3. If in_background is false, parent waits for
         *    child to finish. Otherwise, parent loops back to
         *    read_command() again immediately.
         */
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(1);
        }

        if (pid == 0) {
            int r = execvp(tokens[0], tokens);
            if (r < 0) {
                snprintf(buffer,
                         BUFFER_SIZE-1,
                         "Error %s: %s\n",
                         tokens[0],
                         strerror(errno));
                write(STDOUT_FILENO, buffer, strlen(buffer));
                exit(1);
            }
        } else {
            if (!in_background) {
                waitpid(pid, NULL, 0);
            }
            while (waitpid(-1, NULL, WNOHANG) > 0) {
                ;
            }
        }
    }
    return 0;
}
