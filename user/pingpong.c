#include "kernel/types.h"
#include "user/user.h"

int main() {
    int p1[2], p2[2];
    // p[0] is read, p[1] is write
    pipe(p1); // parent -> child, parent write, child read - print "<pid>: received ping" 
    pipe(p2); // parent <- child, child write, parent read - print "<pid>: received pong"
    /*
    What Each Process Needs:
    Parent:
    Write to p1[1]: This is the write end of pipe 1 (parent → child).
    Read from p2[0]: This is the read end of pipe 2 (child → parent).
    
    Child:
    Read from p1[0]: This is the read end of pipe 1 (parent → child).
    Write to p2[1]: This is the write end of pipe 2 (child → parent).

    Exactly! When the parent closes p1[0] (the read end of pipe 1 in the parent process), it does not affect the child's copy of p1[0]. The child's p1[0] remains open and fully functional because each process (parent and child) has its own copy of the file descriptors after the fork() call.
    */
    
    char buf[1];

    int pid = fork();
    // parent
    if (pid > 0) {
        // in parent, close read end of p1
        close(p1[0]); 
        // close write end of p2 
        close(p2[1]);
        
        buf[0] = 'A';   
        // write to child
        write(p1[1], buf, 1);
        // read from child
        read(p2[0], buf, 1);

        printf("%d: received pong\n", getpid()); 

        // Close the remaining pipe ends
        close(p1[1]);  // Close write end of p1
        close(p2[0]);  // Close read end of p2

        wait(0); // need to wait for child to finish 

        exit(0);
    }
    // child 
    else {
        // in child, close p1 write and p2 read 
        close(p1[1]);
        close(p2[0]);

        read(p1[0], buf, 1);
        printf("%d: received ping\n", getpid()); 

        write(p2[1], buf, 1);
        // Close the remaining pipe ends
        close(p1[0]);  // Close read end of p1
        close(p2[1]);  // Close write end of p2

        exit(0);
    }

}