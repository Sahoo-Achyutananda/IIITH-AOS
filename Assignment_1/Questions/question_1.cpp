#include<iostream>
#include<cstring>
#include<unistd.h>
#include<fcntl.h>
#include<chrono> // to track time
#include<errno.h>
#include<sys/stat.h>

using namespace std;
using namespace std::chrono;

// colors and font styles
const char* colorRed = "\033[91m"; // bright red
const char* colorGreen = "\033[92m"; // bright green 
const char* colorYellow = "\033[93m"; // bringht yellow 
const char* colorBlue = "\033[94m"; // bright blue 
const char* foregroundGreen = "\033[102m";
const char* foregroundBlue = "\033[104m"; 
const char* foregroundRed = "\033[101m";
const char* reset = "\033[0m";
const char* fontBold = "\033[1m";
const char* fontItalic = "\033[3m";

// modules and constants -
const int bufferSize = 4096;
void throwError();
void showProgress(unsigned long long processed, unsigned long long total);
void showSuccessMessage();
void showTaskDescription(int type, char* fileName);
void showDetails(string fileName, string outputPath, unsigned long long fileSize);
void reverseBuffer(char *buffer, unsigned long long size);
void reverseBlocks(char* name, unsigned long long blockSize); // flag 0
void reverseComplete(char* fileName); // flag 1
void reverseRange(char* name, unsigned long long startIndex, unsigned long long endIndex); // flag 2

int main(int argc, char* argv[]){
    // command format - ./a.out <input file name> <flag> => total count = 3
    auto start = high_resolution_clock::now();
    if(argc < 3){ // MINIMUM 3 arguments are needed !
        throwError();
        return 1;
    }

    char * fileName = argv[1];
    int flag =  atoi(argv[2]);

    // handling flag Out of Bound error -
    if(flag < 0 || flag > 2){
         cerr << colorRed << fontBold << "Invalid Flag" << "\n" << reset;
         throwError();
        return 1;
    }

    int blockSize = 0;
    int startIndex = 0;
    int endIndex = 0;

    if(flag == 0){  
        if(argc == 4){
            blockSize =  atoi(argv[3]); // assigning block
        }else{
            throwError();
            return 1;
        }
    }

    if(flag == 2){
        if(argc == 5){
            startIndex =  atoi(argv[3]);
            endIndex =  atoi(argv[4]);
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
        default :
            throwError();
            break;
    }

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(stop - start);

    cout << fontBold << colorGreen << fontItalic << "File written in " << duration.count() << "sec" << reset << "\n\n";
}

void throwError(){
     cerr<< colorRed << fontBold << "Command Format : ./a.out <input_file_name> <flag : 0 | 1 | 2 > [offset] | [start_index] [end_index] \n" << reset;
}

void showDetails(string fileName, string outputPath, unsigned long long fileSize){
    cout << "Source File Name : " << fontBold << colorBlue << fileName << reset << endl;
    cout << "Destination File Path : " << fontBold << colorBlue << outputPath << reset <<  endl;
    cout << "File Size : " << fontBold << colorBlue << fileSize << reset << endl;
}

void showProgress(unsigned long long processed, unsigned long long total) {
    const int barWidth = 50;
    static bool firstCall = true;
    
    if (firstCall) {
         cout << "\033[?25l"; // Hide cursor
        firstCall = false;
    }

    const char* done = "█";
    const char* beingProcessed = "█";
    const char* notDone = "░";

    float progress = (float)processed / total;
    progress = min(1.0f, progress);
    int pos = barWidth * progress;
    cout << "\r\033[K";
    // cout << "Processing... \r";
    // cout.flush();
    
    cout << fontBold << colorBlue;
    cout << "[";
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) cout << done;
        else if (i == pos) cout << beingProcessed;
        else cout << notDone;
    }
    cout << "] " << int(progress * 100.0) << "% (" 
              << processed << "/" << total << ")\r";
    cout.flush();
    
    cout << reset;

    // Show cursor and newline when complete
    if (progress >= 1.0f) {
         cout << "\033[?25h";
    }
}

void showSuccessMessage(){
    cout << fontBold << colorBlue << "\nOperation Completed \n" << reset;
}

void showTaskDescription(int type, char* fileName){
    cout << fontBold << string(foregroundGreen) << "\n Task " << reset;
    switch(type){
        case 0 :
            cout << string(foregroundBlue) << fontBold << " Block Wise Reversal on file - " << fileName << " " << reset << endl;
            break;
        case 1 :
            cout << string(foregroundBlue) << fontBold << " Full Reversal of file - " << fileName << reset  << " " << endl ;
            break;
        case 2 :
            cout << string(foregroundBlue) << fontBold << " Partial Range Reversal on file - " << fileName << " "  << reset << endl;
            break;
    }
}

void reverseBuffer(char *buffer, unsigned long long size){
    for(int i = 0; i< size/2; i++){
        swap(buffer[i], buffer[size - i - 1]);
    }
}

void reverseBlocks(char *name, unsigned long long blockSize){
    if(blockSize <= 0){
        cerr << colorRed << fontBold << "Block Size must be greater than 0" << reset << endl;
        return;
    }
    string fileName = name;
    unsigned long long fileDescRead = open(fileName.c_str(), O_RDONLY); 
    if(fileDescRead < 0){
        cerr << "Error Opening Source File : " << strerror(errno) << "\n";
        return;
    }
    unsigned long long fileSize = lseek(fileDescRead, 0 ,SEEK_END);
    if(blockSize > fileSize){
        cerr << colorRed << fontBold << "Block Size exceeds File Size" << reset << endl;
        return;
    }

    string outputPath = "Assignment1/0_" + fileName;
    mkdir("Assignment1", 0777); // 0777 - grants full permission - (read, write, and execute)
    unsigned long long fileDescWrite = open(outputPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);// create a new file to write the reversed version of the source file
    if(fileDescWrite < 0){
        cerr << "Error Creating Destination File : " << strerror(errno) << "\n";
        close(fileDescRead);
        return;
    }

    if(fileSize < 0){
        cerr << "Error retrieving File Size : " << strerror(errno) << "\n";
        close(fileDescRead);
        close(fileDescWrite);
        return;
    }
    lseek(fileDescRead, 0, SEEK_SET);
    // char buffer[blockSize]; stack allocation gave an error - segmentation fault

    // show general information - 
    showTaskDescription(0,name);
    showDetails(fileName, outputPath, fileSize);
    cout << "Block Size : " << fontBold << colorBlue << blockSize << reset << endl;

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

    showSuccessMessage();
    delete [] buffer;
    close(fileDescRead);
    close(fileDescWrite);

    
}

void reverseComplete(char * name){
    string fileName = name;
    long long fileDescRead = open(name, O_RDONLY); 
    if (fileDescRead < 0) {
        cout << "File Name : " << fileName << endl;
         cerr << "Error Opening Source File : " << strerror(errno) << "\n";
        return;
    }
    string outputPath = "Assignment1/1_" + string(name);
    mkdir("Assignment1", 0700); // directory : only user has permission to read + write + execute

    long long fileDescWrite = open(outputPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR, 0600);// create a new file to write the reversed version of the source file
    if(fileDescWrite < 0){
         cerr << "Error Creating DEstination File : " << strerror(errno) << "\n";
        return;
    }

    unsigned long long fileSize = lseek(fileDescRead, 0 ,SEEK_END);
    if(fileSize < 0){
         cerr << "Error retrieving File Size : " << strerror(errno) << "\n";
        close(fileDescRead);
        close(fileDescWrite);
        return;
    }
    showTaskDescription(1,name);
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
             cerr << "Error reading input: " << strerror(errno) << "\n";
            break;
        }
        reverseBuffer(buffer, bytesRead);
        long long bytesWritten = write(fileDescWrite, buffer, bytesRead);
        if (bytesWritten < 0) {
             cerr << "Error writing to output: " << strerror(errno) << "\n";
            break;
        }
        bytesProcessed+=bytesRead;
        showProgress(bytesProcessed, fileSize);
        remaining = remaining - bytesRead;
    }
    
    showSuccessMessage();

    close(fileDescRead);
    close(fileDescWrite);
    return;
}

void reverseRange(char* name, unsigned long long start, unsigned long long end) {
    string fileName = name;
    int fileDescRead = open(name, O_RDONLY); 
    if (fileDescRead < 0) {
         cerr << "Error Opening Source File : " << strerror(errno) << "\n";
        return;
    }

    string outputPath = "Assignment1/2_" + string(name);
    mkdir("Assignment1", 0777); 
    int fileDescWrite = open(outputPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fileDescWrite < 0) {
         cerr << "Error Creating Destination File : " << strerror(errno) << "\n";
        close(fileDescRead);
        return;
    }

    unsigned long long fileSize = lseek(fileDescRead, 0, SEEK_END);
    lseek(fileDescRead, 0, SEEK_SET);

    // Validate indices
    if (start >= fileSize || end >= fileSize || start > end || start < 0 || end < 0) {
         cerr << "Invalid start/end indices.\n";
        close(fileDescRead); 
        close(fileDescWrite);
        return;
    }
    showTaskDescription(2,name);
    showDetails(fileName, outputPath, fileSize);
    cout << "Start Offset : " << colorBlue << fontBold << start << reset << endl;
    cout << "End Offset : " << colorBlue << fontBold <<  end  << reset << endl;

    char* buffer = new char[bufferSize];
    unsigned long long bytesProcessed = 0;

    // handle 0 to start-1
    unsigned long long region1Len = start;
    unsigned long long pos = region1Len;
    while (pos > 0) {
        unsigned long long toRead = (pos >= bufferSize) ? bufferSize : pos;
        pos -= toRead;
        lseek(fileDescRead, pos, SEEK_SET);
        read(fileDescRead, buffer, toRead);
        reverseBuffer(buffer, toRead);
        write(fileDescWrite, buffer, toRead);
        bytesProcessed += toRead;
        showProgress(bytesProcessed, fileSize);
    }

    // handle start to end
    lseek(fileDescRead, start, SEEK_SET);
    unsigned long long region2Len = end - start + 1;
    while (region2Len > 0) {
        unsigned long long toRead = (region2Len >= bufferSize) ? bufferSize : region2Len;
        read(fileDescRead, buffer, toRead);
        write(fileDescWrite, buffer, toRead);
        region2Len -= toRead;
        bytesProcessed += toRead;
        showProgress(bytesProcessed, fileSize);
    }

    // handle end + 1 to EOF
    unsigned long long region3Start = end + 1;
    unsigned long long region3Len = fileSize - region3Start;
    pos = fileSize;
    while (region3Len > 0) {
        unsigned long long toRead = (region3Len >= bufferSize) ? bufferSize : region3Len;
        pos -= toRead;
        lseek(fileDescRead, pos, SEEK_SET);
        read(fileDescRead, buffer, toRead);
        reverseBuffer(buffer, toRead);
        write(fileDescWrite, buffer, toRead);
        region3Len -= toRead;
        bytesProcessed += toRead;
        showProgress(bytesProcessed, fileSize);
    }

    showSuccessMessage();
    delete [] buffer;
    close(fileDescRead);
    close(fileDescWrite);
    return ;

    // // Reverse left part
    // unsigned long long leftEnd = start;
    // while (leftEnd > 0) {
    //     unsigned long long chunkSize = (leftEnd >= bufferSize) ? bufferSize : leftEnd;
    //     /*dry run : 
    //     let start = 4 ABCDE : DCBAE
    //     4 >= 4096 -> false : therefore chunkSize = 4
    //     offset = 4 - 4 = 0
    //     */

    //     unsigned long long offset = leftEnd - chunkSize;
    //     lseek(fileDescRead, offset, SEEK_SET);
    //     int bytesRead = read(fileDescRead, buffer, chunkSize);
    //     reverseBuffer(buffer, bytesRead);
    //     write(fileDescWrite, buffer, bytesRead);
    //     bytesProcessed+=bytesRead;
    //     showProgress(bytesProcessed, fileSize);
    //     leftEnd = offset;
    // }

    // // Copy middle part
    // lseek(fileDescRead, start, SEEK_SET);
    // unsigned long long copyLen = end - start + 1;
    // while (copyLen > 0) {
    //     unsigned long long chunkSize = (copyLen >= bufferSize) ? bufferSize : copyLen;
    //     int bytesRead = read(fileDescRead, buffer, chunkSize);
    //     write(fileDescWrite, buffer, bytesRead);
    //     copyLen -= bytesRead;
    //     bytesProcessed+=bytesRead;
    //     showProgress(bytesProcessed, fileSize);
    // }

    // //Reverse right part
    // unsigned long long rightStart = end + 1;
    // unsigned long long rightEnd = fileSize;
    

    // while (rightEnd > rightStart) {
    //     unsigned long long chunkSize = (rightEnd - rightStart >= bufferSize) ? bufferSize : (rightEnd - rightStart);
    //     unsigned long long offset = rightEnd - chunkSize;
    //     lseek(fileDescRead, offset, SEEK_SET);
    //     int bytesRead = read(fileDescRead, buffer, chunkSize);
    //     reverseBuffer(buffer, bytesRead);
    //     write(fileDescWrite, buffer, bytesRead);
    //     bytesProcessed+=bytesRead;
    //     showProgress(bytesProcessed, fileSize);
    //     rightEnd = offset;
    // }

    // delete[] buffer;
    // showSuccessMessage();

    // close(fileDescRead);
    // close(fileDescWrite);
    // return;
}