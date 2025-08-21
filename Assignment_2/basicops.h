#if !defined(BASICOPS_H)
#define BASICOPS_H
// Part 2 : implementing cd, echo and pwd 
#include "headers.h"
using namespace std;

string prevDir = "";

void runCd(char * args[]){
    int count = 0;
    while(args[count] != NULL)count++;

    if(count > 2){
        cerr << "cd : Too many arguments" << endl;
        return;
    }

    string targetDir;

    if(count == 1 || strcmp(args[1], "~") == 0){
        // const char *home = getenv("HOME");
        // if(home)
            targetDir = shellHome;
        // else
        //     targetDir = "/";
    }
    else if(strcmp(args[1], "-") == 0){
        if(prevDir.empty()){
            cerr << "cd  : OLDPWD not set" << endl;
            return;
        }
        targetDir = prevDir;
    }else{
        targetDir = args[1];
    }

    char currDir[PATH_MAX];
    char * c = getcwd(currDir, PATH_MAX);
    if(!c){
        perror("getcwd");
        return;
    }

    int t = chdir(targetDir.c_str());
    if(t!=0){
        perror("cd");
    }else{
        prevDir = currDir;
    }

}

void runPwd(){
    char cwd[CHAR_LEN_MAX];
    char * c = getcwd(cwd, CHAR_LEN_MAX);

    if(c != NULL){
        cout << cwd << endl;
    }else{
        cerr << "Error getting Current directory ! " << endl;
    }
}

void runEcho(char * arg[]){
    for(int i = 1; arg[i] != NULL; i++){
        cout << arg[i];
        if(arg[i+1])cout << " ";
    }
    cout << endl;
}

#endif // BASICOPS_H
