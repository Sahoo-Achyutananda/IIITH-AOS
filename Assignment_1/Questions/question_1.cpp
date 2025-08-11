#include<iostream>
#include<cstring>
#include<unistd.h>
#include<fcntl.h>
#include<errno.h>
#include<sys/stat.h>

using namespace std;

const int bufferSize = 4096;
void throwError();
void showProgress(unsigned long long processed, unsigned long long total);
void showDetails(string fileName, string outputPath, unsigned long long fileSize);
void reverseBuffer(char *buffer, unsigned long long size);
void reverseBlocks(char* name, unsigned long long blockSize); // flag 0
void reverseComplete(char* fileName); // flag 1
void reverseRange(char* name, unsigned long long startIndex, unsigned long long endIndex); // flag 2

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

    if(flag == 0){  
        if(argc == 4){
            blockSize = std::atoi(argv[3]); // assigning block
        }else{
            throwError();
            return 1;
        }
    }

    if(flag == 2){
        if(argc == 5){
            startIndex = std::atoi(argv[3]);
            endIndex = std::atoi(argv[4]);
        }else{
            throwError();
            return 1;
        }
    }

    switch(flag){
        case 0 :
            reverseBlocks(fileName, blockSize);
            break;
        case 1 :
            reverseComplete(fileName);
            break;
        case 2 :
            reverseRange(fileName, startIndex, endIndex);
            break;
    }
}

void throwError(){
    std::cerr<<"Command Format : ./a.out <input_file_name> <flag : 0 | 1 | 2 > [offset] | [start_index] [end_index] \n";
}

void showDetails(string fileName, string outputPath, unsigned long long fileSize){
    cout << "Source File Name : " << fileName << endl;
    cout << "Destination File Path : " << outputPath << endl;
    cout << "File Size : " << fileSize << endl;
}

void showProgress(unsigned long long processed, unsigned long long total) {
    const int barWidth = 50;
    static bool firstCall = true;
    
    // // Hide cursor on first call
    // if (firstCall) {
    //     std::cout << "\033[?25l"; // Hide cursor
    //     firstCall = false;
    // }

    const char* done = "█";
    const char* beingProcessed = ">";
    const char* notDone = " ";

    float progress = (float)processed / total;
    progress = std::min(1.0f, progress); // Clamp to 100%
    int pos = barWidth * progress;
    std::cout << "\r\033[K";
    
    std::cout << "Processing... [";
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << done;
        else if (i == pos) std::cout << beingProcessed;
        else std::cout << notDone;
    }
    std::cout << "] " << int(progress * 100.0) << "% (" 
              << processed << "/" << total << ")\r";
    std::cout.flush();
    
    // // Show cursor and newline when complete
    // if (progress >= 1.0f) {
    //     std::cout << "\033[?25h" << std::endl;
    // }
}

void reverseBuffer(char *buffer, unsigned long long size){
    for(int i = 0; i< size/2; i++){
        std::swap(buffer[i], buffer[size - i - 1]);
    }
}

void reverseBlocks(char *name, unsigned long long blockSize){
    string fileName = name;
    unsigned long long fileDescRead = open(fileName.c_str(), O_RDONLY); 
    if(fileDescRead < 0){
        std::cerr << "Error Opening Source File : " << strerror(errno) << "\n";
        return;
    }

    string outputPath = "Assignment1/0_" + fileName;
    mkdir("Assignment1", 0777); // 0777 - grants full permission - (read, write, and execute)
    unsigned long long fileDescWrite = open(outputPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);// create a new file to write the reversed version of the source file
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
    lseek(fileDescRead, 0, SEEK_SET);
    // char buffer[blockSize]; stack allocation gave an error - segmentation fault

    // show general information - 

    showDetails(fileName, outputPath, fileSize);
    cout << "Block Size : " << blockSize << endl;

    char * buffer = new char[blockSize];
    unsigned long long bytesRead = 9999; // set to a large value initially - to enter the while loop | do while loop gave some bugs
    unsigned long long bytesProcessed = 0;
    while(bytesRead > 0){
        bytesRead = read(fileDescRead,buffer,blockSize); // Note : the cursor is automatically moved one byte at a time in the write operation, therefore lseek is not needed !
        if(bytesRead <= 0) break; 
        reverseBuffer(buffer, bytesRead);
        write(fileDescWrite,buffer,bytesRead); 
        bytesProcessed+=bytesRead;
        showProgress(bytesProcessed, fileSize);
    }

    if(bytesRead < 0){
        cerr<< "Error reading Source File : " << strerror(errno) << "\n";
    }

    cout << "File writing completed !" << "\n";
    delete [] buffer;
    close(fileDescRead);
    close(fileDescWrite);

    
}

void reverseComplete(char * name){
    string fileName = name;
    long long fileDescRead = open(name, O_RDONLY); 
    if (fileDescRead < 0) {
        cout << "File Name : " << fileName << endl;
        std::cerr << "Error Opening Source File : " << strerror(errno) << "\n";
        return;
    }
    string outputPath = "Assignment1/1_" + string(name);
    mkdir("Assignment1", 0700); // directory : only user has permission to read + write + execute

    long long fileDescWrite = open(outputPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR, 0600);// create a new file to write the reversed version of the source file
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

    showDetails(fileName, outputPath, fileSize);

    char buffer[bufferSize];
    unsigned long long remaining = fileSize;
    unsigned long long bytesProcessed = 0;

    while(remaining > 0){
        unsigned long long toRead = remaining >= bufferSize ? bufferSize : remaining ;
        unsigned long long offset = remaining - toRead ;

        lseek(fileDescRead, offset, SEEK_SET);

        long long bytesRead = read(fileDescRead, buffer, toRead);
        if (bytesRead < 0) {
            std::cerr << "Error reading input: " << strerror(errno) << "\n";
            break;
        }
        reverseBuffer(buffer, bytesRead);
        long long bytesWritten = write(fileDescWrite, buffer, bytesRead);
        if (bytesWritten < 0) {
            std::cerr << "Error writing to output: " << strerror(errno) << "\n";
            break;
        }
        bytesProcessed+=bytesRead;
        showProgress(bytesProcessed, fileSize);
        remaining = remaining - bytesRead;
    }
    
    std::cout << "Operation Completed \n";

    close(fileDescRead);
    close(fileDescWrite);
    return;
}

void reverseRange(char* name, unsigned long long start, unsigned long long end) {
    string fileName = name;
    int fileDescRead = open(name, O_RDONLY); 
    if (fileDescRead < 0) {
        std::cerr << "Error Opening Source File : " << strerror(errno) << "\n";
        return;
    }

    string outputPath = "Assignment1/2_ " + string(name);
    mkdir("Assignment1", 0777); 
    int fileDescWrite = open(outputPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fileDescWrite < 0) {
        std::cerr << "Error Creating Destination File : " << strerror(errno) << "\n";
        close(fileDescRead);
        return;
    }

    unsigned long long fileSize = lseek(fileDescRead, 0, SEEK_END);
    lseek(fileDescRead, 0, SEEK_SET);

    // Validate indices
    if (start >= fileSize || end >= fileSize || start > end || start < 0 || end < 0) {
        std::cerr << "Invalid start/end indices.\n";
        close(fileDescRead); 
        close(fileDescWrite);
        return;
    }
    showDetails(fileName, outputPath, fileSize);
    cout << "Start Offset : " << start << endl;
    cout << "End Offset : " << end << endl;

    char* buffer = new char[bufferSize];
    unsigned long long bytesProcessed = 0;

    // === Reverse left part (0 to start-1) ===
    unsigned long long leftEnd = start;
    while (leftEnd > 0) {
        unsigned long long chunkSize = (leftEnd >= bufferSize) ? bufferSize : leftEnd;
        /*dry run : 
        let start = 4 ABCDE : DCBAE
        4 >= 4096 -> false : therefore chunkSize = 4
        offset = 4 - 4 = 0
        */

        unsigned long long offset = leftEnd - chunkSize;
        lseek(fileDescRead, offset, SEEK_SET);
        int bytesRead = read(fileDescRead, buffer, chunkSize);
        reverseBuffer(buffer, bytesRead);
        write(fileDescWrite, buffer, bytesRead);
        bytesProcessed+=bytesRead;
        showProgress(bytesProcessed, fileSize);
        leftEnd = offset;
    }

    // === Copy middle part (start to end) as-is ===
    lseek(fileDescRead, start, SEEK_SET);
    unsigned long long copyLen = end - start + 1;
    while (copyLen > 0) {
        unsigned long long chunkSize = (copyLen >= bufferSize) ? bufferSize : copyLen;
        int bytesRead = read(fileDescRead, buffer, chunkSize);
        write(fileDescWrite, buffer, bytesRead);
        copyLen -= bytesRead;
        bytesProcessed+=bytesRead;
        showProgress(bytesProcessed, fileSize);
    }

    // // === Reverse right part (end+1 to EOF) ===
    unsigned long long rightStart = end + 1;
    unsigned long long rightEnd = fileSize;
    

    while (rightEnd > rightStart) {
        unsigned long long chunkSize = (rightEnd - rightStart >= bufferSize) ? bufferSize : (rightEnd - rightStart);
        unsigned long long offset = rightEnd - chunkSize;
        lseek(fileDescRead, offset, SEEK_SET);
        int bytesRead = read(fileDescRead, buffer, chunkSize);
        reverseBuffer(buffer, bytesRead);
        write(fileDescWrite, buffer, bytesRead);
        bytesProcessed+=bytesRead;
        showProgress(bytesProcessed, fileSize);
        rightEnd = offset;
    }

    delete[] buffer;
    std::cout << "Operation Completed \n";

    close(fileDescRead);
    close(fileDescWrite);
    return;
}