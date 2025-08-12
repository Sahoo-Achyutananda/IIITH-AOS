// flag2_reversal.cpp
// Usage: ./a.out <input-file> 2 <start-index> <end-index>
// Reverses [0..start-1] and [end+1..EOF], copies [start..end] as-is.
// Uses only long long / unsigned long long for sizes and positions.

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstring>
#include <cerrno>

using namespace std;

const unsigned long long BUFFER_SIZE = 4096ULL; // 4 KiB buffer

// Read exactly 'count' bytes (unless EOF). Returns bytes actually read, or -1 on error.
long long read_full(long long fd, char *buf, unsigned long long count) {
    unsigned long long total = 0;
    while (total < count) {
        long long r = read(fd, buf + (size_t)total, (size_t)(count - total));
        if (r < 0) return -1;            // error
        if (r == 0) break;               // EOF
        total += (unsigned long long)r;
    }
    return (long long) total;
}

// Write exactly 'count' bytes. Returns bytes written or -1 on error.
long long write_full(long long fd, const char *buf, unsigned long long count) {
    unsigned long long total = 0;
    while (total < count) {
        long long w = write(fd, buf + (size_t)total, (size_t)(count - total));
        if (w < 0) return -1;            // error
        total += (unsigned long long)w;
    }
    return (long long) total;
}

void reverseBuffer(char *buf, unsigned long long len) {
    for (unsigned long long i = 0; i + 1 < len - i; ++i) {
        char tmp = buf[i];
        buf[i] = buf[len - 1 - i];
        buf[len - 1 - i] = tmp;
    }
}

void showProgress(unsigned long long processed, unsigned long long total) {
    if (total == 0) return;
    float progress = (float) processed / (float) total;
    int width = 40;
    int pos = (int)(progress * width);
    cout << "\r[";
    for (int i = 0; i < width; ++i) cout << (i < pos ? '#' : '-');
    cout << "] " << (int)(progress * 100.0f) << "% (" << processed << "/" << total << ")";
    cout.flush();
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        cerr << "Usage: " << argv[0] << " <input-file> 2 <start> <end>\n";
        return 1;
    }

    const char *inPath = argv[1];
    long long tmpStart = atoll(argv[3]);
    long long tmpEnd   = atoll(argv[4]);

    if (tmpStart < 0 || tmpEnd < 0 || tmpStart > tmpEnd) {
        cerr << "Invalid start/end indices\n";
        return 1;
    }

    unsigned long long startIdx = (unsigned long long) tmpStart;
    unsigned long long endIdx   = (unsigned long long) tmpEnd;

    // open input
    long long fdIn = open(inPath, O_RDONLY);
    if (fdIn < 0) { cerr << "open input: " << strerror(errno) << "\n"; return 1; }

    long long tsize = lseek(fdIn, 0, SEEK_END);
    if (tsize < 0) { cerr << "lseek: " << strerror(errno) << "\n"; close(fdIn); return 1; }
    unsigned long long fileSize = (unsigned long long) tsize;
    if (startIdx > fileSize) { cerr << "start index > file size\n"; close(fdIn); return 1; }
    if (endIdx >= fileSize) { cerr << "end index >= file size\n"; close(fdIn); return 1; }

    // create directory "Assignment1" with permissions 700
    mkdir("Assignment1", S_IRWXU);

    // prepare output path: Assignment1/2<basename>
    const char *base = strrchr(inPath, '/');
    string name = base ? (base + 1) : inPath;
    string outPath = string("Assignment1/2") + name;

    // open output with permission 600
    long long fdOut = open(outPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fdOut < 0) { cerr << "open output: " << strerror(errno) << "\n"; close(fdIn); return 1; }

    char *buffer = new char[BUFFER_SIZE];
    unsigned long long bytesProcessed = 0ULL;

    // -------- Region 1: reverse [0 .. startIdx-1] --------
    unsigned long long region1Len = startIdx;
    unsigned long long pos = region1Len;
    while (pos > 0) {
        unsigned long long toRead = (pos >= BUFFER_SIZE) ? BUFFER_SIZE : pos;
        pos -= toRead; // new read position in input for this chunk
        if (lseek(fdIn, (off_t)pos, SEEK_SET) < 0) { cerr << "lseek region1: " << strerror(errno) << "\n"; delete[] buffer; close(fdIn); close(fdOut); return 1; }

        long long r = read_full(fdIn, buffer, toRead);
        if (r != (long long)toRead) { cerr << "short read region1\n"; delete[] buffer; close(fdIn); close(fdOut); return 1; }

        reverseBuffer(buffer, toRead);

        long long w = write_full(fdOut, buffer, toRead);
        if (w != (long long)toRead) { cerr << "write error region1\n"; delete[] buffer; close(fdIn); close(fdOut); return 1; }

        bytesProcessed += toRead;
        showProgress(bytesProcessed, fileSize);
    }

    // -------- Region 2: copy [startIdx .. endIdx] as-is --------
    unsigned long long region2Len = endIdx - startIdx + 1ULL;
    unsigned long long origPos = startIdx;
    while (region2Len > 0) {
        unsigned long long toRead = (region2Len >= BUFFER_SIZE) ? BUFFER_SIZE : region2Len;
        if (lseek(fdIn, (off_t)origPos, SEEK_SET) < 0) { cerr << "lseek region2: " << strerror(errno) << "\n"; delete[] buffer; close(fdIn); close(fdOut); return 1; }

        long long r = read_full(fdIn, buffer, toRead);
        if (r != (long long) toRead) { cerr << "short read region2\n"; delete[] buffer; close(fdIn); close(fdOut); return 1; }

        long long w = write_full(fdOut, buffer, toRead);
        if (w != (long long) toRead) { cerr << "write error region2\n"; delete[] buffer; close(fdIn); close(fdOut); return 1; }

        origPos += toRead;
        region2Len -= toRead;
        bytesProcessed += toRead;
        showProgress(bytesProcessed, fileSize);
    }

    // -------- Region 3: reverse [endIdx+1 .. EOF] --------
    unsigned long long region3Start = endIdx + 1ULL;
    unsigned long long region3Len = (fileSize > region3Start) ? (fileSize - region3Start) : 0ULL;
    pos = fileSize;
    while (region3Len > 0) {
        unsigned long long toRead = (region3Len >= BUFFER_SIZE) ? BUFFER_SIZE : region3Len;
        pos -= toRead;
        if (lseek(fdIn, (off_t)pos, SEEK_SET) < 0) { cerr << "lseek region3: " << strerror(errno) << "\n"; delete[] buffer; close(fdIn); close(fdOut); return 1; }

        long long r = read_full(fdIn, buffer, toRead);
        if (r != (long long) toRead) { cerr << "short read region3\n"; delete[] buffer; close(fdIn); close(fdOut); return 1; }

        reverseBuffer(buffer, toRead);

        long long w = write_full(fdOut, buffer, toRead);
        if (w != (long long) toRead) { cerr << "write error region3\n"; delete[] buffer; close(fdIn); close(fdOut); return 1; }

        region3Len -= toRead;
        bytesProcessed += toRead;
        showProgress(bytesProcessed, fileSize);
    }

    cout << "\nReversal finished. Output: " << outPath << "\n";

    delete[] buffer;
    close(fdIn);
    close(fdOut);
    return 0;
}
