#if !defined(IO_H)
#define IO_H

#include "headers.h"
#include "colors.h"
#include <algorithm>

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
    int argc = countArgs(arg);
    if(argc < 2 || argc > 3){
        cerr << fontBold << colorRed << "Usage : cat <file_name> or cat < <file_name>" << reset << endl;
        return;
    }
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
    int argc = countArgs(arg);
    if(argc < 4 || argc > 5){
        cerr << fontBold << colorRed << "Usage : sort <source_filename> > <destination_filename> OR sort < <source_filename> > <destination_filename>" << reset << endl;
        return;
    }
    char * sourceFile;
    char * destinationFile;
    if(strcmp(arg[1], "<") == 0 && strcmp(arg[3], ">") == 0){
        sourceFile = arg[2];
        destinationFile = arg[4];
    }else if(strcmp(arg[2], ">") == 0 ){
        sourceFile = arg[1];
        destinationFile = arg[3];
    }else{
        cerr << fontBold << colorRed << "Usage : sort <source_filename> > <destination_filename> OR sort < <source_filename> > <destination_filename>" << reset << endl;
        return;
    }

    // if(strcmp(arg[2], ">") != 0 ){
    //     cerr << fontBold << colorRed << "Usage : sort <source_filename> > <destination_filename>" << reset << endl;
    //     return;
    // }

    if(sourceFile == NULL || destinationFile == NULL){
        cerr << fontBold << colorRed << "Usage : sort <source_filename> > <destination_filename>" << reset << endl;
        return;
    }

    long long fds = open(sourceFile, O_RDONLY);
    long long fdd = open(destinationFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if(fds < 0){
        cerr << fontBold << colorRed << "Error opening source file" << reset << endl;
        close(fds);
        close(fdd);
        return;
    }

    if(fdd < 0){
        cerr << fontBold << colorRed << "Error accessing destination file" << reset << endl;
        close(fds);
        close(fdd);
        return;
    }

    vector<string> lines;
    char * buffer = new char[1024];

    long long r = read(fds, buffer, 1024);

    string temp = "";

    while(r > 0){
        int c = 0;
        for(int c = 0; c < r; c++){
            if(buffer[c] != '\n'){
                temp.push_back(buffer[c]);
            }else{
                lines.push_back(temp);
                temp.clear();
            }
        }    
        r = read(fds,buffer,1024);
    }

    sort(lines.begin(), lines.end());

    long long tempHolder = dup(STDOUT_FILENO);
    dup2(fdd,STDOUT_FILENO);
    close(fdd);

    for(auto &l : lines){
        cout << l << endl;
    }
    cout.flush();

    delete [] buffer;

    dup2(tempHolder, STDOUT_FILENO);
    close(tempHolder);


}


// pipeline is not pipelining 
// void runCatSimple() {
//     char buffer[1024];
//     long long bytes = read(STDIN_FILENO, buffer, 1024);
//     while(bytes > 0) {
//         write(STDOUT_FILENO, buffer, bytes);
//         bytes = read(STDIN_FILENO, buffer, 1024);
//     }
// }

// void runEchoSimple(char *arg[]) {
//     for(int i = 1; arg[i] != nullptr; i++) {
//         write(STDOUT_FILENO, arg[i], strlen(arg[i]));
//         if(arg[i+1] != nullptr) {
//             write(STDOUT_FILENO, " ", 1);
//         }
//     }
//     write(STDOUT_FILENO, "\n", 1);
// }

// void runCatSimple(char * arg[]) {
//     int i = 1;
//     while(arg[i] != NULL) {
//         // Only process non-redirection arguments
//         if(strcmp(arg[i], "<") == 0 || strcmp(arg[i], ">") == 0 || strcmp(arg[i], ">>") == 0) {
//             i += 2; // skip symbol and filename
//             continue;
//         }
        
//         int fd = open(arg[i], O_RDONLY);
//         if(fd < 0) {
//             cerr << fontBold << colorRed << "Erroe Opening FIle" << reset << endl;
//             i++;
//             continue;
//         }
        
//         char buffer[1024];
//         ssize_t bytes;
//         while((bytes = read(fd, buffer, sizeof(buffer))) > 0) {
//             write(STDOUT_FILENO, buffer, bytes);
//         }
        
//         close(fd);
//         i++;
//     }
// }
#endif // IO_H
