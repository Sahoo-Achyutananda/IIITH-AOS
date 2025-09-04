#if !defined(FILESEARCH_H)
#define FILESEARCH_H

#include "headers.h"
#include "colors.h"

using namespace std;

bool searchFile(const char *path, const char * target){
    DIR* dir = opendir(path);
    if(!dir) return false;

    struct dirent* entry;
    // there are like 5 fields here that are filled when readdir is called -> the important one is the d_name and d_type
    // It returns NULL on reaching the end of the directory stream or if an error occurred.

    while((entry = readdir(dir)) != NULL){

        // this is important else the search will loop forever 
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        if(strcmp(entry->d_name,target) == 0){
            closedir(dir);
            return true;
        }

        if(entry->d_type == DT_DIR){
            char nextPath[PATH_MAX];
            snprintf(nextPath, PATH_MAX, "%s/%s", path, entry->d_name);
            if(searchFile(nextPath, target)){
                closedir(dir);
                return true;
            }
        }
    }

    closedir(dir);
    return false;

}

void searchFileHelper(char **args){
    int argc = countArgs(args);
    if(argc > 2){
        cerr << fontBold << colorRed << "Usage : search <file_name/folder_name>" << reset << endl;
        return;
    }
    if(args[1] == NULL){
        cerr << colorRed << fontBold << "WRONG USAGE : Missing Argument <filename>" <<  reset << endl ;
        return;
    }

    char * target = args[1];
    // .  : signifies current directory 
    const char * currDir = ".";
    if (searchFile(currDir, args[1])) {
        cout << "True" << endl;
    } else {
        cout << "False" << endl;
    }
}

#endif // FILESEARCH_H
