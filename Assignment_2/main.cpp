#include "headers.h"
#include "prompt.h"
#include "clear.h"
#include "basicops.h"
#include "ls.h"
#include "systemCommands.h"
#include "pinfo.h"
#include "filesearch.h"
#include "io.h"
#include "history.h"
#include "pipeline.h"

using namespace std;


void runCommand(char * cmd){
    // pehle check karlo kahi pipeline toh nahi hai
    if(strstr(cmd, "|") != NULL){
        // vector<vector<string>> commands = parsePipeline(cmd);
        // cout << "debug" << endl;
        executePipeline(cmd);
        return;
    }

    char * args[50];
    int i = 0;
    bool bgProcess = false; // by default bg process is false
    char delimiter[5] = " \t\n"; // we have to ignore these - " ", tabs and new lines

    char * save = nullptr;
    char * token = strtok_r(cmd, delimiter, &save);

    while(token){
        if(strcmp(token, "&") == 0){
            bgProcess = true; // detecting 
        } else {
            args[i++] = token;
        }
        token = strtok_r(NULL, delimiter, &save);
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
    if(strcmp(args[0], "cat") == 0){
        runCat(args);
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

    if (strcmp(args[0], "pinfo") == 0) {
        runPinfo(args);
        return;
    }

    if(strcmp(args[0], "search") == 0){
        searchFileHelper(args);
        return;
    }

    if(strcmp(args[0], "history") == 0){
        printHistory(args);
        return;
    }

    if(strcmp(args[0], "sort") == 0){
        sortIO(args);
        return;
    }

    // fallback - any other command not fromt he  assignment
    if(bgProcess){
        background(args);
    } else {
        foreground(args);
    }
}

// utility to trim leading/trailing spaces
char* trim(char* str) {
    // remove spaces at the start
    while(isspace(*str)) str++;
    char *end = str + strlen(str) - 1;
    // remove spaces from the end
    while(end > str && isspace(*end)) *end-- = '\0';

    return str;
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
    
    // enableRawMode();// we're entering a black hole

    clearScreen();
    addToHistory();
    string input;
    while(1){
        printPrompt();
        cout << flush; // this is importatn - idk why .. but without this it didnt work properly
        // if(!getline(cin,input)) break;
        string input = readInput();

        if (input == "clear"){
            clearScreen();
        }
        if(input == "exit"){
            saveHistory();
            break;
        }

        appendToHistory(input.c_str());
        char * cmdLine = strdup(input.c_str());
        char * save = nullptr;
        char * cmd = strtok_r(cmdLine, ";", &save); // handle multiple commands separated by command lines

        while(cmd != NULL){
            char * trimmedCmd = trim(cmd);
            if(trimmedCmd)
                runCommand(trimmedCmd);
            cmd = strtok_r(NULL, ";", &save);
        }

        free(cmdLine);
        
    }
    // disableRawMode();
}

