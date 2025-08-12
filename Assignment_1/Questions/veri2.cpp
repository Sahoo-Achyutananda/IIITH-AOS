// flag2_verify.cpp
// Usage: ./a.out <newfile> <oldfile> <directory> 2 <start> <end>
// Verifies that:
//  - new[0..start-1] == reverse(old[0..start-1])
//  - new[start..end] == old[start..end]
//  - new[end+1..EOF] == reverse(old[end+1..EOF])
// Uses only long long / unsigned long long types.

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstring>
#include <cerrno>

using namespace std;
// const unsigned long long BUFFER_SIZE = 4096ULL;

#define BUFFER_SIZE 4096

// Reads exactly `count` bytes (unless EOF) into buf from fd
// Returns total bytes read, or -1 on error
long long read_full(long long fd, char *buf, unsigned long long count) {
    unsigned long long total = 0;
    while (total < count) {
        long long r = read(fd, buf + total, count - total);
        if (r < 0) return -1;  // error while reading
        if (r == 0) break;     // reached EOF
        total += r;
    }
    return total;
}

// Compare two buffers byte-by-byte
bool buffersEqual(const char *a, const char *b, unsigned long long len) {
    for (unsigned long long i = 0; i < len; i++)
        if (a[i] != b[i]) return false;
    return true;
}

// Reverse a buffer in place
void reverseBuffer(char *buf, unsigned long long len) {
    for (unsigned long long i = 0; i < len / 2; i++) {
        char tmp = buf[i];
        buf[i] = buf[len - 1 - i];
        buf[len - 1 - i] = tmp;
    }
}

/*
    verifyRegion():
    ----------------
    Compares a specific region of two files: original file (fdOrig) and modified file (fdNew).

    Parameters:
        offset    = starting byte position of the region in both files
        length    = number of bytes to check in this region
        reversed  = whether the modified file's region has been reversed

    If reversed == false:
        We read both files from `offset` forward and compare chunks directly.

    If reversed == true:
        We read the original file forward from `offset`.
        We read the modified file backwards from the end of the region, chunk-by-chunk,
        reverse the chunk in memory, and compare with the original chunk.

    This method ensures that huge files are processed in small chunks (BUFFER_SIZE),
    avoiding excessive memory usage.
*/
bool verifyRegion(long long fdOrig, long long fdNew,
                  unsigned long long offset, unsigned long long length,
                  bool reversed) 
{
    if (length == 0) return true; // No data to check

    char *bufOrig = new char[BUFFER_SIZE];
    char *bufNew  = new char[BUFFER_SIZE];

    unsigned long long origPos = offset;              // Reading position in original file
    unsigned long long newPos  = offset;              // For forward reading in modified file
    unsigned long long regionEnd = offset + length;   // End position of this region (exclusive)

    while (length > 0) {
        // Decide how much to read in this iteration (≤ BUFFER_SIZE)
        unsigned long long chunk = (length > BUFFER_SIZE) ? BUFFER_SIZE : length;

        // Step 1: Read a chunk from the original file (forward)
        if (lseek(fdOrig, origPos, SEEK_SET) < 0) {
            delete[] bufOrig; delete[] bufNew; return false;
        }
        if (read_full(fdOrig, bufOrig, chunk) != (long long)chunk) {
            delete[] bufOrig; delete[] bufNew; return false;
        }

        // Step 2: Read from modified file
        if (!reversed) {
            // Forward reading for non-reversed region
            if (lseek(fdNew, newPos, SEEK_SET) < 0) {
                delete[] bufOrig; delete[] bufNew; return false;
            }
            if (read_full(fdNew, bufNew, chunk) != (long long)chunk) {
                delete[] bufOrig; delete[] bufNew; return false;
            }
            newPos += chunk;
        } else {
            // Backward reading for reversed region:
            // Move backwards chunk-by-chunk from regionEnd
            regionEnd -= chunk;
            if (lseek(fdNew, regionEnd, SEEK_SET) < 0) {
                delete[] bufOrig; delete[] bufNew; return false;
            }
            if (read_full(fdNew, bufNew, chunk) != (long long)chunk) {
                delete[] bufOrig; delete[] bufNew; return false;
            }
            // Reverse the chunk in memory so it matches original's order
            reverseBuffer(bufNew, chunk);
        }

        // Step 3: Compare the two chunks
        if (!buffersEqual(bufOrig, bufNew, chunk)) {
            delete[] bufOrig; delete[] bufNew; return false;
        }

        // Step 4: Advance positions for original and reduce remaining length
        origPos += chunk;
        length -= chunk;
    }

    // Cleanup
    delete[] bufOrig;
    delete[] bufNew;
    return true;
}


int main(int argc, char *argv[]) {
    if (argc != 7) {
        cerr << "Usage: " << argv[0] << " <newfile> <oldfile> <directory> 2 <start> <end>\n";
        return 1;
    }

    const char *newPath = argv[1];
    const char *oldPath = argv[2];
    long long tmpStart = atoll(argv[5]);
    long long tmpEnd   = atoll(argv[6]);
    if (tmpStart < 0 || tmpEnd < 0 || tmpStart > tmpEnd) { cerr << "Invalid indices\n"; return 1; }
    unsigned long long startIdx = (unsigned long long) tmpStart;
    unsigned long long endIdx   = (unsigned long long) tmpEnd;

    long long fdNew = open(newPath, O_RDONLY);
    long long fdOld = open(oldPath, O_RDONLY);
    if (fdNew < 0 || fdOld < 0) { cerr << "open: " << strerror(errno) << "\n"; if (fdNew>=0) close(fdNew); if (fdOld>=0) close(fdOld); return 1; }

    long long t1 = lseek(fdNew, 0, SEEK_END);
    long long t2 = lseek(fdOld, 0, SEEK_END);
    if (t1 < 0 || t2 < 0) { cerr << "lseek: " << strerror(errno) << "\n"; close(fdNew); close(fdOld); return 1; }
    unsigned long long sizeNew = (unsigned long long) t1;
    unsigned long long sizeOld = (unsigned long long) t2;
    if (sizeNew != sizeOld) { cerr << "File sizes differ\n"; close(fdNew); close(fdOld); return 1; }
    if (endIdx >= sizeOld) { cerr << "end index >= file size\n"; close(fdNew); close(fdOld); return 1; }

    bool ok = true;
    ok &= verifyRegion(fdOld, fdNew, 0ULL, startIdx, true);                                         // region1 reversed
    ok &= verifyRegion(fdOld, fdNew, startIdx, endIdx - startIdx + 1ULL, false);                     // region2 unchanged
    ok &= verifyRegion(fdOld, fdNew, endIdx + 1ULL, sizeOld - (endIdx + 1ULL), true);               // region3 reversed

    cout << "Whether file contents are correctly processed: " << (ok ? "YES" : "NO") << "\n";

    close(fdNew);
    close(fdOld);
    return ok ? 0 : 2;
}
