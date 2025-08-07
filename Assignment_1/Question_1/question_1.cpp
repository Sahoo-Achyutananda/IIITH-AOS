#include<iostream>
#include<cstring>
#include<unistd.h>
#include<fcntl.h>
#include<errno.h>
#include<sys/stat.h>

using namespace std;

const int bufferSize = 4096;
void throwError();
void reverseBuffer(char *buffer, unsigned long long size);
void reverseComplete(char* fileName);
void reverseBlocks(char* fileName, unsigned long long blockSize);

int main(int argc, char* argv[]){
    // command format - ./a.out <input file name> <flag> => total count = 3

    if(argc < 3){ // MINIMUM 3 arguments are needed !
        throwError();
        return 1;
    }

    char * fileName = argv[1];
    int flag = std::atoi(argv[2]);

    // handling flag Out of Bound error -
    if(flag < 0 || flag > 2){
        std::cerr << "Invalid Flag" << "\n";
        return 1;
    }

    int blockSize = 0;
    int startIndex = 0;
    int endIndex = 0;

    if(flag == 0 && argc == 4){  
        blockSize = std::atoi(argv[3]); // assigning block
    }else{
        throwError();
        return 1;
    }

    if(flag == 3 && argc == 5){
        startIndex = std::atoi(argv[3]);
        endIndex = std::atoi(argv[4]);
    }else{
        throwError();
        return 1;
    }

    switch(flag){
        case 0 :
            reverseBlocks(fileName, blockSize);
            break;
        case 1 :
            reverseComplete(fileName);
            break;
        case 2 :
            break;
    }
}

void throwError(){
    std::cerr<<"Command Format : ./a.out <input_file_name> <flag : 0 | 1 | 2 > [offset] | [start_index] [end_index]";
}

void reverseBuffer(char *buffer, unsigned long long size){
    for(int i = 0; i< size/2; i++){
        std::swap(buffer[i], buffer[size - i - 1]);
    }
}

void reverseBlocks(char *name, unsigned long long blockSize){
    string fileName = name;
    ssize_t fileDescRead = open(fileName.c_str(), O_RDONLY); 
    if(fileDescRead < 0){
        std::cerr << "Error Opening Source File : " << strerror(errno) << "\n";
        return;
    }

    string outputPath = "Assignment1/0_" + fileName;
    mkdir("Assignment1", 0777); // 0777 - grants full permission - (read, write, and execute)
    ssize_t fileDescWrite = open(outputPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);// create a new file to write the reversed version of the source file
    if(fileDescWrite < 0){
        std::cerr << "Error Creating Destination File : " << strerror(errno) << "\n";
        close(fileDescRead);
        return;
    }

    unsigned long long fileSize = lseek(fileDescRead, 0 ,SEEK_END);
    if(fileSize < 0){
        std::cerr << "Error retrieving File Size : " << strerror(errno) << "\n";
        close(fileDescRead);
        close(fileDescWrite);
        return;
    }

    char buffer[blockSize];
    unsigned long long bytesRead;
    do{
        bytesRead = read(fileDescRead,buffer,blockSize);
        reverseBuffer(buffer, blockSize);
        write(fileDescWrite,buffer,bytesRead);
    }while(bytesRead > 0);

    if(bytesRead < 0){
        cerr<< "Error reading Source File : " << strerror(errno) << "\n";
    }

    close(fileDescRead);
    close(fileDescWrite);

    cout << "File writing completed !" << "\n";
    
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
