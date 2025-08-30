#if !defined(AUTOCOMPLETE_H)
#define AUTOCOMPLETE_H

#include "headers.h"

using namespace std;

void redrawCurrentLine(const string& input) {
    // Move to beginning of line and clear entire line
    cout << "\r\033[K" << flush;
    printPrompt();
    cout << input << flush;
}

// helper - to split a stringggggg based on a toooooooken
vector<string> split(const string& s, char delimiter) {
    vector<string> tokens;
    size_t start = 0;

    while (true) {
        size_t pos = s.find_first_of(delimiter, start);
        if (pos == string::npos) {
            tokens.push_back(s.substr(start));
            break;
        }
        tokens.push_back(s.substr(start, pos - start));
        start = pos + 1;
    }
    return tokens;
}

string findCommonPrefix(vector<string>& strings){
    if(strings.empty()) return "";

    string prefix = strings[0];
    for(int i = 1; i < strings.size(); i++){
        int j = 0;
        while(j < prefix.length() && j < strings[i].length() && prefix[j] == strings[i][j])
            j++;
        prefix = prefix.substr(0,j);
    }
    return prefix;
}

void completeWord(string &inp, string &prefix, string &partial,string &completion) {
    // Calculate how many characters to erase
    // int charsToErase = partial.length();
    // for (int i = 0; i < charsToErase; i++) {
    //     cout << "\b \b" << flush; // goback- rrplacewith space - goback
    // }
    
    // // Print the complet
    // cout << completion << flush;
    
    // Update the current input
    inp = prefix + completion;
}

void showCompletions(vector<string> &completions, string & input) {
    // why flush ?????????????
    // the flush manipulator is used to force the output buffer of cout to be sent immediately to the terminal. <idk what that means>
    cout << "\033[s" << flush; // save cursor pos
    cout << endl << flush;
    // cout << "\033[E" << flush; //mpve to the netx line
    for (auto& comp : completions) {
        cout << comp << " ";
    }
    cout << endl;
    
    // cout << "\033[u" << flush; // restore cursorpos
    cout << "\033[K" << flush; //redisplay prompt
    printPrompt();
    cout << input << flush;
}

vector<string> getCommandCompletions(string &inp){

    vector<string> result;
    vector<string> cmds = {
        "echo", "pwd", "cd", "ls", "cat", "pinfo", 
        "search", "history", "clear", "exit"
    };

    for(auto &cmd : cmds){
        if(cmd.find(inp) == 0){
            result.push_back(cmd);
        }
    }

    // checking path - 
    char * path = getenv("PATH");
    vector<string> dirs = split(string(path), ':');

    for(auto &dir : dirs){
        // DIR *d = opendir(dir.c_str());
        // struct dirent* dirInfo;
        // dirInfo = readdir(d);

        // string filename = dirInfo->d_name;
        // if(filename.find(inp) == 0)
        //     result.push_back(filename);
        
        // closedir(d);

            DIR *d = opendir(dir.c_str());
            if (!d) continue;  // in case dir can't be opened sadd

            struct dirent* dirInfo;
            while ((dirInfo = readdir(d))) {
                string filename = dirInfo->d_name;
                if(filename.find(inp) == 0)
                    result.push_back(filename);
            }
            closedir(d);
    }

    // removing duplicates - maybe the commands i've implemented will be found in the PATH dirs- 
    sort(result.begin(), result.end());
    result.erase(unique(result.begin(), result.end()), result.end());

    return result;
}


// vector<string> getFileCompletions(string &inp){
    
//     vector<string> result;
//     DIR * d = opendir(".");

//     struct dirent* dirinfo;
    
//     // need to be ina while loop - > readdir() points to the files iteratively
//     while(dirinfo = readdir(d)){
//         string filename = dirinfo->d_name;
//         if(filename.find(inp) == 0){
//             // need to check if the matching name is a dir or just a file
//             struct stat filestat;
//             stat(filename.c_str(), &filestat);
//             if(S_ISDIR(filestat.st_mode)){
//                 filename+="/";
//             }
//             result.push_back(filename);
//         }
//         closedir(d);
//     }


//     sort(result.begin(), result.end());
//     return result;
// }

vector<string> getFileCompletions(string &inp){
    vector<string> result;
    DIR * d = opendir(".");
    if (!d) return result;

    struct dirent* dirinfo;
    while ((dirinfo = readdir(d))) {
        string filename = dirinfo->d_name;
        if(filename.find(inp) == 0){
            struct stat filestat;
            stat(filename.c_str(), &filestat);
            if(S_ISDIR(filestat.st_mode)){
                filename += "/";
            }
            result.push_back(filename);
        }
    }
    closedir(d);

    sort(result.begin(), result.end());
    return result;
}


void handleAutocomplete(string &input){
    // cout << "hello" << endl;

    if(input.empty())return;

    int lastSpace = input.find_last_of(" \t|;&");
    string prefix = (lastSpace == string::npos) ? "" : input.substr(0,lastSpace + 1); // before the delimiter - jo bhi tha content - usko store karta hai
    string partialWord = (lastSpace == string::npos) ? input : input.substr(lastSpace + 1);

    vector<string> completions;
    // check for command completion or file completions
    if(lastSpace == string::npos || input.find_first_of("|;&") != string::npos){
        completions = getCommandCompletions(partialWord);
    }else{
        completions = getFileCompletions(partialWord);
    }

    if (completions.empty()) {
        // yeh ek sound hai LOL - POSIX is funny
        cout << '\a' << flush;
        return;
    }
    else if (completions.size() == 1) {
        completeWord(input, prefix, partialWord, completions[0]);
        // cout << "\033[u" << flush;
        redrawCurrentLine(input);

    }
    else {
        string commonPrefix = findCommonPrefix(completions);
        if (commonPrefix.length() > partialWord.length()) {
            completeWord(input, prefix, partialWord, commonPrefix);
            // cout << "\033[u" << flush;
            redrawCurrentLine(input);
        } else {
            showCompletions(completions, input);
            // cout << "\033[u" << flush;
            // redrawCurrentLine(input);
        }
    }
}



#endif // AUTOCOMPLETE_H
