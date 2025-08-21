#if !defined(CLEAR_H)
#define CLEAR_H

#include "headers.h"
using namespace std;

void clearScreen(){
    cout << "\033[2J\033[H" ;
}


#endif // CLEAR_H
