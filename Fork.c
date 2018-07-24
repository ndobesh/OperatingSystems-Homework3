// Lots of really good comments here

#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h> /* For mode constants */
#include <fcntl.h>    /* For O_* constants */
#include <stdlib.h>   /* For exit */
#include <unistd.h>

// You NEED to make this SOMETHING ELSE
#define MY_SHARED_MEMORY "/ndobesh"
// You NEED to make this SOMETHING ELSE


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

    //printf("Ha! It worked! My shared memory is at %p\n", the_data);



    return (0);

}
