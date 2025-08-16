# Code Structure

## 2025202028_A1_Q1.cpp <Question 1>

The following funnctions were declared and defined. Apart from these, constants for buffer size, colors and font styles were declared.
```bash
void throwError(); # shows the actual command format when  wrong arguments are passed
void showProgress(unsigned long long processed, unsigned long long total); # for showing progress bar
void showSuccessMessage(); # To show a simple message after completing file reversals
void showErrorMessage(const char * message, bool showErno); # To show specific error messages, the second argument is used when errno is supposed to be displayed
void showTaskDescription(int type, char* fileName); # Shows the task desription based on the command passed
void showDetails(string fileName, string outputPath, unsigned long long fileSize); # shows necessary details about the task being performed
void reverseBuffer(char *buffer, unsigned long long size); # helper function 
void reverseBlocks(char* name, unsigned long long blockSize); # implementation of Flag 0
void reverseComplete(char* fileName); # implementation of Flag 1
void reverseRange(char* name, unsigned long long startIndex, unsigned long long endIndex); # implementation of Flag 2
```

## 2025202028_A1_Q2.cpp <Question 2>

The following funnctions were declared and defined. Apart from these, constants for buffer size, colors and font styles were declared.
```bash
void showTaskDescription(int type, char* fileName); # same as question 1
void showDetails(string fileName, string outputPath, unsigned long long fileSize); # same as question 1
bool checkDirectoryExists(const char * directoryName); # checks if a directory exists
bool checkSimilar(char * block1, char* block2, long long blockSize); # checks if two buffers are similar or not
void reverseBlock(char * block, unsigned long long blockSize); # reverses a given block
bool checkFileSize(char * modifiedFilePath, char * originalFilePath); # checks whether two files are of same size
bool verifyFlag0(char * modifiedFilePath, char * originalFilePath, unsigned long long blockSize); # implementation of flag 0 verifier 
bool verifyFlag1(char * modifiedFilePath, char * originalFilePath); # implementation of flag 1 verifier
bool verifyFlag2(char * modifiedFilePath, char * originalFilePath, long long startOffset, long long endOffset); # implementation of flag 2 verifier
bool verifyRegion(long long fdOrg, long long fdMod, unsigned long long offset, unsigned long long length, bool checkReversed, unsigned long long fileSize, unsigned long long &bytesProcessed); # a helper function for verifyFlag2()
void checkPermissions(char * filePath); # checks and prints permission
void showErrorMessage(const char * message, bool showErno); # same as question 1
void throwError(); # same as question 1
```


**NOTE : A Buffer Size of 4096 (4KB) is being used by default to handle large files. Detailed working of each program along with examples are given below**

# Question 1 :  File Reversal Utility

## Overview
This program reads a file and writes a modified version to the `Assignment1` directory based on the specified **flag**:

- **Flag 0:** Block-wise reversal  
- **Flag 1:** Full file reversal  
- **Flag 2:** Partial range reversal  

It is implemented using **low-level system calls** (`open`, `read`, `write`, `lseek`, `close`) to handle large files efficiently.

---

## Usage

```bash
./a.out <input_file_name> <flag> [flag_arguments]
```

### Examples
```bash
# Flag 0: Block-wise reversal with block size = 4
./a.out input.txt 0 4

# Flag 1: Full file reversal
./a.out input.txt 1

# Flag 2: Partial range reversal (start=5, end=10)
./a.out input.txt 2 5 10
```

---

## Flag Logics

### **Flag 0 – Block-wise Reversal**
**Logic:**
1. Read the file in chunks of the given `block size`.
2. Reverse the bytes **within** each block.
3. Write the reversed block to the output file.
4. Block order in the file remains unchanged.

**Example:**
```
Input:  ABCDEFGH
Block size: 4
Blocks:  [ABCD] [EFGH]
Reverse each block: [DCBA] [HGFE]
Output: DCBAHGFE
```

Special cases handled:
- **Block size = 1:** File is copied as-is (no change in order).
- **Block size >= file size:** Entire file is reversed (same as Flag 1).

---

### **Flag 1 – Full File Reversal**
**Logic:**
1. **Open** the input file for reading and create the destination file for writing.
2. **Find the file size** using `lseek(fd, 0, SEEK_END)`.  
3. Initialize:
   - `remaining = fileSize` (bytes left to process from the end)
   - fixed `bufferSize` (e.g., 4096) – only a small chunk is held in memory
4. **Loop while `remaining > 0`:**
   - `toRead = min(bufferSize, remaining)`
   - `offset = remaining - toRead` (this jumps backward)
   - `lseek` to `offset` (beginning of the next tail chunk)
   - `read` `toRead` bytes into `buffer`
   - **reverse the buffer in place**
   - `write` the reversed buffer to the output
   - `remaining -= toRead`
5. When the loop ends, the output file is the input file **fully reversed**.

### Example Working :

**Input:** `ABCDEFGH` (8 bytes)  
**bufferSize:** `3`  
Positions: `A(0) B(1) C(2) D(3) E(4) F(5) G(6) H(7)`

Initial: `remaining = 8`, `output = ""`

**Iteration 1**
- `toRead = min(3, 8) = 3`
- `offset = 8 - 3 = 5`
- read `5..7` → `"FGH"`
- reverse → `"HGF"`
- write → output: `"HGF"`
- `remaining = 5`

**Iteration 2**
- `toRead = min(3, 5) = 3`
- `offset = 5 - 3 = 2`
- read `2..4` → `"CDE"`
- reverse → `"EDC"`
- write → output: `"HGFEDC"`
- `remaining = 2`

**Iteration 3**
- `toRead = min(3, 2) = 2`
- `offset = 2 - 2 = 0`
- read `0..1` → `"AB"`
- reverse → `"BA"`
- write → output: `"HGFEDCBA"`
- `remaining = 0` → **done**

**Final Output:** `HGFEDCBA`

Memory stays small (only one buffer), but the whole file is reversed correctly.

### Generalized Working - 

Let the input be split into chunks from the end:  
`[ ... | C2 | C1 ]` where `C1` is the last chunk, `C2` is the second last, etc.  
The algorithm writes chunks in the order `C1, C2, ...` and **reverses each chunk**.  
Thus the final output is:  
`reverse(C1) + reverse(C2) + ...` which equals `reverse(Cn ... C2 C1)` — i.e., the entire file reversed.

---

### **Flag 2 – Partial Range Reversal**
**Logic:**
Given two indices: `start` and `end` (0-based, inclusive).
The file is divided into 3 regions:
1. **Region 1:** From the start of the file to `start - 1` → **reversed**.
2. **Region 2:** From `start` to `end` → **unchanged**.
3. **Region 3:** From `end + 1` to EOF → **reversed**.
The program uses 3 while loopseach handling each part of the file to be processed.
**Example:**
```
Input:   ABCDEFGHI
start=2, end=6
Regions: [AB] [CDEFG] [HI]
Reverse: [BA] [CDEFG] [IH]
Output:  BACDEFGIH
```

**Steps:**
1. Reverse Region 1 by reading it backwards and writing in normal order.
2. Copy Region 2 directly without changes.
3. Reverse Region 3 by reading it backwards and writing in normal order.

---

## Output
- All processed files are stored in the `Assignment1` directory.
- Filenames are prefixed with `<flag>_` to indicate the mode used.  
  Example: `Assignment1/0_input.txt` for Flag 0.

---

## Notes
- Indices in Flag 2 are **inclusive**.
- Works with files larger than available RAM by processing in small chunks.
- No data is lost — the reversal operations only rearrange bytes.

---

# Question 2 : File Reversal Verification Utility

## Overview

This program verifies whether a processed file (reversed in some manner) has been generated correctly from the original file.
It supports three verification modes corresponding to the reversal modes from Question 1:

- **Flag 0:** Block-wise reversal verification

- **Flag 1:** Full file reversal verification

- **Flag 2:** Partial range reversal verification

Additionally, it checks and displays file and directory permissions for:
- The modified (processed) file
- The original file
- The directory containing them (Assignment1)

It uses low-level file handling (`open`, `read`, `lseek`, `close`) to work efficiently with large files without loading them entirely into memory.
Verification is done chunk-by-chunk to minimize memory usage.

---

## Usage

```bash
./a.out <modified_file_path> <original_file_path> <directory_path> <flag> [<blockSize>|<start> <end>]
```

### Examples

```bash
# Flag 0: Verify block-wise reversal (block size = 4)
./a.out Assignment1/0_filename.txt filename.txt Assignment1 0 4

# Flag 1: Verify full file reversal
./a.out Assignment1/1_filename.txt filename.txt Assignment1 1

# Flag 2: Verify partial range reversal (start=5, end=10)
./a.out Assignment1/2_filename.txt filename.txt Assignment1 2 5 10
```

---

## Flag Logics

### **Flag 0 - Block-Wise Reversal Check**
**Logic:**
1. Read both files block-by-block with the given blockSize.
2. Reverse each block from the modified file.
3. Compare the reversed block with the corresponding block in the original file.
4. If all blocks match, verification passes.

**Example:**
```
Original file content: "ABCDEFGH"
Block size: 4
Modified file: "DCBAHGFE"
Block 1: "DCBA" reversed → "ABCD" matches original block 1.
Block 2: "HGFE" reversed → "EFGH" matches original block 2.
All blocks match → verification passes.
```

### **Flag 1 – Full File Reversal Check**
**Logic:**
1. Compare the original file from the start with the modified file from the end.
2. Process in chunks of fixed size (bufferSize = 4096 by default).
3. For each chunk:
    - Reverse the chunk from the original file.
    - Compare it to the corresponding chunk from the modified file.
    - If all chunks match, the reversal is correct.

**Example:**
```
Original file: "ABCDE"
Modified file: "EDCBA"
Process:
    - Compare "ED" (from modified end) with reversed "DE" from original start → match.
    - Compare "CB" (from modified middle) with reversed "BC" from original next segment → match.
    - Compare "A" with "A" → match.
```

### **Flag 2 – Partial Range Reversal Check**
**Logic:**
Given start and end positions (0-based, inclusive):
- **Region 1:** Start of file to start - 1 → should be reversed.
- **Region 2:** start to end → should match exactly.
- **Region 3:** end + 1 to end of file → should be reversed.
The program verifies each region separately using the same chunked comparison approach.

**Example**:
```
Original file: "ABCDEFGHI"
start = 2, end = 6
Modified file: "BADCFEGHI"
    - Region 1 ("AB") reversed to "BA" → matches modified file start.
    - Region 2 ("CDEFG") unchanged → matches exactly.
    - Region 3 ("HI") reversed to "IH" → matches modified file end.
```

# Constraints and Limitations

- The input file has to be present in the same directory as the program file.
- The directory (Assignment1) is created on the same directory where the program and input file exists.
- For flag 0 (block wise file reversal - question 1), reversal of files on the basis of small block sizes (eg : 2, 3 etc ..) is slow.

# Scope for Improvements 

- More modularity could've been achieved. The current program has various redundant/repeating sections.
- The program could be more verbose, providing clearer messages during execution.