// Lots of really good comments here

#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h> /* For mode constants */
#include <fcntl.h>    /* For O_* constants */
#include <stdlib.h>   /* For exit */
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>

#define MY_SHARED_MEMORY "/ndobesh"

//Structure for usage function comes from: https://github.com/ciphron/aseq/blob/master/aseq.c
void usage() {
    printf("usage: Fork proc_num main_seq samp_seq\n"
           "proc_num:\t number of processes to use to find sequence\n"
           "main_seq:\t name of file to search through\n"
           "samp_seq:\t name of file that has sequence to search for\n");
}

//Function to validate number comes from:
// https://stackoverflow.com/questions/29248585/c-checking-command-line-argument-is-integer-or-not
bool isNumber(char number[])
{
    int i = 0;

    //checking for negative numbers
    if (number[0] == '-')
        i = 1;
    for (; number[i] != 0; i++)
    {
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

    printf("Ha! It worked! My shared memory is at %p\n", the_data);

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

    printf("Validation Complete!\n");

    //TODO: Create char array from files
    //TODO: Create sequencer
    //TODO: Display results of sequencer

    fseek(file1, 0, SEEK_END);
    long fsize = ftell(file1);
    fseek(file1, 0, SEEK_SET);

    char *buffer = malloc((size_t) fsize);

    fread(buffer, fsize, 1, file1);

    fclose(file1);

    //DEBUG: make sure sequence file was correctly sent to char array
    for (int i = 0; i < fsize; i++) {
        printf("%c", buffer[i]);
    }

    free(buffer);



    return (0);

}
