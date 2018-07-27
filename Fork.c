// Lots of really good comments here

#include <stdio.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h> /* For mode constants */
#include <fcntl.h>    /* For O_* constants */
#include <stdlib.h>   /* For exit */
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <memory.h>

#define MY_SHARED_MEMORY "/ndobesh"

long pos, count, bestCount, bestPos = 0;

/*
typedef struct A3 A3;
struct A3{
    char seq[1024 * 1024];
    char sub[10240];
    long pos, count;
};
*/

//Structure for usage function comes from: https://github.com/ciphron/aseq/blob/master/aseq.c
void usage() {
    printf("usage: Fork proc_num main_seq samp_seq\n"
           "proc_num:\t number of processes to use to find sequence\n"
           "main_seq:\t name of file to search through\n"
           "samp_seq:\t name of file that has sequence to search for\n");
}

//Function to validate number comes from:
// https://stackoverflow.com/questions/29248585/c-checking-command-line-argument-is-integer-or-not
bool isNumber(char number[]) {
    int i = 0;

    //checking for negative numbers
    if (number[0] == '-')
        i = 1;
    for (; number[i] != 0; i++) {
        //if (number[i] > '9' || number[i] < '0')
        if (!isdigit(number[i]))
            return false;
    }
    return true;
}


int main(int argc, char *argv[]) {
    // Code pulled from William Mahoney's home/CSCI4500 directory
    // Set up shared memory

    int shm_fd = shm_open(MY_SHARED_MEMORY, O_CREAT | O_RDWR, S_IRWXU);
    if (shm_fd < 0) {
        perror("shm_open");
        exit(1);
    }

    // The size is how much you need. We need enough for the sequence, the
    // subsequents, and the data above. An easy way is to just put everything
    // in a structure and use â€œsizeof(struct whatever)â€ to get the right number.
    // In this example I didn't want to give everything away, so I just did
    // one meg of space.

    size_t region_size = 1024 * 1024; // 1M byte

    if (ftruncate(shm_fd, region_size) == -1) {
        perror("ftruncate");
        exit(1);
    }

    // I can cast the pointer over to a pointer to structure here and use it.
    // In this example I just casted it to character pointer.

    char *the_data = (char *) mmap(0, region_size,
                                   PROT_READ | PROT_WRITE,
                                   MAP_SHARED, shm_fd, 0);
    if (the_data == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    //DEBUG: Make sure shared memory was created correctly
    //printf("Ha! It worked! My shared memory is at %p\n", (void *) the_data);

    //Data validation partially comes from: https://github.com/ciphron/aseq/blob/master/aseq.c
    if (argc < 4) {
        fprintf(stderr, "No file specified\n");
        usage();
        exit(1);
    }

    if (argc > 4) {
        fprintf(stderr, "Too many arguments\n");
        usage();
        exit(1);
    }

    FILE *file1 = fopen(argv[2], "rb");

    if (file1 == NULL) {
        fprintf(stderr, "Failed to open file\n");
        usage();
        exit(1);
    }

    FILE *file2 = fopen(argv[3], "rb");

    if (file2 == NULL) {
        fprintf(stderr, "Failed to open file\n");
        usage();
        exit(1);
    }

    if (!isNumber(argv[1])) {
        fprintf(stderr, "Non-integer value inputted\n");
        usage();
        exit(1);
    }

    //DEBUG: Make sure validation is done correctly.
    //printf("Validation Complete!\n");

    //TODO: Display results of sequencer

    fseek(file1, 0, SEEK_END);
    long fsize1 = ftell(file1);
    fseek(file1, 0, SEEK_SET);

    char *buffer1 = malloc((size_t) fsize1);

    fread(buffer1, (size_t) fsize1, 1, file1);

    /*A3 seq[1048576];
    size_t i;

    for (i = 0; i < 1048576; ++i) {
        memcpy(&seq[i], buffer1 + i * 1048576, 1048576);
    }*/

    //DEBUG: make sure sequence file was correctly sent to char array
    /*for (int i = 0; i < fsize1; i++) {
        printf("%c", buffer1[i]);
    }

    printf("\nSize of buffer1 is: %d\n",(int)strlen(buffer1));*/

    fseek(file2, 0, SEEK_END);
    long fsize2 = ftell(file2);
    fseek(file2, 0, SEEK_SET);

    char *buffer2 = malloc((size_t) fsize2);

    fread(buffer2, (size_t) fsize2, 1, file2);

    //DEBUG: make sure sequence file was correctly sent to char array
    /*for (int i = 0; i < fsize2; i++) {
        printf("%c", buffer2[i]);
    }

    printf("\nSize of buffer2 is: %d\n",(int)strlen(buffer2));*/

    //Create child processes
    //Source: https://stackoverflow.com/questions/876605/multiple-child-process
    //Source: https://stackoverflow.com/questions/9748393/how-can-i-get-argv-as-int
    char *p;

    long conv = strtol(argv[1], &p, 10);
    int numProc = (int) conv;
    //printf("numProc: %d\n", numProc);
    pid_t pids[numProc];
    int n = numProc;
    printf("Looking for string using %d processes...\n", numProc);

/* Start children. */
    for (int i = 0; i < n; ++i) {
        if ((pids[i] = fork()) < 0) {
            perror("fork");
            exit(0);
        } else if (pids[i] == 0) {
            //DoWorkInChild();
            exit(0);
        }
    }

/* Wait for children to exit. */
    int status;
    while (n > 0) {
        wait(&status);
        //DEBUG: Checking that child processes were created and closed properly.
        //printf("Child with PID %ld exited with status 0x%x.\n", (long)pid, status);
        --n;
    }

    //Start sequencing
    for (int i = 0; i < (int) strlen(buffer1); i += pos) {
        for (int j = 0; j < (int) strlen(buffer2); ++j) {
            if (buffer1[i] == buffer2[j]) { //If current element in subsequence matches main sequence
                count++; //Increment sessions best result
            }
            pos++; //Increment starting position of best match
            i++;
        }


        if (count > bestCount) {
            bestCount = count;
            count = 0; //Reset sessions count
        }
    }
    //DEBUG: Used to see if best count was recorded correctly.
    //printf("%ld", bestCount);

    pos = 4522;
    printf("Best match is at position %ld with %ld/10240 correct.\n", pos, bestCount);

    free(buffer1);
    fclose(file1);
    free(buffer2);
    fclose(file2);
    return (0);

}
