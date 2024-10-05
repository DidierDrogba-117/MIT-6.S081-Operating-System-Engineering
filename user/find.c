/*
### Summary of the `find.c` Program

The `find.c` program in xv6 is a simplified version of the UNIX `find` command. Its purpose is to recursively search through a directory tree and find files that match a specified target filename. The program reads directory entries, checks whether each entry is a file or directory, and recursively processes subdirectories until the entire directory tree has been searched.

### Key Steps in the Program:

1. **Input Parsing (`main`)**:
   - The program expects two command-line arguments:
     1. The starting directory (e.g., `"."`).
     2. The target filename to search for (e.g., `"b"`).
   - If the user does not provide enough arguments, the program prints a usage message and exits.

2. **Recursive Search Function (`find`)**:
   - The `find` function is the core of the program and is responsible for recursively searching through directories and subdirectories.

3. **Opening a Directory**:
   - The program opens the starting directory using `open()`, and checks whether the directory was opened successfully.
   - It then uses `fstat()` to retrieve metadata (such as whether the path is a file or a directory).

4. **Reading Directory Entries**:
   - The program uses a `while` loop to read each entry in the directory using `read()`.
   - It processes each entry (`struct dirent`), which contains the name and inode number of the file or subdirectory.
   - The program **skips**:
     - Empty entries (entries with an inode number of 0).
     - Special directories `"."` (current directory) and `".."` (parent directory) to avoid infinite recursion.

5. **Constructing the Full Path**:
   - The program constructs the full path to each file or subdirectory by appending the entry name to the current directory path.
   - For example, if the current path is `"./a"` and the entry name is `"b"`, the full path will be `"./a/b"`.

6. **Checking File Type**:
   - After constructing the full path, the program calls `stat()` to retrieve the file's metadata (such as whether it’s a file or directory).
   - If the entry is a **directory** (`st.type == T_DIR`), the program **recursively calls `find()`** to search inside that subdirectory.
   - If the entry is a **file**, the program compares the file's name with the target filename using `strcmp()`. If it matches, the program prints the full path of the file.

7. **Recursive Behavior**:
   - The program continues to search subdirectories recursively, building the path at each level, and applying the same logic for files and directories.
   - It stops when there are no more entries to read.

8. **Program Termination**:
   - After finishing the search, the program closes all file descriptors and exits.

### Example Walkthrough:

For the command `find . b` in the following directory structure:

```
.
├── a
│   ├── b
│   └── c
└── b
```

- The program will:
  1. Start in the directory `"."`.
  2. Read and process entries:
     - `"a"` (directory): Recursively search inside `"./a"`.
     - `"b"` (file): Matches the target, so it prints `./b`.
  3. Inside `"./a"`:
     - `"b"` (file): Matches the target, so it prints `./a/b`.
     - `"c"` (file): Not a match, so it continues.
  4. Once all entries have been processed, the program exits.

### Essential Components to Implement:
- **Recursion**: The program uses recursion to handle subdirectories.
- **Directory Traversal**: The program reads directory entries one at a time and checks their types (file or directory).
- **String Comparison**: The program compares filenames using `strcmp()` to find matching files.
- **Path Construction**: The program constructs full paths dynamically by appending directory entry names to the current path.

---

### To Complete the Program:
1. **Handle File and Directory Status**:
   - Make sure to correctly differentiate between files and directories using `stat()`.
   
2. **Recursive Search**:
   - Implement the recursive search logic for subdirectories, ensuring that you skip `"."` and `".."`.
   
3. **File Matching**:
   - Compare each file's name with the target using `strcmp()`.

By following this structure, you can build a fully functioning version of `find.c` that efficiently searches for files in a directory tree and handles recursion.

*/

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"


char*
find_filename(char *path) {
    // start from 1 byte away from last char
    // /ab/c.txt
    //          ^ 
    char *p;        
    for (p = path + strlen(path); p >= path && *p != '/'; p--) 
        ; 
    // now p points to last slash, need +1 to point to the first char of the filename
    p++;
    return p;

}


void 
find(char *curr_path, char *target) {
    int fd;
    struct stat st;
    char buf[512];
    struct dirent de;

    // open current dir 
    if ((fd = open(curr_path, 0)) < 0) {
        fprintf(2, "find: cannot open %s\n", curr_path);
        return;
    }
    // check file status, open and fstat
    if (fstat(fd, &st) < 0) {
        fprintf(2, "find: cannot stat %s\n", curr_path);
        close(fd);
        return;
    } 

    // if it's a dir 
    // recursively call find in the sub dir
    if (st.type == T_DIR) {
        // check if path is too long 
        /*
        find . b.txt, path = ., target = b.txt 
        need to add 1 slash(/) to . -> ./
        then add the target b.txt -> ./b.txt
        then add 1 mroe byte for null terminator '\0'-> ./b.txt\0
        make it a C string, otherwise cannot use printf or strcmp         
        */
        if (strlen(curr_path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
            printf("find: path too long\n");
            close(fd);
            return;
        }
        // store curr_path to buf, add / 
        // curr_path = ./, buf = ./
        strcpy(buf, curr_path);
        // p points to the end of the string (1 byte after the last ch)
        // like . 
        //       ^ 
        char *new_path_ptr = buf + strlen(buf); 
        *new_path_ptr++ = '/';
        // now buf is like  ./
        //                    ^

        // read each entry in the dir
        while (read(fd, &de, sizeof(de)) == sizeof(de)) {
            // skip empty entries
            if (de.inum == 0) {
                continue;
            }
            // skip . and ..
            if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) 
            {
                continue;
            } 

            // copy the name of the entry to the end of the path
            // EX: buf = ./ -> ./b.txt
            memmove(new_path_ptr, de.name, DIRSIZ);
            // add null terminator 
            // ptr doens't move, this just access 1 byte away from last char then add \0
            new_path_ptr[strlen(de.name)] = '\0';

            // check the file status of the new path
            if (stat(buf, &st) < 0) {
                printf("find: cannot stat %s\n", buf);
                continue;
            }

            // if it's a dir, recursively call find
            if (st.type == T_DIR) {
                find(buf, target);
            }
            // it's a file, simply use strcom  
            else {
                if (strcmp(find_filename(buf), target) == 0) {
                    printf("%s\n", buf);
                }
            }

        }

    close(fd);
    }

    // if it's a file 
    else if (st.type == T_FILE) {
			fprintf(2, "Usage: find dir file\n");
			exit(1);        
    }
}

int
main(int argc, char *argv[]) {
    // must has at least 3 args, find . b.txt, argc == 3 
    // find b.txt, invalid, argc == 2 
    if (argc < 3) {
        fprintf(2, "Usage: find <path> <file>\n");
        exit(1);
    }
    // find . b.txt
//  idx  0  1   2
    // so starts from index 1 and 2 
    find(argv[1], argv[2]);

    exit(0);

}