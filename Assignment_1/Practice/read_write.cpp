#include<errno.h>// for errno
#include<cstring>// for strerror
#include<iostream>// for cerr
#include<unistd.h> // for read, write and close
#include<fcntl.h> // for open

const int bufferSize = 4096;

int main(int argc, char* argv[]){
    if(argc <3){
        std::cerr<<"Command Format : ./a.out <file_read> <file_write>" << "\n";
        return 1;
    }

    char *fileRead = argv[1];
    char *fileWrite = argv[2];

    int fileDescRead = open(fileRead, O_RDONLY); // read-only mode
    if(fileDescRead < 0){
        std::cerr << "Error Opening File, Error No :  " << errno << "-" << strerror(errno) << "\n";
        return 1;
    }
    int fileDescWrite = open(fileWrite, O_WRONLY); // write-only mode
    if(fileDescWrite < 0){
        std::cerr << "Error Opening File, Error No :  " << errno << "-" << strerror(errno) << "\n";
        return 1;
    }

    char buffer[bufferSize];
    ssize_t bytesRead, bytesWritten;
    do{
        bytesRead = read(fileDescRead, buffer, bufferSize);
        bytesWritten = write(fileDescWrite, buffer, bytesRead);
        if(bytesWritten < 0){
            std::cerr << "Error Writing into destination file " << strerror(errno) << "\n";
            break;
        }
    }while(bytesRead > 0);

    if(bytesRead < 0){
        std::cerr << "Error Reading Source file " << strerror(errno) << "\n";
    }

    // delete [] buffer;
    close(fileDescRead);
    close(fileDescWrite);

    return 0;

}