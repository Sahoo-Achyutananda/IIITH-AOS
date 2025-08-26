#if !defined(PINFO_H)
#define PINFO_H

#include "headers.h"
using namespace std;

void runPinfo(char **args){
    long long pid;

    if(args[1] == NULL){
        pid = getpid();
    }else{
        pid = atoi(args[1]);
    }

    char processPath[PATH_MAX];

    snprintf(processPath,PATH_MAX, "/proc/%d/stat", pid);
    FILE *fp = fopen(processPath, "r");
    if(!fp){
        perror("fopen");
        return;
    }

    int ppid; char state;
    fscanf(fp, "%*d %*s %c %d", &state, &ppid); 
    fclose(fp);

    snprintf(processPath, PATH_MAX, "/proc/%d/statm", pid);
    fp = fopen(processPath, "r");
    if (fp == NULL) {
        perror("fopen");
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
    printf("Process Status -- %c\n", state);
    printf("memory -- %ld\n", mem);
    printf("Executable Path -- %s\n", exePath);

}



#endif // PINFO_H
