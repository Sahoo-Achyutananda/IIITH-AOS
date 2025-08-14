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

// functions and constants -
const int bufferSize = 4096;
void throwError();
void showProgress(unsigned long long processed, unsigned long long total);
void showSuccessMessage();
void showErrorMessage(const char * message, bool showErno);
void showTaskDescription(int type, char* fileName);
void showDetails(string fileName, string outputPath, unsigned long long fileSize);
void reverseBuffer(char *buffer, unsigned long long size); //helper
void reverseBlocks(char* name, unsigned long long blockSize); // flag 0
void reverseComplete(char* fileName); // flag 1
void reverseRange(char* name, unsigned long long startIndex, unsigned long long endIndex); // flag 2

int main(int argc, char* argv[]){
    // command format - ./a.out <input file name> <flag> => total count = 3
    auto start = high_resolution_clock::now();
    if(argc < 3){ // mininum 3 arguments are needed !
        throwError();
        return 1;
    }

    char * fileName = argv[1];
    int flag =  atoi(argv[2]);

    // prelimenary checks 
    long long fd = open(fileName, O_RDONLY);
    if(fd < 0){
        showErrorMessage("Error reading File Path", true);
        close(fd);
        return 1;
    }

    unsigned long long fileSize = lseek(fd,0,SEEK_END);
    if(fileSize == 0){
        showErrorMessage("File is Empty", false);
        close(fd);
        return 1;
    }

    // handling flag Out of Bound error -
    if(flag < 0 || flag > 2){
        showErrorMessage("Invalid Flag", false);
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
            showErrorMessage("Incorrect Flag", false);
            throwError();
            break;
    }

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(stop - start);

    chmod("Assignment1", 0700);

    cout << fontBold << colorGreen << fontItalic << "\nProgram Executed in " << duration.count() << "sec" << reset << "\n\n";
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
         cout << "\033[?25l"; // Hide cursor - to hide the movement of the cursor that appears glitchy while the  progress bar is rendering
        firstCall = false;
    }

    // special characters for the progress bar, initially they were set to their respective ASCII codes, but they didnt appear on the screen as expected, the special characters were taken from the web - 
    const char* done = "█";
    const char* beingProcessed = "█";
    const char* notDone = "░";

    float progress = (float)processed / total;
    progress = min(1.0f, progress);
    int pos = barWidth * progress;
    cout << "\r\033[K";
    
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

void showErrorMessage(const char * message, bool showErno){
    if(showErno){
        cerr << fontBold << fontItalic << colorRed << "Error: " << message << " (" << strerror(errno) << ")" << reset << endl;
    }else{
        cerr << fontBold << fontItalic << colorRed << "Error: " << message << reset << endl;
    } 
}

void showTaskDescription(int type, char* fileName){
    cout << "\n" << fontBold << foregroundGreen << " Task " << reset;
    switch(type){
        case 0 :
            cout << string(foregroundBlue) << fontBold << " Block Wise Reversal on file - " << fileName << " " << reset << reset << "\n";
            break;
        case 1 :
            cout << string(foregroundBlue) << fontBold << " Full Reversal of file - " << fileName << " "  << reset << reset << "\n" ;
            break;
        case 2 :
            cout << string(foregroundBlue) << fontBold << " Partial Range Reversal on file - " << fileName << " "  << reset << reset << "\n";
            break;
    }
}

// this function was used to handle a block size of 1 -
void copyFile(const char* source, const char* destination){
    long long sourcefd = open(source, O_RDONLY);
    // if(sourcefd < 0){
    //     showErrorMessage("Error Opening Source file ! ", true);
    //     return;
    // }
    unsigned long long fileSize = lseek(sourcefd, 0, SEEK_END);
    lseek(sourcefd, 0 ,SEEK_SET);

    long long destfd = open(destination, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if(destfd < 0){
        showErrorMessage("Error Opening Source file ! ", true);
        return;
    }

    char* buffer = new char[bufferSize];
    unsigned long long bytesRead;
    unsigned long long bytesProcessed = 0;

    while ((bytesRead = read(sourcefd, buffer, bufferSize)) > 0) {
        unsigned long long bytesWritten = write(destfd, buffer, bytesRead);
        if (bytesWritten < 0) {
            showErrorMessage("Error writing to output file", true);
            close(sourcefd);
            close(destfd);
            return;
        }
        bytesProcessed+=bytesWritten;
        showProgress(bytesProcessed, fileSize);
    }

    if (bytesRead < 0) {
        showErrorMessage("Error reading input file", true);
    }
    showSuccessMessage();
    delete [] buffer;
    close(sourcefd);
    close(destfd);

}

// used as a helper function
void reverseBuffer(char *buffer, unsigned long long size){
    for(int i = 0; i< size/2; i++){
        swap(buffer[i], buffer[size - i - 1]);
    }
}


// Flag 0
void reverseBlocks(char *name, unsigned long long blockSize){
    // edge case : invalid block size
    if(blockSize <= 0){
        showErrorMessage("Block Size must be greater than 0", false);
        return;
    }
    
    string fileName = name;
    long long fileDescRead = open(fileName.c_str(), O_RDONLY); 
    // // edge case : invalid file name/ file doesnt exist
    // if(fileDescRead < 0){
    //     showErrorMessage("Error Opening Source File . ", true); 
    //     close(fileDescRead);
    //     return;
    // }
    unsigned long long fileSize = lseek(fileDescRead, 0 ,SEEK_END);
    // edge case : block size larger than the file size
    if(blockSize >= fileSize){
        cout << fontBold << colorBlue << "Performing Full file reversal - (Block Size is greater than File size)" << reset << endl;
        reverseComplete(name);
        return;
    }

    // if(fileSize == 0){
    //     showErrorMessage("The File is Empty", false);
    //     close(fileDescRead);
    //     return;
    // }

    string outputPath = "Assignment1/0_" + fileName;  

    mkdir("Assignment1", 0700);  // Force 700 permissions - didnt work used chmod at the end of the function

    unsigned long long fileDescWrite = open(outputPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);// create a new file to write the reversed version of the source file
    if(fileDescWrite < 0){
        showErrorMessage("Error Creating Destination File", true);
        close(fileDescRead);
        return;
    }

    if(fileSize < 0){
        showErrorMessage("Error retrieving File Size", true);
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
    cout << fontBold << colorBlue <<  "\nProcessing ... " << endl;
    
    if(blockSize == 1){
        copyFile(fileName.c_str(), outputPath.c_str());
        return;
    }
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
        showErrorMessage("Error reading Source File", true);
    }

    showSuccessMessage();
    chmod("Assignment1", 0700);
    delete [] buffer;
    close(fileDescRead);
    close(fileDescWrite);
}

//Flag 1
void reverseComplete(char * name){
    string fileName = name;
    long long fileDescRead = open(name, O_RDONLY); 
    // if (fileDescRead < 0) {
    //     showErrorMessage("Error Opening Source File", true);
    //     close(fileDescRead);
    //     return;
    // }
    unsigned long long fileSize = lseek(fileDescRead, 0 ,SEEK_END);
    // if(fileSize == 0){
    //     showErrorMessage("The File is Empty", false);
    //     close(fileDescRead);
    //     return;
    // }
    string outputPath = "Assignment1/1_" + string(name);

    mkdir("Assignment1", 0700);  // Force 700 permissions
    

    long long fileDescWrite = open(outputPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR, 0600);// create a new file to write the reversed version of the source file
    if(fileDescWrite < 0){
        showErrorMessage("Error Creating Destination File", true);
        return;
    }

    if(fileSize < 0){
        showErrorMessage("Error retrieving File Size", true);
        close(fileDescRead);
        close(fileDescWrite);
        return;
    }
    showTaskDescription(1,name);
    showDetails(fileName, outputPath, fileSize);
    cout << fontBold << colorBlue <<  "\nProcessing ... " << endl;

    char buffer[bufferSize];
    unsigned long long remaining = fileSize;
    unsigned long long bytesProcessed = 0;

    while(remaining > 0){
        unsigned long long toRead = remaining >= bufferSize ? bufferSize : remaining ;
        unsigned long long offset = remaining - toRead ;

        lseek(fileDescRead, offset, SEEK_SET);

        long long bytesRead = read(fileDescRead, buffer, toRead);
        if (bytesRead < 0) {
            showErrorMessage("Error Reading Input", true);
            break;
        }
        reverseBuffer(buffer, bytesRead);
        long long bytesWritten = write(fileDescWrite, buffer, bytesRead);
        if (bytesWritten < 0) {
            showErrorMessage("Error Writing to Output", true);
            break;
        }
        bytesProcessed+=bytesRead;
        showProgress(bytesProcessed, fileSize);
        remaining = remaining - bytesRead;
    }
    
    showSuccessMessage();
    chmod("Assignment1", 0700);
    close(fileDescRead);
    close(fileDescWrite);
    return;
}

// Flag 2
void reverseRange(char* name, unsigned long long start, unsigned long long end) {
    string fileName = name;
    int fileDescRead = open(name, O_RDONLY); 
    // if (fileDescRead < 0) {
    //     showErrorMessage("Error Opening Source File", true);
    //     return;
    // }
    unsigned long long fileSize = lseek(fileDescRead, 0, SEEK_END);
    // if(fileSize == 0){
    //     showErrorMessage("The File is Empty", false);
    //     close(fileDescRead);
    //     return;
    // }

    string outputPath = "Assignment1/2_" + string(name);
 
    mkdir("Assignment1", 0700);

    int fileDescWrite = open(outputPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fileDescWrite < 0) {
        showErrorMessage("Error Creating Destination File", true);
        close(fileDescRead);
        return;
    }

    lseek(fileDescRead, 0, SEEK_SET);

    // Validate indices
    if (start >= fileSize || end >= fileSize || start > end || start < 0 || end < 0) {
        showErrorMessage("Invalid start/end indices", false);
        close(fileDescRead); 
        close(fileDescWrite);
        return;
    }
    showTaskDescription(2,name);
    showDetails(fileName, outputPath, fileSize);
    cout << "Start Offset : " << colorBlue << fontBold << start << reset << endl;
    cout << "End Offset : " << colorBlue << fontBold <<  end  << reset << endl;
    cout << fontBold << colorBlue <<  "\nProcessing ... " << endl;

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
    chmod("Assignment1", 0700);
    delete [] buffer;
    close(fileDescRead);
    close(fileDescWrite);
    return ;
}