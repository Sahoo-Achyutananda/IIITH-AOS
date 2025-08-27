#if !defined(IO_H)
#define IO_H

#include "headers.h"
#include "colors.h"

using namespace std;

void echoIOComplete(char * fileName, char *arg[], int symbolPos){
    long long fd = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if(fd == -1){
        cerr << colorRed << fontBold << "Error accessing file" << reset << endl;
        return;
    }

    long long fdOut = dup(STDOUT_FILENO);
    dup2(fd,STDOUT_FILENO);
    close(fd);

    int i = 1;
    while(arg[i] != NULL){
        if(strcmp(arg[i], ">") == 0){
            break;
        }
        write(STDOUT_FILENO, arg[i], strlen(arg[i]));
        if (i+1 < symbolPos) {
            write(STDOUT_FILENO, " ", 1);
        }
        i++;
    }
    write(STDOUT_FILENO, "\n", 1);

    dup2(fdOut, STDOUT_FILENO);
    close(fdOut);

}

void echoIOAppend(char * fileName, char *arg[], int symbolPos){
    int fd = open(fileName, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if(fd == -1){
        cerr << colorRed << fontBold << "Error accessing file" << reset << endl;
        return;
    }

    int fdOut = dup(STDOUT_FILENO);
    dup2(fd, STDOUT_FILENO);
    close(fd);

    for(int i = 1; i < symbolPos; i++){
        write(STDOUT_FILENO, arg[i], strlen(arg[i]));
        if(i+1 < symbolPos){
            write(STDOUT_FILENO, " ", 1);
        }
    }
    write(STDOUT_FILENO, "\n", 1);

    dup2(fdOut, STDOUT_FILENO);
    close(fdOut);
}

#endif // IO_H
