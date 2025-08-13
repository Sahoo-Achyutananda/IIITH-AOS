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

### Example 1: Small file with small buffer

**Input:** `ABCDE` (5 bytes)  
**bufferSize:** `2`

We number positions for clarity:  
```
A(0) B(1) C(2) D(3) E(4)
```

Initial:
```
fileSize = 5
remaining = 5
output = "" (empty)
```

**Iteration 1**
- `toRead = min(2, 5) = 2`
- `offset = 5 - 2 = 3`
- `lseek(fd, 3)` → read bytes at positions 3..4 → `"DE"`
- reverse buffer: `"ED"`
- write → `output = "ED"`
- `remaining = 5 - 2 = 3`

**Iteration 2**
- `toRead = min(2, 3) = 2`
- `offset = 3 - 2 = 1`
- `lseek(fd, 1)` → read bytes 1..2 → `"BC"`
- reverse buffer: `"CB"`
- write → `output = "EDCB"`
- `remaining = 3 - 2 = 1`

**Iteration 3**
- `toRead = min(2, 1) = 1`
- `offset = 1 - 1 = 0`
- `lseek(fd, 0)` → read byte 0 → `"A"`
- reverse buffer: `"A"` (unchanged)
- write → `output = "EDCBA"`
- `remaining = 0` → **done**

**Final Output:** `EDCBA`

### Example 2: Length not a multiple of buffer size

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

### Example 3: Large file (conceptual)

**Input size:** 1 GB+  
**bufferSize:** 4096 bytes (4 KB)

The loop will:
- Jump back 4 KB at a time (`offset = remaining - 4096`),
- Read 4 KB, reverse those 4 KB, write them,
- Repeat until the beginning is reached.

Memory stays small (only one buffer), but the whole file is reversed correctly.

### Correctness intuition (why this always works)

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