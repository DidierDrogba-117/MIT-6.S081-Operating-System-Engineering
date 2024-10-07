/*
echo hello too | xargs echo bye
first part output: hello too
then xargs reads the input from the pipe, and takes this input and appends it to the command specified after xargs
so the command becomes echo bye hello too

in shell, it firsts executes echo hello too, then output - hello too piped to xargs, then xargs executes echo bye hello too
*/

/*
copy part 2 args to a char array
then in a utimate while loop, exit until \n was read.

read from stdin

use a buffer(each element is a str) to store part 2 args and append part 1 stdin to it

fork and exec the command with the args
*/

#include "kernel/types.h"
#include "user/user.h"
#include "kernel/param.h"

#define MAX_LINE 128 // Maximum length of input line

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(2, "Usage: xargs command\n");
        exit(1);
    }

    char *finalargs[MAXARG];
    char buffer[MAX_LINE];
    int i; 
    int curr_line_index;

    // copy part 2 args to a char array
    // echo hello world | xargs echo bye -> finalargs = [echo, bye]
    for (i = 1; i < argc; i++)
    {
        finalargs[i - 1] = argv[i];
    }

    // ultimate while loop
    // read from stdin (pipe) from part 1 output
    while (1)
    {
        // Clear the buffer to ensure no leftover data remains using memset
        memset(buffer, '\0', sizeof(buffer));
        curr_line_index = 0;
        while (curr_line_index < MAX_LINE - 1)
        {
            // cannot read from stdin, just exit 
            if (read(0, &buffer[curr_line_index], 1) <= 0) {exit(0);}
            // if read \n, break
            // echo hi -> hi\n 
            if (buffer[curr_line_index] == '\n') {break;}
            curr_line_index++;
        }

        // change \n to \0 in the end of buffer 
        buffer[curr_line_index] = '\0';
        // finalargs append buffer 
        // echo hello world | xargs echo bye -> finalargs = 
        // [echo, bye, helloworld], argc = 3
        finalargs[argc - 1] = buffer;
        // finalargs passed to exec should be an array of ptrs, the last ptr should be 0
        // echo hello world | xargs echo bye -> finalargs = 
        // [echo, bye, helloworld, 0]
        finalargs[argc] = 0;
        // fork and exec the command with the args
        int pid = fork(); 
        // child 
        if (pid == 0) {
            exec(finalargs[0], finalargs);
            printf("xargs: exec %s failed\n", finalargs[1]);
            exit(1);
        }
        // parent 
        else {
            wait(0);
        }
    }
    exit(0);
}
