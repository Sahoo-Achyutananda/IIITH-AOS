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

    long long tempHolder = dup(STDOUT_FILENO);
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

    dup2(tempHolder, STDOUT_FILENO);
    close(tempHolder);

}

void echoIOAppend(char * fileName, char *arg[], int symbolPos){
    int fd = open(fileName, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if(fd == -1){
        cerr << colorRed << fontBold << "Error accessing file" << reset << endl;
        return;
    }

    int tempHolder = dup(STDOUT_FILENO);
    dup2(fd, STDOUT_FILENO);
    close(fd);

    for(int i = 1; i < symbolPos; i++){
        write(STDOUT_FILENO, arg[i], strlen(arg[i]));
        if(i+1 < symbolPos){
            write(STDOUT_FILENO, " ", 1);
        }
    }
    write(STDOUT_FILENO, "\n", 1);

    dup2(tempHolder, STDOUT_FILENO);
    close(tempHolder);
}

void runCat(char * arg[]){

    char *fileName;

    if(strcmp(arg[1], "<") != 0)
        fileName = arg[1];
    else{
        fileName = arg[2];
    }

    if(fileName == NULL){
        cerr << fontBold << colorRed << "Usage : cat <file_name> or cat < <file_name>" << reset << endl;
        return;
    }

    long long fd = open(fileName, O_RDONLY);
    if(fd < 0){
        cerr << fontBold << colorRed << "Error Opening File" << reset << endl;
        return;
    }

    long long tempHolder = dup(STDIN_FILENO);
    dup2(fd,STDIN_FILENO);
    close(fd);

    char * buffer = new char[1024]; 
    long long r = read(STDIN_FILENO, buffer,1024);
    
    while(r > 0){
        write(STDOUT_FILENO,buffer,r);
        r = read(STDIN_FILENO, buffer,1024);
    }
    cout << endl;

    delete [] buffer;

    dup2(tempHolder, STDIN_FILENO);
    close(tempHolder);
}


void sortIO(char * arg[]){
    char * sourceFile = arg[1];
    char * destinationFile = arg[2];

    if(sourceFile == NULL || destinationFile == NULL){
        cerr << fontBold << colorRed << "Usage : sort <source_filename> <destination_filename>" << reset << endl;
    }

    long long fds = open(sourceFile, O_RDONLY);
    long long fdd = open(destinationFile, O_WRONLY | O_APPEND | O_TRUNC);

    if(fds < 0){
        cerr << fontBold << colorRed << "Error opening source file" << reset << endl;
    }

    if(fdd < 0){
        cerr << fontBold << colorRed << "Error opening destination file" << reset << endl;
    }







    
}

#endif // IO_H
