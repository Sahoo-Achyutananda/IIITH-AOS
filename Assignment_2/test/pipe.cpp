#include<iostream>
#include<unistd.h>
#include<fcntl.h>
using namespace std;

int main(){
    int arr[] = {3,5,12,8,46,5,21,90};
    int start, end;
    int arrSize = sizeof(arr)/sizeof(int);
    
    int pipe1[2], pipe2[2];
    pipe(pipe1);
    pipe(pipe2);

    if(fork() == 0){
        close(pipe1[0]);

    }

}