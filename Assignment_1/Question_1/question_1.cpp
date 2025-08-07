#include<iostream>
#include<cstring>
#include<unistd.h>
#include<fcntl.h>
#include<errno.h>

const int bufferSize = 4096;
void reverseBuffer(char *buffer, int size);
void reverseComplete(char* fileName);

int main(int argc, char* argv[]){
    // command format - ./a.out <input file name> <flag> => total count = 3

    if(argc < 3){ // MINIMUM 3 arguments are needed !
        std::cerr<<"Command Format : ./a.out <input_file_name> <flag : 0 | 1 | 2 > [offset]";
        return 1;
    }

    char * fileName = argv[1];
    int flag = std::atoi(argv[2]);

    // handling flag Out of Bound error -
    if(flag < 0 || flag >=3){
        std::cerr << "Invalid Flag" << "\n";
        return 1;
    }
    
    switch(flag){
        case 0 :
            break;
        case 1 :
            reverseComplete(fileName);
            break;
        case 2 :
            break;
    }
}

void reverseBuffer(char *buffer, int size){
    for(int i = 0; i< size/2; i++){
        std::swap(buffer[i], buffer[size - i - 1]);
    }
}

void reverseComplete(char * fileName){
    ssize_t fileDescRead = open(fileName, O_RDONLY); 
    if(fileDescRead < 0){
        std::cerr << "Error Opening Source File : " << strerror(errno) << "\n";
        return;
    }

    ssize_t fileDescWrite = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);// create a new file to write the reversed version of the source file
    if(fileDescWrite < 0){
        std::cerr << "Error Creating DEstination File : " << strerror(errno) << "\n";
        return;
    }

    unsigned long long fileSize = lseek(fileDescRead, 0 ,SEEK_END);
    if(fileSize < 0){
        std::cerr << "Error retrieving File Size : " << strerror(errno) << "\n";
        close(fileDescRead);
        close(fileDescWrite);
        return;
    }
    char buffer[bufferSize];
    unsigned long long remaining = fileSize;
    
    while(remaining > 0){
        unsigned long long toRead = remaining >= bufferSize ? bufferSize : remaining ;
        unsigned long long offset = remaining - toRead ;

        lseek(fileDescRead, offset, SEEK_SET);

        ssize_t bytesRead = read(fileDescRead, buffer, toRead);
        if (bytesRead < 0) {
            std::cerr << "Error reading input: " << strerror(errno) << "\n";
            break;
        }
        reverseBuffer(buffer, bytesRead);
        ssize_t bytesWritten = write(fileDescWrite, buffer, bytesRead);
        if (bytesWritten < 0) {
            std::cerr << "Error writing to output: " << strerror(errno) << "\n";
            break;
        }

        remaining = remaining - bytesRead;

    }

    std::cout << "Operation Completed \n";

    close(fileDescRead);
    close(fileDescWrite);
    return;
}
