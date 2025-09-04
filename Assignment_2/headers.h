// all header files go here - 

#if !defined(HEADERS_H)
#define HEADERS_H

#include <unistd.h>
#include <iostream>
#include <string>
#include <vector>
#include <limits.h>
#include <error.h>
#include <cstring>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <iomanip>
#include <ctime>
#include <iostream>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>

const long long CHAR_LEN_MAX = 256;
std::string shellHome; // it'll be set by the main function - cannot hard code it coz path may change if kept in another directory
std::string historyPath;


#endif // HEADERS_H;
