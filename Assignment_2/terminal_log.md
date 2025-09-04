Absolutely! Here's a set of *README.md* entries in Markdown format that document each input/output interaction from your terminal session. I've replaced eswar with user and eswar-Inspiron-3505 with system-name as requested.

---

## 📁 README.md — Terminal Session Documentation

### 📍 Path: /home/user/monsoon 2025/AOS

#### 1. echo "hello" ""world"
bash
Input: echo "hello" ""world"

Output: "hello" ""world"

This command prints the string with quoted segments. The empty quotes are preserved in the output.

#### 2. echo &nbsp;&nbsp;&nbsp;&nbsp;Aos  &nbsp; &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Assignment &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;2
bash
Input: echo Aos &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;        Assignment&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;2

Output: Aos Assignment 2

Multiple spaces are collapsed in the output. Echo prints the arguments separated by a single space. spaces between quotes should be preserved.

#### 3. pwd
bash
Input: pwd
Output: /home/user/monsoon 2025/AOS

Displays the current working directory.

---

### 📍 Path: /home/user/monsoon 2025/AOS/testing

#### 4. ls
bash
Input: ls

Output: lines.txt inside_testing input.txt in.txt files.txt

Lists files and directories in the current folder.

#### 5. ls -a -l ~ . -al ..
bash
Input: ls -a -l ~ . -al ..
Output: Detailed listing of system home (~), current (.), and parent (..) directories.

Shows hidden files and permissions in multiple directories.

#### ls -al
bash
Input: ls -al
Output: Detailed listing of all files in current directory.

Includes file sizes, permissions, and timestamps.

---
#### 6. cd navigation 
### 📍 terminal Path: /home/user/monsoon 2025/AOS/Assignment2


##### 1. `cd directory`
**bash**  
**Input:** `cd directory`  
**Output:** Navigated to `/directory`  

Moves into the `directory` folder located inside the terminal's  directory.

---


##### 2. `cd ..`
**bash**  
**Input:** `cd ..`  
**Output:** Navigated to `~>`  

Moves one level up from the current directory which is terminal directory.

---


##### 3. `cd ..`
**bash**  
**Input:** `cd ..`  
**Output:** Navigated to `/home/eswar/monsoon 2025/AOS>`  

Moves up another level to the parent directory.

---

### 📍 Path: `/home/eswar/monsoon 2025/AOS`

##### 4. `cd ~`
**bash**  
**Input:** `cd ~`  
**Output:** Navigated to `/home/eswar`  

The tilde `~` is a shortcut to return to the user's home directory.

---

### 📍 Path: `/home/eswar`

##### 5. `cd -`
**bash**  
**Input:** `cd -`  
**Output:** Navigated back to `/home/eswar/monsoon 2025/AOS`  

The `cd -` command switches to the previous working directory.

---


### 📍 Path: /home/user/monsoon 2025/AOS/testing

#### 7. gedit &
bash
Input: gedit &
Output: 5919

Launches gedit in the background. PID 5919 is returned.

#### 8. pinfo
bash
Input: pinfo
Output: Info about process 5476 (a.out from Assignment2)

Displays process status, memory usage, and executable path.

#### 9. pinfo 5919
bash
Input: pinfo 5919
Output: Status: Z (Zombie), Executable Path: Unknown

The gedit process has terminated but not yet cleaned up.

---

### 📍 Path: /home/user/monsoon 2025/AOS/testing/inside_testing

#### 10. ls
bash
Input: ls

Output: file2.txt file3.txt file1.txt

Lists files inside the inside_testing directory.

---

### 📍 Path: /home/user/monsoon 2025/AOS/testing

#### 11. search file1.txt
bash
Input: search file1.txt

Output: True

Confirms presence of file1.txt in the current or subdirectory.

#### 12. search lines.txt
bash
Input: search lines.txt

Output: True

Confirms presence of lines.txt.

### 📍 Path: /home/user/monsoon 2025/AOS/testing/inside_testing

#### 13. search lines.txt (inside inside_testing)
bash
Input: search lines.txt

Output: False

File not found in inside_testing.

---

### 📍 Path: /home/user/monsoon 2025/AOS/testing

#### 14. echo hello >> output.txt
bash
Input: echo hello >> output.txt

Output: Appends "hello" to output.txt


#### 15. cat < output.txt
bash
Input: cat < output.txt
Output: hello

Reads content of output.txt.

---

### 📍 File: lines.txt

#### 16. cat lines.txt
bash
Input: cat lines.txt

Output:
aa  
cat  
banana  
carrot


#### 17. sort < lines.txt > lines_sorted.txt
bash
Input: sort < lines.txt > lines_sorted.txt

Output: Sorted content written to lines_sorted.txt


#### 18. cat < lines_sorted.txt
bash
Input: cat < lines_sorted.txt

Output:
aa  
banana  
carrot  
cat


#### 19. cat lines.txt | wc
bash
Input: cat lines.txt | wc

Output: 4 4 21

Counts lines, words, and bytes.

#### 20. cat lines.txt | head -3 | tail -2
bash
Input: cat lines.txt | head -3 | tail -2

Output:
cat  
banana

Extracts lines 2 and 3 from the file.

---

### 📍 IO redirection with pipeline

#### 21. ls | grep “.txt” > out.txt; cat < in.txt | wc -l > lines.txt
bash
Output:

Creates a file out.txt containing all filenames in the current directory that include .txt and Creates a file lines.txt containing the number of lines in in.txt.


Contents of `out.txt`:

lines.txt  
input.txt  
history.txt  
output.txt  
lines_sorted.txt  
in.txt  
out.txt  
files.txt

Contents of `in.txt`:

hi  
hello  
how are you!

Contents of `line.txt`:

3


---

### 📍 History Commands

#### 21. history
bash
Input: history
Output: Full list of previously executed commands including history


#### 24. history 5
bash
Input: history 5
Output: Last 5 commands including history 5


---