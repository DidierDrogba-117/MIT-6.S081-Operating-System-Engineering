#include "kernel/types.h"
#include "user/user.h"
/*
Your goal is to use pipe and fork to set up the pipeline. The first process feeds the numbers 2 through 35 into the pipeline. For each prime number, you will arrange to create one process that reads from its left neighbor over a pipe and writes to its right neighbor over another pipe. Since xv6 has limited number of file descriptors and processes, the first process can stop at 35.
*/

void sieve(int pipe_fd)
{
    // read from pipe, the first number is always prime
    int prime;
    if (read(pipe_fd, &prime, sizeof(prime)) == 0)
    {
        // didn't read anything, close pipe and return
        close(pipe_fd);
        return;
    }

    printf("prime %d\n", prime);

    // create new pipe, recursively call sieve
    int p[2];
    pipe(p);

    int pid = fork();
    // child process - read from pipe, call prime sieve recursively
    /*
    Each child process is responsible for:
        Receiving the first number (which is its prime).
        Spawning the next process in the pipeline (via recursion).
        Exiting, leaving the parent to do the work of filtering and forwarding.
    */
    if (pid == 0)
    {
        // child only need to read, no need to write
        close(p[1]);
        sieve(p[0]);
        exit(0);
    }
    // parent process - filter primes and write to pipe
    /*
    Each parent process is responsible for:
        Receiving numbers.
        Filtering multiples of its prime.
        Sending remaining numbers to the next process.
    */
    else
    {
        close(p[0]); // parent only need to write, no need to read
        int num;
        // filter primes
        while (read(pipe_fd, &num, sizeof(num)) > 0)
        {
            if (num % prime != 0)
            {
                write(p[1], &num, sizeof(num));
            }
        }

        // parent process finish read from old pipe_fd, and finish writing to pipe p, close them all
        close(pipe_fd);
        close(p[1]);
        // wait for child to finish
        wait(0);
    }
}

int main(int argc, char *argv[])
{
    int p1[2];
    pipe(p1);

    int pid = fork();
    // child process - read from pipe, call prime sieve
    if (pid == 0)
    {
        // child only need to read, no need to write
        // pipe[0] is the read end and pipe[1] is the write end.
        close(p1[1]);
        // call prime sieve to recursively deal with incoming primes
        sieve(p1[0]);
        exit(0);
    }
    // parent process - write to pipe generates 2 - 35 and write to pipe to pass to child
    else
    {
        // parent only need to write, no need to read
        close(p1[0]);
        // write to pipe
        for (int i = 2; i <= 35; i++)
        {
            write(p1[1], &i, sizeof(i));
        }
        // close write end of pipe
        close(p1[1]);
        // wait for child to finish
        wait(0);
        exit(0);
    }
}