#if !defined(PIPELINE_H)
#define PIPELINE_H

#include "headers.h"

using namespace std;

vector<vector<string>> parsePipeline(char * cmd) {
    vector<vector<string>> cmds;
    // vector<string> current;
    
    char* save = nullptr;
    // First split by |, then by spaces - urrhhrhrhrhrhrhhhhhhh
    char* pipe_token = strtok_r(cmd, "|", &save);
    
    while (pipe_token) {
        vector<string> args;
        char* arg_save = nullptr;
        char* arg = strtok_r(pipe_token, " \t\n", &arg_save);
        
        while (arg) {
            args.push_back(arg);
            arg = strtok_r(NULL, " \t\n", &arg_save);
        }
        
        if (!args.empty()) {
            cmds.push_back(args);
        }
        pipe_token = strtok_r(NULL, "|", &save);
    }
    
    return cmds;
}

// vector<vector<string>> parsePipeline(char * cmd){
//     vector<vector<string>> cmds;
//     vector<string> ccmd;

//     char* temp = nullptr;
//     char* token = strtok_r(cmd," \t\n", &temp);

//     while(token){
//         if(strcmp(token ,"|") == 0){
//             if(ccmd.empty() == false){
//                 cmds.push_back(ccmd);
//                 ccmd.clear();
//             }
//         }else{
//             ccmd.push_back(token);
//         }
//         token = strtok_r(NULL, " \t\n", &temp);
//     }

//     //handling the last command if any - 
//     if(!ccmd.empty()){
//         cmds.push_back(ccmd);
//     }

//     return cmds;
// }

// WE HAVE TO TRACK THE PREVIOUS READEND ->> AAhmmmmm...
// for each cmd we 'll have a read end and a write end. -> except for the last command !-> it has to read from the previous write 
void executePipeline(char * cmd){
    vector<vector<string>> cmds = parsePipeline(cmd);
    int n = cmds.size();

    long long tempSTDIN = dup(STDIN_FILENO);
    long long tempSTDOUT = dup(STDOUT_FILENO);
    int prevRead = -1; // holds the file descriptor of the priviously read 

    for(int i = 0; i < n; i++){
        int pipefd[2];
        if(i <= n-2){
            if(pipe(pipefd) < 0){
                cerr << fontBold << colorRed << "Error in Pipe CReation" << reset << endl;
                return;
            }
        }

        int pid = fork();
        // the children - 
        if(pid == 0){
            // imput redirection : from the 2nd command onwards
            if(i >= 1){
                dup2(prevRead, STDIN_FILENO);
                close(prevRead);
            }
            // output redirection : till the last but one command
            if(i <= n - 2){
                close(pipefd[0]);
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);
            }else{
                dup2(tempSTDOUT, STDOUT_FILENO);
            }

            // currently executiing using execvp - willl be changed later --- kar diya change

            // return caused deadlocks - so added exit(EXIT_FAILURE)
            int len = cmds[i].size();
            vector<char*> args;
            for (int j = 0; j < cmds[i].size(); j++) {
                args.push_back(const_cast<char*>(cmds[i][j].c_str()));
            }
            args.push_back(nullptr); // NULL terminate

            if(strcmp(args[0], "pwd") == 0){
                runPwd();
                // return;
                exit(EXIT_SUCCESS);
            }
            if(strcmp(args[0], "echo") == 0){
                runEcho(args.data());
                // return;
                exit(EXIT_SUCCESS);
            }
            if(strcmp(args[0], "cat") == 0){
                runCat(args.data());
                // return;
                exit(EXIT_SUCCESS);
            }
            if(strcmp(args[0], "cd") == 0){
                runCd(args.data());
                // return;
                exit(EXIT_SUCCESS);
            }

            if(strcmp(args[0], "ls") == 0){
                runLS(args.data());
                // return;
                exit(EXIT_SUCCESS);
            }

            if (strcmp(args[0], "pinfo") == 0) {
                runPinfo(args.data());
                // return;
                exit(EXIT_SUCCESS);
            }

            if(strcmp(args[0], "search") == 0){
                searchFileHelper(args.data());
                // return;
                exit(EXIT_SUCCESS); 
            }

            if(strcmp(args[0], "history") == 0){
                printHistory(args.data());
                // return;
                exit(EXIT_SUCCESS);
            }

            if(strcmp(args[0], "sort") == 0){
                sortIO(args.data());
                // return;
                exit(EXIT_SUCCESS);
            }

            execvp(args[0], args.data());
            perror("execvp");
            exit(EXIT_FAILURE);
        }else if (pid > 0){
            if(prevRead != -1){
                close(prevRead);
            }

            if(i < n - 1){
                close(pipefd[1]);
                prevRead = pipefd[0];
            }
        }else{
            // pid < 0 => error hua, baar baar hua
            cerr << fontBold << colorRed << "Something went wrong in process creation" << reset << endl;
            return;
        }
    }

    for(int i = 0 ; i < n ; i++){
        wait(NULL);
    }

    dup2(tempSTDIN, STDIN_FILENO);
    dup2(tempSTDOUT, STDOUT_FILENO);
    close(tempSTDIN);
    close(tempSTDOUT);

}

#endif // PIPELINE_H
