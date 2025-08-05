#include<iostream> // for cerr : primatily used to output error messages or diagnostic information
#include<fcntl.h>
#include<unistd.h>
#include<cstring>
#include<errno.h>

/*
GOAL : Opening a file and printing it's content
1. accept a filname as a command line argument along with the offset <start and end > 
end offset is not mandatory
2. opens the file
3. reads the content of the file
4. prints the content to the console.
*/
int main(int argc, char* argv[]){
    /*argc - argument count, argv - argument vector : they are inbuilt, the OS provides these */
    if(argc < 3){
        // if we recieve less than 3 arguments - we return 
        std::cerr << "Usage: ./a.out <file_name> <start_offset> [end_offset]\n";
        return 1;
    }
    const char* fileName = argv[1];
    int start = std::atoi(argv[2]);

    // checking file - 
    int fileDesc = open(fileName, O_RDONLY);
    if(fileDesc < 0){
        std::cerr << "Error Opening File / Error No : " << errno << ": " << strerror(errno) << "\n";
        return 1;
    }
    // error handling : checking start_offset out of bound !
    off_t fileSize = lseek(fileDesc, 0, SEEK_END);
    if(start >= fileSize){
        std::cerr << "Start offset is beyond end of the file. \n";
        close(fileDesc);
        return 1;
    }

    // setting end offset - check is user has passed the end_offset and then proceed further - 
    int end  = -1;
    if(argc >= 4){
        end = std::atoi(argv[3]);
        if(end < start){
            std::cerr << "End Offset must be >= start offset";
            close(fileDesc);
            return 1;
        }
        if(end >= fileSize) end = fileSize - 1;
    }else{
        end = fileSize - 1;
    }

    const int bufferSize = end - start + 1;
    char buffer[bufferSize];
    ssize_t bytesRead;

    lseek(fileDesc, start, SEEK_SET); //moving to the start offset

    do{
        bytesRead = read(fileDesc, buffer, bufferSize);
        write(STDOUT_FILENO, buffer, bytesRead); 
    }while(bytesRead > 0);

    if(bytesRead < 0){
        std::cerr << "Error Reading File : " << strerror(errno) << "\n";
    }

    close(fileDesc); // closing the file
    return 0;

}