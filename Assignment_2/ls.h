#if !defined(LS_H)
#define LS_H

#include "headers.h"
using namespace std;

void printFileLong(const string &path, const string &name, const struct stat &st){
    // file type
    cout << (S_ISDIR(st.st_mode) ? "d" : "-");

    // permissions
    cout << ((st.st_mode & S_IRUSR) ? "r" : "-");
    cout << ((st.st_mode & S_IWUSR) ? "w" : "-");
    cout << ((st.st_mode & S_IXUSR) ? "x" : "-");
    cout << ((st.st_mode & S_IRGRP) ? "r" : "-");
    cout << ((st.st_mode & S_IWGRP) ? "w" : "-");
    cout << ((st.st_mode & S_IXGRP) ? "x" : "-");
    cout << ((st.st_mode & S_IROTH) ? "r" : "-");
    cout << ((st.st_mode & S_IWOTH) ? "w" : "-");
    cout << ((st.st_mode & S_IXOTH) ? "x" : "-");

    // number of links
    cout << " " << setw(2) << st.st_nlink;

    // owner and group
    struct passwd *pw = getpwuid(st.st_uid);
    struct group  *gr = getgrgid(st.st_gid);
    cout << " " << (pw ? pw->pw_name : to_string(st.st_uid));
    cout << " " << (gr ? gr->gr_name : to_string(st.st_gid));

    // size
    cout << " " << setw(6) << st.st_size;
    // time (last modified)
    char timebuf[80];
    strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", localtime(&st.st_mtime));
    cout << " " << timebuf;

    // name
    cout << " " << name << endl;
}

void listDirectory(const string &path, bool a, bool l){
    DIR *dir = opendir(path.c_str());
    if(!dir){
        cerr << "error opening path : " << path << endl;
        return;
    }

    struct dirent *entry;
    // entry = readdir(dir);
    long long totalBlocks = 0;
    
    while((entry = readdir(dir))!= NULL){
        string name = entry->d_name;

        if(!a && name[0] == '.') continue;

        string fullPath = path + "/" + name;

        struct stat st;
        if(stat(fullPath.c_str(), &st) == -1){
            cerr << "error retrieving file info" << endl;
            continue;
        }
        if (l)
            printFileLong(path, name, st);
        else
            cout << name << endl;
    }

    // cout << endl;

    closedir(dir);
}


void runLS(char *args[]){
    bool a = false;
    bool l = false;

    string path[20]; // to handle directory paths given with ls
    int pathcount = 0;

    for(int i = 1; args[i] != NULL; i++){
        // -l and -a strats with - , this part handles that
        if(args[i][0] == '-'){
            for(int j = 1; args[i][j] != '\0'; j++){
                if(args[i][j] == 'a') a = true;
                else if(args[i][j] == 'l') l = true;
            }
        }
        else{
            string p = args[i];
            if(p == "~") p = shellHome;
            path[pathcount++] = p;
        }
    }

    // if no paths are passed - 
    if(pathcount == 0)
        path[pathcount++] = "."; // fallback to the current directory
    
    // path has atleast one path fpr which the details of the files has to be extracted - 
    for(int i = 0; i < pathcount; i++){
        struct stat st;
        if(pathcount > 1){
            cout << path[i] << ":" << endl;
        }
        if(stat(path[i].c_str(), &st) == -1){
            cerr << "error opening path : " << path[i] << endl;
            continue;
        }

        if(S_ISDIR(st.st_mode))
            listDirectory(path[i], a,l);
        else{
            if(l)
                printFileLong(".", path[i], st);
            else
                cout << path[i] << endl;
        }

        if (i < pathcount - 1) cout << endl;
    }

}

#endif // LS_H
