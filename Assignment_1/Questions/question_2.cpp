#include<iostream>
#include<unistd.h>
#include<cstring>
#include<sys/stat.h>
#include<fcntl.h>
#include<errno.h>

using namespace std;

const unsigned long long bufferSize = 4096; 
bool checkSimilar(char * block1, char* block2, long long blockSize);
void reverseBlock(char * block, unsigned long long blockSize);
bool verifyFlag0(char * modifiedFilePath, char * originalFilePath, unsigned long long blockSize);
bool verifyFlag1(char * modifiedFilePath, char * originalFilePath);
bool verifyFlag2(char * modifiedFilePath, char * originalFilePath, long long startOffset, long long endOffset);
bool verifyRegion(long long fdOrg, long long fdMod, unsigned long long offset, unsigned long long length, bool checkReversed);
void checkPermissions(char * filePath);
int main(int argc, char * argv[]){
    // format : ./a.out <newfilepath> <oldfilepath> <directory> <flag> [<blockSize>|<start> <end>]
    char *modifiedFilePath = argv[1];
    char *originalFilePath = argv[2];
    char *directory = argv[3];
    int flag = atoi(argv[4]);

    long long blockSize = -1;
    long long startOffset = -1;
    long long endOffset = -1;
    if(flag == 0){
        blockSize = atoi(argv[5]);
    }else if(flag == 3){
        startOffset = atoi(argv[5]);
        endOffset = atoi(argv[6]);
    }

    switch(flag){
        case 0:
            if(verifyFlag0(modifiedFilePath, originalFilePath, blockSize)){
                cout << "Whether file contents are correctly processed : YES" << endl;
            }else{
                cout << "Whether file contents are correctly processed : NO" << endl;
            }

            break;
        case 1:
            if(verifyFlag1(modifiedFilePath, originalFilePath)){
                cout << "Whether file contents are correctly processed : YES" << endl;
            }else{
                cout << "Whether file contents are correctly processed : NO" << endl;
            }
            break;
        case 2:
            verifyFlag2(modifiedFilePath, originalFilePath, startOffset, endOffset);
            break;
        default:
            cout << "Invalid Flag" << endl;
            break;
    }

    checkPermissions(modifiedFilePath);
    checkPermissions(originalFilePath);
    checkPermissions(directory);
}

void showProgress(unsigned long long processed, unsigned long long total) {
    const int barWidth = 50;
    static bool firstCall = true;
    

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
    

    // if (progress >= 1.0f) {
    //     std::cout << "\033[?25h" << std::endl;
    // }
}

bool checkSimilar(char* block1, char* block2, long long blockSize){
    long long i = 0;
    while(i < blockSize){
        if(block1[i] != block2[i])
            return false;
        i++;
    }
    return true;
}

void reverseBlock(char * block, unsigned long long blockSize){
    for (long long i = 0; i < blockSize / 2; i++) {
        swap(block[i], block[blockSize - i - 1]);
    }
}

bool verifyFlag0(char * modifiedFilePath, char * originalFilePath, unsigned long long blockSize){
    // 1. get files paths
    // 2. open them using open() sys call
    long long fileDescModifiedFile = open(modifiedFilePath, O_RDONLY);
    long long fileDescOriginalFile = open(originalFilePath, O_RDONLY);
    
    if(fileDescModifiedFile < 0){ 
        cerr << "Error Reading Modofied File " << strerror(errno) << endl;
        return false;
    }
    if(fileDescOriginalFile < 0){ 
        cerr << "Error Reading Original File " << strerror(errno) << endl;
        return false;
    }
    // 3. create buffers to store the data to be read
    char * bufferModified = new char[blockSize];
    char * bufferOriginal = new char[blockSize];
    // 4. compare the buffers
    bool result = true;
    unsigned long long bytesProcessed = 0;
    unsigned long long fileSize = lseek(fileDescOriginalFile, 0, SEEK_END);
    while(true){
        unsigned long long bytesModified = read(fileDescModifiedFile, bufferModified, blockSize);
        unsigned long long bytesOriginal = read(fileDescOriginalFile, bufferOriginal, blockSize);

        if (bytesModified != bytesOriginal) { // the sizes are not barabar
            result = false;
            break;
        }
        if (bytesModified == 0) break; // EOF

        // reverse the modified block to bring back to it's original form
        reverseBlock(bufferModified, bytesModified);
        // check the reversed modifed block and the original block
        if (!checkSimilar(bufferModified, bufferOriginal, bytesModified)) {
            result = false;
            break;
        }

        bytesProcessed+=bytesModified;
        showProgress(bytesProcessed, fileSize);
    }
    // 5. return 
    delete [] bufferModified;
    delete [] bufferOriginal;
    close(fileDescModifiedFile);
    close(fileDescOriginalFile);
    return result;
}

bool verifyFlag1(char * modifiedFilePath, char * originalFilePath) {

    // LOGIC : take k bytes from the org file <from the start>
    // take the same k bytes from the modified file <from the end>
    // reverse one of them
    // compare
    // repeat the process by moving the offsets carefully
    
    int fileDescModifiedFile = open(modifiedFilePath, O_RDONLY);
    int fileDescOriginalFile = open(originalFilePath, O_RDONLY);
    
    if (fileDescModifiedFile < 0) { 
        cerr << "Error Reading Modified File: " << strerror(errno) << endl;
        return false;
    }
    if (fileDescOriginalFile < 0) { 
        cerr << "Error Reading Original File: " << strerror(errno) << endl;
        return false;
    }

    unsigned long long fileSizeModified = lseek(fileDescModifiedFile, 0, SEEK_END);
    unsigned long long fileSizeOriginal = lseek(fileDescOriginalFile, 0, SEEK_END);

    lseek(fileDescModifiedFile, 0, SEEK_SET);
    lseek(fileDescOriginalFile, 0, SEEK_SET);

    if (fileSizeModified != fileSizeOriginal) {
        cout << "File Size are same : NO" << endl;
        close(fileDescModifiedFile);
        close(fileDescOriginalFile);
        return false;
    } else {
        cout << "File Size are same : YES" << endl;
    }

    char * bufferModified = new char[bufferSize];
    char * bufferOriginal = new char[bufferSize];

    unsigned long long orgPos = 0;
    unsigned long long revPos = fileSizeModified;
    bool result = true;

    while (orgPos < fileSizeOriginal) {
        unsigned long long toRead = (fileSizeOriginal - orgPos < bufferSize) 
                                     ? fileSizeOriginal - orgPos 
                                     : bufferSize;

        // Read from start of original file
        lseek(fileDescOriginalFile, orgPos, SEEK_SET);
        unsigned long long bytesReadOriginal = read(fileDescOriginalFile, bufferOriginal, toRead);

        // Read from end of modified file
        revPos -= toRead;
        lseek(fileDescModifiedFile, revPos, SEEK_SET);
        unsigned long long bytesReadModified = read(fileDescModifiedFile, bufferModified, toRead);

        // If read sizes differ, files are not matching
        if (bytesReadOriginal != bytesReadModified) {
            result = false;
            break;
        }

        // Reverse the original chunk
        reverseBlock(bufferOriginal, bytesReadOriginal);

        // Compare chunks
        if (!checkSimilar(bufferModified, bufferOriginal, bytesReadOriginal)) {
            result = false;
            break;
        }
        orgPos += toRead;
        showProgress(orgPos, fileSizeOriginal);
    }
    delete[] bufferModified;
    delete[] bufferOriginal;
    close(fileDescModifiedFile);
    close(fileDescOriginalFile);

    return result;
}

bool verifyFlag2(char * modifiedFilePath, char * originalFilePath, long long startOffset, long long endOffset){
    int fileDescModifiedFile = open(modifiedFilePath, O_RDONLY);
    int fileDescOriginalFile = open(originalFilePath, O_RDONLY);
    
    if (fileDescModifiedFile < 0) { 
        cerr << "Error Reading Modified File: " << strerror(errno) << endl;
        return false;
    }
    if (fileDescOriginalFile < 0) { 
        cerr << "Error Reading Original File: " << strerror(errno) << endl;
        return false;
    }

    unsigned long long fileSizeModified = lseek(fileDescModifiedFile, 0, SEEK_END);
    unsigned long long fileSizeOriginal = lseek(fileDescOriginalFile, 0, SEEK_END);

    lseek(fileDescModifiedFile, 0, SEEK_SET);
    lseek(fileDescOriginalFile, 0, SEEK_SET);

    if (fileSizeModified != fileSizeOriginal) {
        cout << "File Size are same : NO" << endl;
        close(fileDescModifiedFile);
        close(fileDescOriginalFile);
        return false;
    } else {
        cout << "File Size are same : YES" << endl;
    }

    bool isValid = verifyRegion(fileDescOriginalFile, fileDescModifiedFile,0, startOffset, true);

    if(isValid){
        isValid = verifyRegion(fileDescOriginalFile, fileDescModifiedFile,startOffset, endOffset - startOffset + 1 , false);
    }

    if(isValid){
        isValid = verifyRegion(fileDescOriginalFile, fileDescModifiedFile,endOffset+1, fileSizeOriginal, fileSizeModified-endOffset-1, true);
    }

    close(fileDescModifiedFile);
    close(fileDescOriginalFile);

    return isValid;
}


bool verifyRegion(long long fdOrg, long long fdMod, unsigned long long offset,unsigned long long fileSize, unsigned long long length, bool checkReversed){
    char * bufferOrg = new char[bufferSize];
    char * bufferMod = new char[bufferSize];

    unsigned long long remaining = length;
    unsigned long long orgPos = offset;
    unsigned long long modPos = offset;

    while(remaining > 0){
        unsigned long long toRead = min(bufferSize, remaining);
        
        //read block from the original file
        lseek(fdOrg, orgPos, SEEK_SET);
        unsigned long long bytesRead = read(fdOrg, bufferOrg, toRead);
        if(bytesRead != toRead) return false;

        // read block from the modified file
        lseek(fdMod, modPos, SEEK_SET);
        bytesRead = read(fdOrg, bufferOrg, toRead);
        if(bytesRead != toRead) return false;

        if(checkReversed) reverseBlock(bufferOrg, toRead);
        if(checkSimilar(bufferMod, bufferOrg, toRead) != 0)return false;

        orgPos += toRead;
        modPos += toRead;
        showProgress(orgPos, fileSize);
        remaining-=toRead;
    }

    return true;

}

void checkPermissions(char * filePath){
    struct stat fileStat;
    long long statCall = stat(filePath, &fileStat);
    
    if(statCall < 0){
        cout << strerror(errno) << endl;
        return;
    }

    // User Permission Check
    std::cout << "User has read permissions on " << filePath << ": "
              << ((fileStat.st_mode & S_IRUSR) ? "Yes" : "No") << "\n";
    std::cout << "User has write permission on " << filePath << ": "
              << ((fileStat.st_mode & S_IWUSR) ? "Yes" : "No") << "\n";
    std::cout << "User has execute permission on " << filePath << ": "
              << ((fileStat.st_mode & S_IXUSR) ? "Yes" : "No") << "\n";

    // Group Permission Check
    std::cout << "Group has read permissions on " << filePath << ": "
              << ((fileStat.st_mode & S_IRGRP) ? "Yes" : "No") << "\n";
    std::cout << "Group has write permission on " << filePath << ": "
              << ((fileStat.st_mode & S_IWGRP) ? "Yes" : "No") << "\n";
    std::cout << "Group has execute permission on " << filePath << ": "
              << ((fileStat.st_mode & S_IXGRP) ? "Yes" : "No") << "\n";

    // Permission Check on Others
    std::cout << "Others has read permissions on " << filePath << ": "
              << ((fileStat.st_mode & S_IROTH) ? "Yes" : "No") << "\n";
    std::cout << "Others has write permission on " << filePath << ": "
              << ((fileStat.st_mode & S_IWOTH) ? "Yes" : "No") << "\n";
    std::cout << "Others has execute permission on " << filePath << ": "
              << ((fileStat.st_mode & S_IXOTH) ? "Yes" : "No") << "\n";


}