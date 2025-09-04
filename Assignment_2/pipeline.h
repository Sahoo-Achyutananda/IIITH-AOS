#if !defined(PIPELINE_H)
#define PIPELINE_H

#include "headers.h"
#include "colors.h"
#include "history.h"
#include "basicops.h"
#include "io.h"
#include <fcntl.h>
#include "pinfo.h"
#include "ls.h"
#include "filesearch.h"

using namespace std;

struct Cmd{
    vector<string> argv;
    string inputFile;
    string outputFile;
    bool append = false;
};



vector<Cmd> parsePipeline(char * cmd) {
    vector<Cmd> cmds;
    
    // First, split the entire command by pipe symbols
    vector<string> commandStrings;
    char* saveptr = nullptr;
    char* token = strtok_r(cmd, "|", &saveptr);
    
    while (token) {
        // Trim whitespace from the token
        string trimmed = token;
        size_t start = trimmed.find_first_not_of(" \t\n");
        size_t end = trimmed.find_last_not_of(" \t\n");
        
        if (start != string::npos && end != string::npos) {
            trimmed = trimmed.substr(start, end - start + 1);
            commandStrings.push_back(trimmed);
        }
        token = strtok_r(nullptr, "|", &saveptr);
    }
    
    // Now parse each individual command
    for (auto& cmdStr : commandStrings) {
        Cmd c;
        char* cmd_copy = strdup(cmdStr.c_str()); // Make a copy we can tokenize
        char* saveptr2 = nullptr;
        char* arg = strtok_r(cmd_copy, " \t\n", &saveptr2);
        
        while (arg) {
            if (strcmp(arg, "<") == 0) {
                arg = strtok_r(nullptr, " \t\n", &saveptr2);
                if (arg) {
                    c.inputFile = arg;
                }
            } else if (strcmp(arg, ">") == 0) {
                arg = strtok_r(nullptr, " \t\n", &saveptr2);
                if (arg) {
                    c.outputFile = arg;
                    c.append = false;
                }
            } else if (strcmp(arg, ">>") == 0) {
                arg = strtok_r(nullptr, " \t\n", &saveptr2);
                if (arg) {
                    c.outputFile = arg;
                    c.append = true;
                }
            } else {
                c.argv.push_back(arg);
            }
            arg = strtok_r(nullptr, " \t\n", &saveptr2);
        }
        
        free(cmd_copy);
        
        if (!c.argv.empty()) {
            cmds.push_back(c);
        }
    }
    
    return cmds;
}

void executePipeline(char * cmd){
    vector<Cmd> cmds = parsePipeline(cmd);

    // debug - 

    for(int i = 0; i < cmds.size(); i++){
        for(int j = 0; j < cmds[i].argv.size(); j++){
            cout << cmds[i].argv[j] << " " ;
        }
        cout << endl;

        cout << cmds[i].inputFile << endl;
        cout << cmds[i].outputFile << endl;
        cout << cmds[i].append << endl;
    }

    /////////////////////////////

    int n = cmds.size();

    if(n == 0) return;

    int tempSTDIN = dup(STDIN_FILENO);
    int tempSTDOUT = dup(STDOUT_FILENO);
    int prevRead = -1;

    for(int i = 0; i < n; i++){
        int pipefd[2];
        if(i < n-1){  // Create pipe for all except last command
            if(pipe(pipefd) < 0){
                cerr << fontBold << colorRed << "Error in Pipe Creation" << reset << endl;
                return;
            }
        }

        int pid = fork();
        
        if(pid == 0){ // Child process
            // Handle input redirection (file takes precedence over pipe)
            if(!cmds[i].inputFile.empty()) {
                int input_fd = open(cmds[i].inputFile.c_str(), O_RDONLY);
                if(input_fd < 0) {
                    perror("open input file");
                    exit(EXIT_FAILURE);
                }
                dup2(input_fd, STDIN_FILENO);
                close(input_fd);
            } else if(i > 0) {
                // Read from previous pipe if no file input specified
                dup2(prevRead, STDIN_FILENO);
                close(prevRead);
            }

            // Handle output redirection (file takes precedence over pipe)
            if(!cmds[i].outputFile.empty()) {
                int flags = O_WRONLY | O_CREAT;
                flags |= cmds[i].append ? O_APPEND : O_TRUNC;
                
                int output_fd = open(cmds[i].outputFile.c_str(), flags, 0644);
                if(output_fd < 0) {
                    perror("open output file");
                    exit(EXIT_FAILURE);
                }
                dup2(output_fd, STDOUT_FILENO);
                close(output_fd);
            } else if(i < n - 1) {
                // Write to next pipe if no file output specified
                close(pipefd[0]);
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);
            }

            // Close all pipe ends in child
            if(prevRead != -1) {
                close(prevRead);
            }
            if(i < n - 1) {
                close(pipefd[0]);
                close(pipefd[1]);
            }

            // Execute the command
            if(cmds[i].argv.empty()) {
                exit(EXIT_SUCCESS);
            }

            // const char* cmd_name = cmds[i].argv[0].c_str();
            // vector<char*> args;
            // for (auto& arg : cmds[i].argv) {
            //     args.push_back(const_cast<char*>(arg.c_str()));
            // }
            // args.push_back(nullptr);

            const char* cmd_name = cmds[i].argv[0].c_str();
            vector<char*> args;

            // Special-case: echo, cat, sort should ignore <, >, >>
            if (strcmp(cmd_name, "echo") == 0 || strcmp(cmd_name, "cat") == 0 || strcmp(cmd_name, "sort") == 0) {
                for (size_t k = 0; k < cmds[i].argv.size(); k++) {
                    if (cmds[i].argv[k] == "<" || cmds[i].argv[k] == ">" || cmds[i].argv[k] == ">>") {
                        k++; // skip the filename too
                        continue;
                    }
                    args.push_back(const_cast<char*>(cmds[i].argv[k].c_str()));
                }
            } else {
                for (auto& arg : cmds[i].argv) {
                    args.push_back(const_cast<char*>(arg.c_str()));
                }
            }

            args.push_back(nullptr);


            // Built-in command handling - didnt work becUASE some already handle io wirhin them - its creating a conflict
            // if(strcmp(cmd_name, "pwd") == 0){
            //     runPwd();
            //     exit(EXIT_SUCCESS);
            // }
            // if(strcmp(cmd_name, "echo") == 0){
            //     runEcho(args.data());
            //     exit(EXIT_SUCCESS);
            // }
            // if(strcmp(cmd_name, "cat") == 0){
            //     runCat(args.data());
            //     exit(EXIT_SUCCESS);
            // }
            // if(strcmp(cmd_name, "cd") == 0){
            //     runCd(args.data());
            //     exit(EXIT_SUCCESS);
            // }
            // if(strcmp(cmd_name, "ls") == 0){
            //     runLS(args.data());
            //     exit(EXIT_SUCCESS);
            // }
            // if (strcmp(cmd_name, "pinfo") == 0) {
            //     runPinfo(args.data());
            //     exit(EXIT_SUCCESS);
            // }
            // if(strcmp(cmd_name, "search") == 0){
            //     searchFileHelper(args.data());
            //     exit(EXIT_SUCCESS); 
            // }
            // if(strcmp(cmd_name, "history") == 0){
            //     printHistory(args.data());
            //     exit(EXIT_SUCCESS);
            // }
            // if(strcmp(cmd_name, "sort") == 0){
            //     sortIO(args.data());
            //     exit(EXIT_SUCCESS);
            // }

            // External command
            execvp(args[0], args.data());
            perror("execvp");
            exit(EXIT_FAILURE);
            
        } else if (pid > 0) { // Parent process
            // Close previous read end if it exists
            if(prevRead != -1) {
                close(prevRead);
            }

            // Set up for next iteration
            if(i < n - 1) {
                close(pipefd[1]);  // Close write end in parent
                prevRead = pipefd[0];  // Save read end for next command
            }
        } else {
            cerr << fontBold << colorRed << "Fork failed" << reset << endl;
            return;
        }
    }

    // Wait for all children to complete
    for(int i = 0; i < n; i++){
        wait(NULL);
    }

    // Restore original stdin/stdout
    dup2(tempSTDIN, STDIN_FILENO);
    dup2(tempSTDOUT, STDOUT_FILENO);
    close(tempSTDIN);
    close(tempSTDOUT);
}

// Add this function to your code
void executeCommands(char* line) {
    char* saveptr_semicolon = nullptr;
    char* command = strtok_r(line, ";", &saveptr_semicolon);

    while (command != nullptr) {
        // Trim whitespace from the command
        string trimmed(command);
        size_t start = trimmed.find_first_not_of(" \t\n");
        size_t end = trimmed.find_last_not_of(" \t\n");
        if (start != string::npos) {
            trimmed = trimmed.substr(start, end - start + 1);
        } else {
            // Skip empty commands
            command = strtok_r(nullptr, ";", &saveptr_semicolon);
            continue;
        }

        // Call your existing pipeline function with the individual command
        char* pipeline_command = strdup(trimmed.c_str());
        executePipeline(pipeline_command);
        free(pipeline_command);
        
        command = strtok_r(nullptr, ";", &saveptr_semicolon);
    }
}

#endif // PIPELINE_H