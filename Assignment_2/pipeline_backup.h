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
#include <wordexp.h>

using namespace std;

struct Cmd{
    vector<string> argv;
    string inputFile;
    string outputFile;
    bool append = false;
};

// Helper function to split command while respecting quotes
vector<string> splitCommand(const string& cmd) {
    vector<string> tokens;
    string current;
    bool in_quote = false;
    char quote_char = '\0';
    
    for (char c : cmd) {
        if (c == '"' || c == '\'') {
            if (!in_quote) {
                in_quote = true;
                quote_char = c;
                if (!current.empty()) {
                    tokens.push_back(current);
                    current.clear();
                }
            } else if (c == quote_char) {
                in_quote = false;
                if (!current.empty()) {
                    tokens.push_back(current);
                    current.clear();
                }
            } else {
                current += c;
            }
        } else if (isspace(c) && !in_quote) {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    
    if (!current.empty()) {
        tokens.push_back(current);
    }
    
    return tokens;
}

vector<Cmd> parsePipeline(char * cmd) {
    vector<Cmd> cmds;
    
    // First, split the entire command by pipe symbols, respecting quotes
    vector<string> commandStrings;
    string current;
    bool in_quote = false;
    char quote_char = '\0';
    
    for (int i = 0; cmd[i] != '\0'; i++) {
        char c = cmd[i];
        
        if (c == '"' || c == '\'') {
            if (!in_quote) {
                in_quote = true;
                quote_char = c;
            } else if (c == quote_char) {
                in_quote = false;
            }
            current += c;
        } else if (c == '|' && !in_quote) {
            if (!current.empty()) {
                commandStrings.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    
    if (!current.empty()) {
        commandStrings.push_back(current);
    }
    
    // Now parse each individual command
    for (auto& cmdStr : commandStrings) {
        Cmd c;
        vector<string> tokens = splitCommand(cmdStr);
        
        for (size_t i = 0; i < tokens.size(); i++) {
            if (tokens[i] == "<") {
                if (i + 1 < tokens.size()) {
                    c.inputFile = tokens[++i];
                }
            } else if (tokens[i] == ">") {
                if (i + 1 < tokens.size()) {
                    c.outputFile = tokens[++i];
                    c.append = false;
                }
            } else if (tokens[i] == ">>") {
                if (i + 1 < tokens.size()) {
                    c.outputFile = tokens[++i];
                    c.append = true;
                }
            } else {
                c.argv.push_back(tokens[i]);
            }
        }
        
        if (!c.argv.empty()) {
            cmds.push_back(c);
        }
    }
    
    return cmds;
}

void executePipeline(char * cmd){
    vector<Cmd> cmds = parsePipeline(cmd);
    int n = cmds.size();
    
    if(n == 0) return;
    
    int tempSTDIN = dup(STDIN_FILENO);
    int tempSTDOUT = dup(STDOUT_FILENO);
    int prevRead = -1;
    vector<pid_t> child_pids;
    
    for(int i = 0; i < n; i++){
        int pipefd[2];
        if(i < n-1){
            if(pipe(pipefd) < 0){
                cerr << fontBold << colorRed << "Error in Pipe Creation" << reset << endl;
                return;
            }
        }
        
        pid_t pid = fork();
        
        if(pid == 0){
            // Close previous read end if it exists
            if(prevRead != -1) {
                close(prevRead);
            }
            
            // INPUT REDIRECTION HANDLING - FIXED
            if(!cmds[i].inputFile.empty()) {
                // File input takes precedence over pipe
                int input_fd = open(cmds[i].inputFile.c_str(), O_RDONLY);
                if(input_fd < 0) {
                    perror("open input file");
                    exit(EXIT_FAILURE);
                }
                dup2(input_fd, STDIN_FILENO);
                close(input_fd);
                
                // If there was a pipe from previous command, close it
                if(i > 0 && prevRead != -1) {
                    close(prevRead);
                }
            } else if(i > 0) {
                // No file input, so read from previous pipe
                dup2(prevRead, STDIN_FILENO);
                close(prevRead);
            }
            
            // OUTPUT REDIRECTION HANDLING
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
                
                // If there's a pipe to next command, close it
                if(i < n - 1) {
                    close(pipefd[0]);
                    close(pipefd[1]);
                }
            } else if(i < n - 1) {
                // No file output, so write to next pipe
                close(pipefd[0]);
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);
            }
            
            // Close any remaining pipe file descriptors
            if(i < n - 1) {
                close(pipefd[0]);
                close(pipefd[1]);
            }
            
            // Execute the command
            if(cmds[i].argv.empty()) {
                exit(EXIT_SUCCESS);
            }
            
            const char* cmd_name = cmds[i].argv[0].c_str();
            vector<char*> args;
            for (auto& arg : cmds[i].argv) {
                args.push_back(const_cast<char*>(arg.c_str()));
            }
            args.push_back(nullptr);
            
            // Built-in commands
            if(strcmp(cmd_name, "pwd") == 0){
                runPwd();
                exit(EXIT_SUCCESS);
            }
            if(strcmp(cmd_name, "echo") == 0){
                runEcho(args.data());
                exit(EXIT_SUCCESS);
            }
            if(strcmp(cmd_name, "cat") == 0){
                runCat(args.data());
                exit(EXIT_SUCCESS);
            }
            if(strcmp(cmd_name, "cd") == 0){
                runCd(args.data());
                exit(EXIT_SUCCESS);
            }
            if(strcmp(cmd_name, "ls") == 0){
                runLS(args.data());
                exit(EXIT_SUCCESS);
            }
            if (strcmp(cmd_name, "pinfo") == 0) {
                runPinfo(args.data());
                exit(EXIT_SUCCESS);
            }
            if(strcmp(cmd_name, "search") == 0){
                searchFileHelper(args.data());
                exit(EXIT_SUCCESS); 
            }
            if(strcmp(cmd_name, "history") == 0){
                printHistory(args.data());
                exit(EXIT_SUCCESS);
            }
            if(strcmp(cmd_name, "sort") == 0){
                sortIO(args.data());
                exit(EXIT_SUCCESS);
            }
            
            // External command
            execvp(args[0], args.data());
            perror("execvp");
            exit(EXIT_FAILURE);
            
        } else if (pid > 0) {
            child_pids.push_back(pid);
            
            // Close previous read end in parent
            if(prevRead != -1) {
                close(prevRead);
            }
            
            // Close write end of current pipe in parent
            if(i < n - 1) {
                close(pipefd[1]);
            }
            
            // Set read end for next command
            if(i < n - 1) {
                prevRead = pipefd[0];
            } else {
                prevRead = -1;
            }
        } else {
            cerr << fontBold << colorRed << "Fork failed" << reset << endl;
            return;
        }
    }
    
    // Close any remaining read end in parent
    if(prevRead != -1) {
        close(prevRead);
    }
    
    // Wait for all children to complete
    for(pid_t pid : child_pids) {
        int status;
        waitpid(pid, &status, 0);
    }
    
    // Restore original stdin/stdout
    dup2(tempSTDIN, STDIN_FILENO);
    dup2(tempSTDOUT, STDOUT_FILENO);
    close(tempSTDIN);
    close(tempSTDOUT);
}

void executeCommands(char* line) {
    char* saveptr = nullptr;
    char* command = strtok_r(line, ";", &saveptr);
    
    while (command != nullptr) {
        // Trim whitespace
        string trimmed(command);
        size_t start = trimmed.find_first_not_of(" \t\n");
        size_t end = trimmed.find_last_not_of(" \t\n");
        
        if (start != string::npos) {
            trimmed = trimmed.substr(start, end - start + 1);
            if (!trimmed.empty()) {
                char* pipeline_command = strdup(trimmed.c_str());
                executePipeline(pipeline_command);
                free(pipeline_command);
            }
        }
        
        command = strtok_r(nullptr, ";", &saveptr);
    }
}

#endif // PIPELINE_H