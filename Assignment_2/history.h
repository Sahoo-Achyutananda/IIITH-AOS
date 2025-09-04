#if !defined(HISTOTY_H)
#define HISTOTY_H

#include "headers.h"
#include "colors.h"
#include "prompt.h"
#include "autocomplete.h"
#include <termios.h>
#include <vector>
#include <algorithm>
#include <string>

using namespace std;

// this shouldn't have been hardcoded - will not work in other's system
// # define HISTORY "/home/achyutananda-sahoo/Desktop/IIITH-AOS/Assignment_2/.history"
// string historyPath = shellHome + "/.history" ;

int maxHistoryLimt = 20;
int currentHistoryCount = 0;
int navigator = 0;

vector<string> history;

struct termios defaultTerminalSettings;

// diable raw mode

// void disableRawMode(){
//     tcsetattr(STDIN_FILENO, TCSAFLUSH, &defaultTerminalSettings);
// }
// // enable non-canonical mode
// void enableRawMode(){
//     tcgetattr(STDIN_FILENO, &defaultTerminalSettings);
//     // atexit(disableRawMode);

//     // apply new settings
//     struct termios org = defaultTerminalSettings;
//     org.c_lflag &= ~(ECHO | ICANON);
//     org.c_cc[VMIN] = 1;
//     org.c_cc[VTIME] = 0;

//     tcsetattr(STDIN_FILENO,TCSAFLUSH, &org);
// }

// loading the history file
void addToHistory(){
    
    long long fd = open(historyPath.c_str(), O_RDONLY);
    // long long fd = open(HISTORY, O_RDONLY);
    if(fd < 0){
        cerr << fontBold << colorRed << "Error reaading .history file" << reset << endl;
        return;
    }

    char buffer[1024]; // to read from the history file
    long long n = read(fd,buffer,1024); // remember from the first assignment n stores the total bytes read

    string cmd;

    while(n > 0){

        for(long long i = 0; i < n ;i ++){
            if(buffer[i] == '\n'){
                // kabhi kabhi user pagal hoke " " ya "" bhi type karega - inn pagal harkaton ko handle karen ke liye yeh code ka upyog hua hai ... 
                if(cmd.empty() == false){
                    if(history.size() == maxHistoryLimt){
                        history.erase(history.begin());
                    }
                    history.push_back(cmd);
                    cmd.clear();
                }
            }else{
                cmd.push_back(buffer[i]);
            }
        }

        n = read(fd,buffer,1024); // remember fomr the first assignment - the curspor moves auromatically here
    }

    close(fd);
}

// after the terminal exits - we save the commands to the history file
void saveHistory(){
    long long fd = open(historyPath.c_str(), O_WRONLY | O_TRUNC | O_APPEND, 0644);

    // long long fd = open(HISTORY, O_WRONLY | O_TRUNC | O_APPEND, 0644);
    if(fd < 0){
        cerr << fontBold << colorRed << "Error reaading .history file" << reset << endl;
        return;
    }

    for(const auto & cmd : history){
        if(cmd.empty() == false){
            long long n = write(fd,cmd.c_str(), cmd.size());
        }
        long long n = write(fd, "\n", 1);
    }

    close(fd);
}

// to keep updating the history live
void appendToHistory(const char * cmd){
    string scmd = string(cmd);
    if(scmd.empty())
        return;
    
    if(history.size() == maxHistoryLimt)
        history.erase(history.begin());

    history.push_back(scmd);

    saveHistory();
}

void printHistory(char *args[]){
    // debug - 
    // cout << historyPath << endl;

    int argc = countArgs(args);
    if(argc > 2){
        cerr << fontBold << colorRed << "Usage : history OR history <limit>" << reset << endl;
        return;
    }

    if(args[1] == NULL){
        // for(int i =0; i < history.size(); i++){
        //     cout << history[i] << endl;
        // }
        long long lim;
        if(history.size() >= 10)lim = 10;
        else lim = history.size();

        int i = history.size() - lim ;
        while(lim--){
            cout << history[i] << endl;
            i++;
        }
    }else{
        int lim = atoi(args[1]);
        if(lim <= 0){
            cerr << fontBold << colorRed << "History Limit must be a +ve Number" << reset << endl;
            return;
        }
        if(lim > history.size()) lim = history.size();

        for(int i = history.size() - lim; i < history.size(); i++){
            cout << history[i] << endl;
        }
    }
}

// since we are in the raw mode - i have to handle backspaces manually lolllllllllll ... 

string readInput() {
    // Save original terminal settings
    struct termios original_termios;
    tcgetattr(STDIN_FILENO, &original_termios);
    
    // Set up raw mode
    struct termios raw = original_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    
    string input;
    
    navigator = history.size();
    
    while (true) {
        char c;
        if (read(STDIN_FILENO, &c, 1) != 1) {
            break;
        }

        if (c == '\n') {
            // Enter key
            cout << endl;
            break;
        }
        else if (c == 4) {  // ASCII 4 = Ctrl+D
            // RESTORE terminal before exiting
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
            cout << "\nexit" << endl;
            saveHistory();
            exit(0);
        }
        else if (c == 127 || c == '\b') {
            // Backspace
            if (!input.empty()) {
                input.pop_back();
                cout << "\b \b" << flush;
            }
        }
        else if (c == '\t') {
            handleAutocomplete(input);

            // few importnt lines - 
            // cout << "\033[K" << flush;
            // printPrompt();
            // cout << input << flush;

        }
        else if (c == 27) {
            // Escape sequence (likely arrow keys)
            char seq[2];
            if (read(STDIN_FILENO, &seq[0], 1) != 1) continue;
            if (read(STDIN_FILENO, &seq[1], 1) != 1) continue;
            
            if (seq[0] == '[') {
                if (seq[1] == 'A') {
                    // Up arrow
                    if (navigator > 0) {
                        navigator--;
                        // Clear current line and show history item
                        cout << "\033[2K\r";
                        printPrompt();
                        input = history[navigator];
                        cout << input << flush;
                    }
                } 
                else if (seq[1] == 'B') {
                    // Down arrow
                    if (navigator < (int)history.size() - 1) {
                        navigator++;
                        cout << "\033[2K\r";
                        printPrompt();
                        input = history[navigator];
                        cout << input << flush;
                    } 
                    else if (navigator == (int)history.size() - 1) {
                        navigator = history.size();
                        cout << "\033[2K\r";
                        printPrompt();
                        input.clear();
                        cout << flush;
                    }
                }
            }
        }
        else if (isprint(c)) {
            // Printable character
            input += c;
            cout << c << flush;
        }
        // else if (c == 3) {
        //     // Ctrl-C
        //     cout << "^C" << endl;
        //     input.clear();
        //     cout << "\033[2K\r";
        //     printPrompt();
        //     cout << flush;
        //     navigator = history.size();
        // }
    }
    
    // RESTORE original terminal settings before returning
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
    return input;
}

#endif // HISTOTY_H
