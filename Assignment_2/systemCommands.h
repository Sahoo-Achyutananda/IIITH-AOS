#if !defined(SYSTEMCOMMANDS_H)
#define SYSTEMCOMMANDS_H

#include "headers.h"
using namespace std;

void foreground(char **args){
    long long pid = fork();

    if(pid == 0){
        if(execvp(args[0],args) == -1){
            cerr << "execvp failed !" << endl;
            exit(1);
        }
    }else if(pid > 0){
        int status;
        waitpid(pid,&status,0);
        cout << pid << " " <<  args[0] << endl;
    }else{
        cerr << "Fork Failed" << endl;
    }
}

void background(char **args){
    long long pid = fork();

    if(pid == 0){
        if(execvp(args[0],args) == -1){
            cerr << "execvp failed !" << endl;
            exit(1);
        }
    }else if(pid > 0){
        cout << pid << " " <<  args[0] << endl;
    }else{
        cerr << "Fork Failed" << endl;
    }
}

#endif // SYSTEMCOMMANDS_H
