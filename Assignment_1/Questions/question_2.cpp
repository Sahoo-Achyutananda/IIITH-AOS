#include<iostream>
#include<unistd.h>
#include<cstring>
#include<sys/stat.h>
#include<fcntl.h>
#include<errno.h>

using namespace std;

bool checkSimilar(char * block1, char* block2, long long blockSize);
void reverseBlock(char * block, unsigned long long blockSize);
bool verifyFlag0(char * modifiedFilePath, char * originalFilePath, unsigned long long blockSize);
void verifyFlag1(char * modifiedFilePath, char * originalFilePath);
void verifyFlag2(char * modifiedFilePath, char * originalFilePath, long long startOffset, long long endOffset);

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
            verifyFlag0(modifiedFilePath, originalFilePath, blockSize);
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
    return false;
}

void reverseBlock(char * block, unsigned long long blockSize){
    for (long long i = 0; i < blockSize / 2; i++) {
        swap(block[i], block[blockSize - i - 1]);
    }
}

bool verifyFlag0(char * modifiedFilePath, char * originalFilePath, unsigned long long blockSize){
    // 1. get files paths
    // 2. open them using open() sys call
    unsigned long long fileDescModifiedFile = open(modifiedFilePath, O_RDONLY);
    unsigned long long fileDescOriginalFile = open(originalFilePath, O_RDONLY);
    
    if(fileDescModifiedFile < 0){ 
        cerr << "Error Reading Modofied File " << strerror(errno) << endl;
        return;
    }
    if(fileDescOriginalFile < 0){ 
        cerr << "Error Reading Original File " << strerror(errno) << endl;
        return;
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


