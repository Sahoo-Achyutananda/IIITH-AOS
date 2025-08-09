#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>

#define bufferSize 100000

using namespace std;

char chars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

void genRandomBuff(char buff[], int size) {
    for (int i = 0; i < size; i++) {
        buff[i] = chars[rand() % 62];
    }
}

int main(int argc, char* argv[]) {
    char fname[256];
    strcpy(fname, "input.txt");
    long long fsize = 1000000;
    
    if (argc >= 2) {
        strcpy(fname, argv[1]);
    }
    if (argc >= 3) {
        fsize = strtoll(argv[2], nullptr, 10);
    }
    
    srand(69420);
    
    int ofd = open(fname, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (ofd == -1) {
        printf("Can't create file!!!\n");
        exit(1);
    }
    
    char* buff = (char*)malloc(bufferSize);
    
    struct timeval start, curr;
    gettimeofday(&start, NULL);
    
    long long written = 0;
    long long chunks = fsize / bufferSize;
    int rem = fsize % bufferSize;
    
    printf("Generating file %s with %lld chars...\n", fname, fsize);
    
    for (long long i = 0; i < chunks; i++) {
        genRandomBuff(buff, bufferSize);
        write(ofd, buff, bufferSize);
        written += bufferSize;
        
        if (i % 500 == 0) {
            gettimeofday(&curr, NULL);
            double elapsed = (curr.tv_sec - start.tv_sec) + (curr.tv_usec - start.tv_usec) / 1000000.0;
            double percent = ((double)written / fsize) * 100;
            double speed = (written / 1024.0 / 1024.0) / elapsed;
            
            int bars = (int)(percent / 2);
            printf("\r[");
            for (int j = 0; j < 50; j++) {
                if (j < bars) printf("=");
                else printf(" ");
            }
            printf("] %.1f%% %.1fMB/s", percent, speed);
            fflush(stdout);
        }
    }
    
    if (rem > 0) {
        genRandomBuff(buff, rem);
        write(ofd, buff, rem);
        written += rem;
    }
    
    gettimeofday(&curr, NULL);
    double total_time = (curr.tv_sec - start.tv_sec) + (curr.tv_usec - start.tv_usec) / 1000000.0;
    
    printf("\nDone! Wrote %lld chars in %.2f seconds\n", written, total_time);
    
    free(buff);
    close(ofd);
    return 0;
}


/*
    This file can be used with the following args:
    executable <out_file.txt> <number of chars>

    1GB will have number of characters of order 1e9 approx so generate carefully.
    A single extra zero can lead to crashing of your entire system. Don't try to open the generated file in any text editor.
*/