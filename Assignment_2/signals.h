#if !defined(SIGNAL_H)
#define SIGNAL_H

#include <signal.h>
#include <unistd.h>
#include <iostream>
using namespace std;

extern long long foregroundPid;

void ctrlC(int sig){
    if(foregroundPid > 0){
        kill(foregroundPid, 2);
    }
}

void ctrlZ(int sig){
    if(foregroundPid > 0){
        kill(foregroundPid, 20);
        cerr  << "\nProcess with pid - " << foregroundPid << " stopped" << endl;
    }
}

#endif // SIGNAL_H
