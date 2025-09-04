#if !defined(BASICOPS_H)
#define BASICOPS_H
// Part 2 : implementing cd, echo and pwd 
#include "headers.h"
#include "io.h"

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
    if(count == 1){
        const char *home = getenv("HOME");
        targetDir = home;
    }
    else if(strcmp(args[1], "~") == 0){
            targetDir = shellHome; // shell home is declared globally in headers.h
    }
    else if(strcmp(args[1], "-") == 0){
        if(prevDir.empty()){
            cerr << fontBold << colorRed <<  "cd : OLDPWD not set" << reset << endl;
            return;
        }
        cout << fontBold << colorBlue << prevDir << reset << endl;
        targetDir = prevDir;
    }else{
        targetDir = args[1];
    }

    char currDir[PATH_MAX];
    char * c = getcwd(currDir, PATH_MAX);
    if(!c){
        cerr << fontBold << colorRed <<  "getcwd : Error fetching current working directory" << reset << endl;
        return;
    }

    // handling directory changing at the end
    int t = chdir(targetDir.c_str());
    if(t!=0){
        cerr << fontBold << colorRed <<  "chdir : Error changing directory" << reset << endl;
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
        cerr << fontBold << colorRed << "Error getting Current directory ! " << reset << endl;
    }
}

void runEcho(char * arg[]){
    int symbolPos = -1; // to track > and  >> 
    for (int i = 1; arg[i] != NULL; i++) {
        if (strcmp(arg[i], ">") == 0 || strcmp(arg[i], ">>") == 0) {
            symbolPos = i;
            break;
        }
    }

    if(symbolPos == -1){
        for(int i = 1; arg[i] != NULL; i++){
            cout << arg[i];
            if(arg[i+1])cout << " ";
        }
        cout << endl;
        return;
    }else{
        char *op = arg[symbolPos];
        char *fileName = arg[symbolPos + 1];

        if (fileName == NULL) {
            cerr << colorRed << fontBold << "Error: Missing output file " << reset << endl;
            return;
        }

        if(strcmp(op, ">") == 0){
            echoIOComplete(fileName, arg, symbolPos);
        }else if(strcmp(op, ">>")== 0){
            echoIOAppend(fileName, arg, symbolPos);
        }
    }
}

#endif // BASICOPS_H
