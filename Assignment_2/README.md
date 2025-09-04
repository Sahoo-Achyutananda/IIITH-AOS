# AOS Assignment - 2  : POSIX Shell

## Task :

Implement a shell that supports a semi-colon separated list of commands. Use 'strtok' to tokenize the command.Also, support '&' operator which lets a program run in the background after printing the process id of the newlycreated process. Write this code in a modular fashion.

## How to run ?

```bash
g++ main.cpp -o terminal
./terminal
```

## Requirements and their corresponding files that manage them - 

|S No | Requirement                                                                 | File               |
|-----|-----------------------------------------------------------------------------|--------------------|
|1.   | Display Requirement                                                         | `prompt.h`         |
|2.| `cd`, `echo`, `pwd`                                                         | `basicops.h`       |
|3. | `ls`                                                                        | `ls.h`             |
|4.| System Commands (background/foreground), with and without arguments         | `systemCommands.h` |
|5.| `pinfo`                                                                     | `pinfo.h`          |
|6.| Search                                                                      | `filesearch.h`     |
|7.| I/O Redirection                                                             | `io.h`             |
|8.| Pipeline                                                                    | `pipeline.h`       |
|9.| Redirection with Pipeline                                                   | `pipeline.h`       |
|10.| Simple Signals                                                              | `signals.h`        |
|11.| Autocomplete for commands and all files/directories under current directory | `autocomplete.h`   |
|12.| History                                                                     | `history.h`        |

Other supporting files - `colors.h` , `clear.h` , `headers.h`

## Brief Descriptions - 

##### 1. Display Requirement - 
- used `getlogin()` to fetch the userName.
- used `gethostname()` to fecth the hostName - i.e. the system name.
- used `getcwd()` to get the directory where the terminal resides - replaced it with ~

##### 2. cd, echo and pwd
- used `getcwd()` to handle `pwd`.
- handled basic `echo`. The implementation also calls I/O redirection versions of the echo command (present in the `io.h` file)
- handled `cd` with basic flags - `..` , `-` and `~` .

##### 3. ls
- used `opendir()` and `readdir()` to open and read contents of the directory. 
- iteratively displayed the file information by extracting the info using `stat()` 
- folders appear as `blue`.

##### 4. System Commands (background/foreground), with and without arguments
- used fork() to create a child process.
- used execvp() in child to execute the given command (with arguments).
- for foreground → parent waits using waitpid(), tracks foregroundPid for signals.
- for background → parent doesn’t wait, just prints PID & command.

##### 5. pinfo
- takes an optional PID argument (defaults to current shell PID if none given).
- opens /proc/[pid]/stat to read process state, process group, and foreground info.
- uses `tcgetpgrp()` to check if the process is in the foreground.
- opens /proc/[pid]/statm to fetch the memory usage.
- uses /proc/[pid]/exe with `readlink()` to resolve the executable path.
- finally prints PID, status, memory consumed, and executable path.

##### 6. Search

- takes a filename argument and starts search from the current directory (.).

- opens directories with `opendir()` and reads entries using `readdir()`.

- skips special entries "." and ".." to avoid infinite recursion.

- compares each entry’s d_name with the target filename.

- if the entry is a directory, recursively searches inside it.

- prints True if found, else False.

##### 7. I/O Redirection -

- implemented echo with redirection: > (overwrite) and >> (append) using dup2() and write().

- implemented cat to read files or via input redirection (<), printing contents to stdout.

- implemented sort that reads a file, stores lines in a vector, sorts them, and writes output to another file.

- used `open()`, `read()`, `write()`, and `dup2()` for low-level file handling and redirection.

##### 8 and 9. Pipeline and Pipeline with I/O Redirection - 
- defined a Cmd struct to hold command arguments, input file, output file, and append flag.
- `parsePipeline()` splits the input string by |, tokenizes each command, and detects <, >, and >>.
- `executePipeline()` sets up pipes between commands, applies input/output redirection, and forks processes.
- child processes run commands via `execvp() `
- parent process manages pipe ends and waits for all children to finish.
- `executeCommands()` also supports multiple commands separated by ;, executing each pipeline sequentially.

##### 10. Simple Signals

- declares an external foregroundPid to track the current foreground process.
- defines ctrlC(int sig) → sends `SIGINT` (2) to the foreground process, terminating it.
- defines ctrlZ(int sig) → sends `SIGTSTP` (20) to the foreground process, stopping it.
- prints a message when a process is stopped via Ctrl+Z.
- both functions only act if a valid foreground process exists.

##### 11. AutoComplete -

- `split()` breaks strings on delimiters; `findCommonPrefix()` finds the longest shared prefix among completions.

- `getCommandCompletions()` suggests built-in commands or executables found in $PATH.

- `getFileCompletions()` suggests files/directories from the current folder (adds / if directory).

- `completeWord()` updates the input with the matched completion; `redrawCurrentLine()` refreshes prompt + input.

- `showCompletions()` displays multiple options neatly without breaking prompt state.

- `handleAutocomplete()` decides between command/file completions, applies unique match, prefix expansion, or shows list.

##### 12. History
- maintains a global history vector with a max limit (default 20 commands).

- `addToHistory()` loads .history file into memory; `saveHistory()` writes back on exit.

- `appendToHistory()` keeps live updates by adding new commands and saving immediately.

- `printHistory()` shows last 10 commands by default or a user-specified limit.

- `readInput()` enables raw mode (no echo/canonical), handling backspace, Ctrl+D, tab (autocomplete), and arrow key navigation.

- integrates history navigation (↑/↓) with prompt redraw for smooth command recall.

## References Used - 
- getlogin : [Man Pages](https://man7.org/linux/man-pages/man3/getlogin.3.html)
- gethostname : [Man Pages](https://man7.org/linux/man-pages/man2/gethostname.2.html)


some string functions used : 
- strdup - duplicates a strong
- strchr - checks whether a char is present in a string
- strtok - for tokenization

References : [gfg](https://www.geeksforgeeks.org/cpp/strtok-strtok_r-functions-c-examples/)


to implement background and foreground process part - 

- wait :[Man Pages](https://man7.org/linux/man-pages/man3/wait.3p.html)
- exec : [Man Pages](https://man7.org/linux/man-pages/man3/exec.3.html)

WTH is a symlink - [fcc](https://www.freecodecamp.org/news/symlink-tutorial-in-linux-how-to-create-and-remove-a-symbolic-link/)
- readlink : [Man Pages](https://man7.org/linux/man-pages/man2/readlink.2.html)
- proc/pid/exe : [Man Pages](https://man7.org/linux/man-pages/man5/proc_pid_exe.5.html)
- proc : [Man Pages](https://man7.org/linux/man-pages/man5/proc.5.html) <proc pseudo file system>

[tcsetpgrp](https://man7.org/linux/man-pages/man3/tcsetpgrp.3.html)


input and output redirection - 
- readdir - 
[readdir](https://man7.org/linux/man-pages/man3/readdir.3.html)
The readdir() function returns a pointer to a dirent structurerepresenting the next directory entry in the directory stream pointed to by dirp.  It returns NULL on reaching the end of the directory stream or if an error occurred.

- youtube videos : 
  [codevault - dup and dup2](https://www.youtube.com/watch?v=5fnVr-zH-SE)
  [dup2 - Man Pages](https://man7.org/linux/man-pages/man2/dup.2.html)

pipes
  [codevault - pipe](https://www.youtube.com/watch?v=Mqb2dVRe0uo&t=440s)
  [codevault - fork and pipe](https://www.youtube.com/watch?v=6u_iPGVkfZ4)
  [pipe - man pages](https://man7.org/linux/man-pages/man2/pipe.2.html)

