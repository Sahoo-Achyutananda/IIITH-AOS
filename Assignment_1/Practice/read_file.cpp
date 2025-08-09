#include<iostream> // for cerr : primatily used to output error messages or diagnostic information
#include<fcntl.h>
#include<unistd.h>
#include<cstring>
#include<errno.h>

/*
Opening a file and printing it's content
1. accept a filname as a command line argument
2. opens the file
3. reads the content of the file
4. prints the content to the console.
*/
int main(int argc, char* argv[]){
    /*argc - argument count, argv - argument vector : they are inbuilt, the OS provides these */
    if(argc < 2){
        // if we recieve less than 2 arguments - we return 
        std::cerr << "Usage: ./a.out <file_name> \n";
        return 1;
    }
    const char* fileName = argv[1];
    int fileDesc = open(fileName, O_RDONLY); // readonly mode is specified
    /*a file descriptor (FD) is a non-negative integer that serves as a unique identifier or 
    handle for an open file or other input/output (I/O) resource within a process.  */
    if(fileDesc < 0){
        std::cerr << "Error Opening File / Error No : " << errno << ": " << strerror(errno) << "\n";
        return 1;
        /* errno is a global variable that exists per process -> whenever a system call returns an error 
        - usually < 0, the errno variable is set to the corresponding error code - defined in errno.h
        strerror() is used to convert a errno to a human redable error message !
        */
    }

    const int bufferSize = 1024;
    char buffer[bufferSize];
    ssize_t bytesRead; // initially set to a large value, ssize_t is similar to int

    do{
        bytesRead = read(fileDesc, buffer, bufferSize);
        // read returns a value != 0 when there is still content exists
        write(STDOUT_FILENO, buffer, bytesRead);
    }while(bytesRead > 0);

    if(bytesRead < 0){
        std::cerr << "Error Reading File : " << strerror(errno) << "\n";
    }

    close(fileDesc); // closing the file
    return 0;

}