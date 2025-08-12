#include<iostream>
#include<unistd.h>
#include<cstring>
#include<sys/stat.h>
#include<fcntl.h>
#include<errno.h>
#include<iomanip> // for some terminal display manipulation 
#include<chrono>

using namespace std;
using namespace std::chrono;

// colors and font styles
const char* colorRed = "\033[91m"; // bright red
const char* colorGreen = "\033[92m"; // bright green 
const char* colorYellow = "\033[93m"; // bringht yellow 
const char* colorBlue = "\033[94m"; // bright blue 
const char* foregroundBlue = "\033[104m"; 
const string foregroundGreen = "\033[102m";
const string foregroundRed = "\033[101m";
const char* reset = "\033[0m";
const char* fontBold = "\033[1m";
const char* fontItalic = "\033[3m";

const unsigned long long bufferSize = 4096; 
void showTaskDescription(int type, char* fileName);
void showDetails(string fileName, string outputPath, unsigned long long fileSize);

bool checkSimilar(char * block1, char* block2, long long blockSize);
void reverseBlock(char * block, unsigned long long blockSize);
bool verifyFlag0(char * modifiedFilePath, char * originalFilePath, unsigned long long blockSize);
bool verifyFlag1(char * modifiedFilePath, char * originalFilePath);
bool verifyFlag2(char * modifiedFilePath, char * originalFilePath, long long startOffset, long long endOffset);
bool verifyRegion(long long fdOrg, long long fdMod, unsigned long long offset, unsigned long long length, bool checkReversed, unsigned long long fileSize, unsigned long long &bytesProcessed);
void checkPermissions(char * filePath);

int main(int argc, char * argv[]){
    // format : ./a.out <newfilepath> <oldfilepath> <directory> <flag> [<blockSize>|<start> <end>]

    auto start = high_resolution_clock::now();
    char *modifiedFilePath = argv[1];
    char *originalFilePath = argv[2];
    char *directory = argv[3];
    int flag = atoi(argv[4]);

    long long blockSize = -1;
    long long startOffset = -1;
    long long endOffset = -1;
    if(flag == 0){
        blockSize = atoll(argv[5]);
    }else if(flag == 2){
        startOffset = atoll(argv[5]);
        endOffset = atoll(argv[6]);
    }

    switch(flag){
        case 0:

            if(verifyFlag0(modifiedFilePath, originalFilePath, blockSize)){
                cout << fontBold << colorBlue << "\nWhether file contents are correctly processed : " << colorGreen << "YES" << reset << endl;
            }else{
                cout << fontBold << colorBlue << "\nWhether file contents are correctly processed : " << colorRed << "NO" << reset << endl;
            }

            break;
        case 1:
            if(verifyFlag1(modifiedFilePath, originalFilePath)){
                cout << fontBold << colorBlue << "\nWhether file contents are correctly processed : " << colorGreen << "YES" << reset << endl;
            }else{
                cout << fontBold << colorBlue << "\nWhether file contents are correctly processed : " << colorRed << "NO" << reset << endl;
            }
            break;
        case 2:
            if (verifyFlag2(modifiedFilePath, originalFilePath, startOffset, endOffset)){
                cout << fontBold << colorBlue << "\nWhether file contents are correctly processed : " << colorGreen << "YES" << reset << endl;
            }else{
                cout << fontBold << colorBlue << "\nWhether file contents are correctly processed : " << colorRed << "NO" << reset << endl;
            };
            break;
        default:
            cout << "\nInvalid Flag" << endl;
            break;
    }

    cout << endl;
    cout << fontBold << foregroundGreen << "Permissions on Reversed File : " << modifiedFilePath << reset << endl;
    checkPermissions(modifiedFilePath);
    cout << fontBold << foregroundGreen << "Permissions on Original File : " << originalFilePath << reset << endl;
    checkPermissions(originalFilePath);
    cout << fontBold << foregroundGreen << "Permissions on Directory : " << directory << reset << endl;
    checkPermissions(directory);

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(stop - start);

    cout << fontBold << colorGreen << fontItalic << "Program Terminated in " << duration.count() << "sec" << reset << "\n";
}

void showDetails(string fileName, string outputPath, unsigned long long fileSize){
    cout << "Source File Name : " << fontBold << colorBlue << fileName << reset << endl;
    cout << "Destination File Path : " << fontBold << colorBlue << outputPath << reset <<  endl;
    cout << "File Size : " << fontBold << colorBlue << fileSize << reset << endl;
}

void showTaskDescription(int type, char* fileName){
    cout << fontBold << foregroundGreen << "\n Task " << reset;
    switch(type){
        case 0 :
            cout << foregroundBlue << fontBold << " Verify Block Wise Reversal on file - " << fileName << reset << endl;
            break;
        case 1 :
            cout << foregroundBlue << fontBold << " Verify Full Reversal of file - " << fileName << reset << endl ;
            break;
        case 2 :
            cout << foregroundBlue << fontBold << " Verify Partial Range Reversal on file - " << fileName << reset << endl;
            break;
    }
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

bool checkSimilar(char* block1, char* block2, long long blockSize){
    long long i = 0;
    while(i < blockSize){
        if(block1[i] != block2[i]){
            cout << block1[i] << block2[i] << i << endl;
            return false;
        }
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
    lseek(fileDescOriginalFile, 0, SEEK_SET);

    showTaskDescription(0,originalFilePath);
    showDetails(originalFilePath, modifiedFilePath, fileSize);
    cout << "Block Size : " << fontBold << colorBlue << blockSize << reset << endl;
    cout << fontBold << colorBlue <<  "Processing ... " << endl;

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
    showTaskDescription(1,originalFilePath);
    showDetails(originalFilePath, modifiedFilePath, fileSizeOriginal);
    cout << fontBold << colorBlue <<  "Processing ... " << endl;

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

bool verifyRegion(long long fdOrg, long long fdMod,
                  unsigned long long offset, unsigned long long length,
                  bool checkReversed, unsigned long long fileSize, unsigned long long &bytesProcessed) {

    if (length == 0) return true; // nothing to compare

    char *bufferOrg = new char[bufferSize];
    char *bufferMod = new char[bufferSize];

    unsigned long long remaining = length;
    unsigned long long orgPos = offset;

    // yeh dono are the pointers to keep a track of the indices on the modified<reversed> file 
    unsigned long long modPosForward = offset;
    unsigned long long modPosBack = offset + length; // exclusive end

    while (remaining > 0) {
        unsigned long long toRead = (remaining > bufferSize) ? bufferSize : remaining;

        // Read from original forward
        if (lseek(fdOrg, orgPos, SEEK_SET) < 0) { delete[] bufferOrg; delete[] bufferMod; return false; }
        long long bytesReadOrg = read(fdOrg, bufferOrg, toRead);
        if (bytesReadOrg != (long long) toRead) { delete[] bufferOrg; delete[] bufferMod; return false; }

        if (!checkReversed) {
            // Non-reversed region: read from same forward position
            if (lseek(fdMod, modPosForward, SEEK_SET) < 0) { 
                delete[] bufferOrg; 
                delete[] bufferMod; 
                return false; 
            }
            long long bytesReadMod = read(fdMod, bufferMod, toRead);
            if (bytesReadMod != (long long) toRead) { 
                delete[] bufferOrg; 
                delete[] bufferMod; 
                return false; 
            }

            if (!checkSimilar(bufferOrg, bufferMod, toRead)) { 
                delete[] bufferOrg; 
                delete[] bufferMod; 
                return false; 
            }

            orgPos += toRead;
            modPosForward += toRead;
        } else {
            // Reversed region: read chunk from end backwards
            modPosBack -= toRead; // move pointer back
            if (lseek(fdMod, modPosBack, SEEK_SET) < 0) { 
                delete[] bufferOrg; 
                delete[] bufferMod; 
                return false; 
            }
            long long bytesReadMod = read(fdMod, bufferMod, toRead);
            if (bytesReadMod != (long long) toRead) { 
                delete[] bufferOrg; 
                delete[] bufferMod; 
                return false; 
            }

            reverseBlock(bufferMod, toRead);

            if (!checkSimilar(bufferOrg, bufferMod, toRead)) { delete[] bufferOrg; delete[] bufferMod; return false; }

            orgPos += toRead;
        }

        remaining -= toRead;
        bytesProcessed+= toRead;
        showProgress(bytesProcessed, fileSize);
    }

    delete[] bufferOrg;
    delete[] bufferMod;
    return true;
}

bool verifyFlag2(char *modifiedFilePath, char *originalFilePath, long long startOffset, long long endOffset) {
    int fdMod = open(modifiedFilePath, O_RDONLY);
    int fdOrg = open(originalFilePath, O_RDONLY);
    if (fdMod < 0 || fdOrg < 0) {
        cerr << "Error opening files: " << strerror(errno) << endl;
        if (fdMod >= 0) close(fdMod);
        if (fdOrg >= 0) close(fdOrg);
        return false;
    }

    unsigned long long sizeMod = lseek(fdMod, 0, SEEK_END);
    unsigned long long sizeOrg = lseek(fdOrg, 0, SEEK_END);
    unsigned long long bytesProcessed = 0;

    if (sizeMod != sizeOrg) {
        cout << "File sizes differ" << endl;
        close(fdMod);
        close(fdOrg);
        return false;
    }

    showTaskDescription(2,originalFilePath);
    showDetails(originalFilePath, modifiedFilePath, sizeOrg);
    cout << "Start Offset : " << colorBlue << fontBold << startOffset << reset << endl;
    cout << "End Offset : " << colorBlue << fontBold <<  endOffset  << reset << endl;
    cout << fontBold << colorBlue <<  "Processing ... " << endl;

    bool ok = true;
    if (!verifyRegion(fdOrg, fdMod, 0, startOffset, true, sizeMod, bytesProcessed)) ok = false;
    if (ok && !verifyRegion(fdOrg, fdMod, startOffset, endOffset - startOffset + 1, false, sizeMod, bytesProcessed)) ok = false;
    if (ok && !verifyRegion(fdOrg, fdMod, endOffset + 1, sizeOrg - endOffset - 1, true, sizeMod, bytesProcessed)) ok = false;

    close(fdMod);
    close(fdOrg);
    return ok;
}

void checkPermissions(char * filePath){
    struct stat fileStat;
    long long statCall = stat(filePath, &fileStat);
    
    if(statCall < 0){
        cout << strerror(errno) << endl;
        return;
    }

    // User Permission Check
    cout << fontBold << fontItalic << colorBlue << "User Permissions" << reset << endl;
    cout << setw(40) << "User has read permissions on " << filePath << ": "
              << fontBold << ((fileStat.st_mode & S_IRUSR) ? (foregroundGreen  + "Yes"): (foregroundRed +  "No "))<< reset << "\n";
    cout << setw(40) << "User has write permission on " << filePath << ": "
              << fontBold << ((fileStat.st_mode & S_IWUSR) ? (foregroundGreen  + "Yes"): (foregroundRed +  "No "))<< reset << "\n";
    cout << setw(40) << "User has execute permission on " << filePath << ": "
              << fontBold << ((fileStat.st_mode & S_IXUSR) ? (foregroundGreen  + "Yes"): (foregroundRed +  "No "))<< reset << "\n";

    // Group Permission Check
    cout << fontBold << fontItalic << colorBlue << "Group Permissions" << reset << endl;
    cout << setw(40) << "Group has read permissions on " << filePath << ": "
              << fontBold << ((fileStat.st_mode & S_IRGRP) ? (foregroundGreen  + "Yes"): (foregroundRed +  "No "))<< reset << "\n";
    cout << setw(40) << "Group has write permission on " << filePath << ": "
              << fontBold << ((fileStat.st_mode & S_IWGRP) ? (foregroundGreen  + "Yes"): (foregroundRed +  "No "))<< reset << "\n";
    cout << setw(40) << "Group has execute permission on " << filePath << ": "
              << fontBold << ((fileStat.st_mode & S_IXGRP) ? (foregroundGreen  + "Yes"): (foregroundRed +  "No "))<< reset << "\n";

    // Permission Check on Others
    cout << fontBold << fontItalic << colorBlue << "Others Permissions" << reset << endl;
    cout << setw(40) << "Others have read permissions on " << filePath << ": "
              << fontBold << ((fileStat.st_mode & S_IROTH) ? (foregroundGreen  + "Yes"): (foregroundRed +  "No "))<< reset << "\n";
    cout << setw(40) << "Others have write permission on " << filePath << ": "
              << fontBold << ((fileStat.st_mode & S_IWOTH) ? (foregroundGreen  + "Yes"): (foregroundRed +  "No "))<< reset << "\n";
    cout << setw(40) << "Others have execute permission on " << filePath << ": "
              << fontBold << ((fileStat.st_mode & S_IXOTH) ? (foregroundGreen  + "Yes"): (foregroundRed +  "No "))<< reset << "\n";

}