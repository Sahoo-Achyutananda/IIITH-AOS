#if !defined(PROMPT_H)
#define PROMPT_H

#include "headers.h"
#include "colors.h"

using namespace std;

void printPrompt(){
    const char* userName = getlogin();
    char hostName[CHAR_LEN_MAX];
    char currentDirectory[CHAR_LEN_MAX];

    if(!userName){
        userName = "unknown123";
        return;
    }

    int host = gethostname(hostName,CHAR_LEN_MAX);
    if(host == -1){
        cerr << fontBold << colorRed << "Error extracting host-name" << reset << endl;
    }
    char* c = getcwd(currentDirectory, CHAR_LEN_MAX);
    if(!c){
        cerr << fontBold << colorRed << "Error extracting current directory ! " << reset << endl;
    }


    // const char* home = getenv("HOME");   // get the path to the /home directory
    string displayDir = currentDirectory;

    if (displayDir.find(shellHome) == 0) {
        displayDir.replace(0, strlen(shellHome.c_str()), "~");
    }

    cout << fontBold << bgGreen <<  userName << "@" << hostName << ":" << bgBlue << displayDir << reset << " > ";
}

#endif // PROMPT_H
