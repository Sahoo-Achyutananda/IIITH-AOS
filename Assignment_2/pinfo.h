#if !defined(PINFO_H)
#define PINFO_H

#include "headers.h"
using namespace std;


void printProcessStatus(long long pid) {
    char path[PATH_MAX];
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        cerr << fontBold << colorRed << "open : Error opening file - " << path << reset << endl;
        return;
    }

    char buffer[1024];
    int n = read(fd, buffer, sizeof(buffer)-1);
    close(fd);
    if (n <= 0) {
        perror("read");
        cerr << fontBold << colorRed << "read : Error reading file - " << path << reset << endl;
        return;
    }

    buffer[n] = '\0';

    int pgrp, tpgid;
    char state;
    sscanf(buffer, "%*d %*s %c %*d %d %*d %*d %d", &state, &pgrp, &tpgid);

    long long fg_pgrp = tcgetpgrp(STDIN_FILENO);

    if (pgrp == fg_pgrp) {
        printf("Process status -- %c+\n", state);
    } else {
        printf("Process status -- %c\n", state);
    }
}

void runPinfo(char **args){
    int argc = countArgs(args);
    if(argc > 2){
        cerr << fontBold << colorRed << "Usage : pinfo OR pinfo <pid>" << reset << endl;
        return;
    }
    long long pid;

    if(args[1] == NULL){
        pid = getpid();
    }else{
        pid = atoi(args[1]);
    }
    if(pid == 0){
        cerr << fontBold << colorRed << "Enter a valid PID" << reset << endl;
        return;
    }
    char processPath[PATH_MAX];

    snprintf(processPath,PATH_MAX, "/proc/%d/stat", pid);
    FILE *fp = fopen(processPath, "r");
    if(!fp){
        cerr << fontBold << colorRed << "fopen : Error opening file - " << processPath << reset << endl;
        return;
    }

    int ppid;
    fscanf(fp, "%*d %*s %*c %d", &ppid); 
    fclose(fp);

    snprintf(processPath, PATH_MAX, "/proc/%d/statm", pid);
    fp = fopen(processPath, "r");
    if (fp == NULL) {
        cerr << fontBold << colorRed << "fopen : Error opening file - " << processPath << reset << endl;
        return;
    }
    long mem;
    fscanf(fp, "%ld", &mem);
    fclose(fp);

    snprintf(processPath, PATH_MAX, "/proc/%d/exe", pid);
    char exePath[PATH_MAX];
    ssize_t len = readlink(processPath, exePath, sizeof(exePath) - 1);
    if (len != -1) {
        exePath[len] = '\0'; 
    } else {
        strcpy(exePath, "Executable path not found");
    }

    printf("pid -- %d\n", pid);
    printProcessStatus(pid);
    // printf("Process Status -- %c\n", state);
    printf("memory -- %ld\n", mem);
    printf("Executable Path -- %s\n", exePath);

}



#endif // PINFO_H
