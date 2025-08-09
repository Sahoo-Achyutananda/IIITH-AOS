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
void verifyFlag2(char * modifiedFilePath, char * originalFilePath, long long startOffset, long long endOffset){
    cout << "HI" << endl;
};

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
            verifyFlag1(modifiedFilePath, originalFilePath);
            break;
        case 2:
            verifyFlag2(modifiedFilePath, originalFilePath, startOffset, endOffset);
            break;
        default:
            cout << "Invalid Flag" << endl;
            break;
    }
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
    }

    delete[] bufferModified;
    delete[] bufferOriginal;
    close(fileDescModifiedFile);
    close(fileDescOriginalFile);

    return result;
}

// bool verifyFlag1(char * modifiedFilePath, char * originalFilePath){
//     long long fileDescModifiedFile = open(modifiedFilePath, O_RDONLY);
//     long long fileDescOriginalFile = open(originalFilePath, O_RDONLY);
    
//     if(fileDescModifiedFile < 0){ 
//         cerr << "Error Reading Modofied File " << strerror(errno) << endl;
//         return false;
//     }
//     if(fileDescOriginalFile < 0){ 
//         cerr << "Error Reading Original File " << strerror(errno) << endl;
//         return false;
//     }

//     unsigned long long fileSizeModified = lseek(fileDescModifiedFile, 0, SEEK_END);
//     unsigned long long fileSizeOriginal = lseek(fileDescOriginalFile, 0, SEEK_END);
//     lseek(fileDescModifiedFile, 0, SEEK_SET);
//     lseek(fileDescModifiedFile, 0, SEEK_SET);
//     if(fileSizeModified != fileSizeOriginal){
//         cout << "File Size are same : NO" << endl;
//         close(fileDescModifiedFile);
//         close(fileDescOriginalFile);
//         return false;
//     }else{
//         cout << "File Size are same : YES" << endl;
//     }

//     char * bufferModified = new char[bufferSize];
//     char * bufferOriginal = new char[bufferSize];
//     unsigned long long orgPos = 0;
//     unsigned long long revPos = fileSizeModified;
//     bool result = true;
//     while(orgPos < fileDescOriginalFile){
        
//         unsigned long long toRead = min(bufferSize, fileSizeOriginal-orgPos);
//         lseek(fileDescOriginalFile, orgPos, SEEK_SET);
//         unsigned long long bytesRead = read(fileDescOriginalFile, bufferOriginal, toRead);

//         revPos-=toRead;
//         lseek(fileDescModifiedFile, revPos, SEEK_SET);
//         bytesRead = read(fileDescModifiedFile, bufferModified, toRead);

//         reverseBlock(bufferOriginal, toRead);
//         if(!checkSimilar(bufferModified, bufferOriginal, toRead)){
//             result = false;
//             break;
//         }
//         orgPos+=toRead;
//     }
//     delete [] bufferModified;
//     delete [] bufferOriginal;
//     close(fileDescModifiedFile);
//     close(fileDescOriginalFile);
//     return result;
// }



