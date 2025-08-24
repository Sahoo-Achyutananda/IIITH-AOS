#include "headers.h"
#include "prompt.h"
#include "clear.h"
#include "basicops.h"
#include "ls.h"
#include "systemCommands.h"

using namespace std;

void runCommand(char * cmd){
    char * args[50];
    int i = 0;
    bool bgProcess = false; // by default bg process is false
    char delimiter[5] = " \t\n"; // we have to ignore these - " ", tabs and new lines

    char * token = strtok(cmd, delimiter);

    while(token){
        if(strcmp(token, "&") == 0){
            bgProcess = true;
        } else {
            args[i++] = token;
        }
        token = strtok(NULL, delimiter);
    }
    args[i] = NULL;  // Without it,runEcho keep reading garbage memory.
    if(args[0] == NULL) return;


    if(strcmp(args[0], "pwd") == 0){
        runPwd();
        return;
    }
    if(strcmp(args[0], "echo") == 0){
        runEcho(args);
        return;
    }
    if(strcmp(args[0], "cd") == 0){
        runCd(args);
        return;
    }

    if(strcmp(args[0], "ls") == 0){
        runLS(args);
        return;
    }


    if(bgProcess){
        background(args);
    } else {
        foreground(args);
    }
}

int main(){
    // getting the l;ocation where the shell program resides - 
    char cwd[CHAR_LEN_MAX];
    char * c = getcwd(cwd,CHAR_LEN_MAX);
    if(!c){
        perror("getcwd");
        return 0;
    }
    shellHome = cwd;
    
    clearScreen();
    string input;
    while(1){
        printPrompt();
        if(!getline(cin,input)) break;

        if (input == "clear"){
            clearScreen();
        }
        if(input == "exit"){
            break;
        }

        char * cmdLine = strdup(input.c_str());
        char * cmd = strtok(cmdLine, ";"); // handle multiple commands separated by command lines

        while(cmd != NULL){
            runCommand(cmd);
            cmd = strtok(NULL, ";");
        }

        free(cmdLine);
        
    }
    
    

}