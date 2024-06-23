#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define GRID_SIZE 32
#define BUFF_SIZE (GRID_SIZE * GRID_SIZE)

struct Coordinates {
    uint8_t X;
    uint8_t Y;
};

// flipping the bytes should be with XOR operation (a ^ b) ^ b = a
// to make the swap from buffer the equation is array[length * row + col] = value
void flipper(struct Coordinates targets[7], unsigned char *buffer, size_t size_grid) {

    int rep, coo, index = 0;
    size_t size_targets = sizeof(&targets);
    
    // repeat the thing 128 times because why not doing that
    for (; rep < 128; rep++) {
        index = 0;
        coo = 0;
        for (; coo < size_targets; coo++) {
            for (; index < size_grid; index++) {
            
                int targ = size_grid * targets[coo].X + targets[coo].Y;
                assert(targ > size_grid);

                // swap values with bitwise XOR
                //printf("actual: %d, target: %d\n", buffer[index], buffer[targ]);
                
                buffer[index] ^= buffer[targ];
                //printf("actual: %d, target: %d\n", buffer[index], buffer[targ]);
                
                buffer[targ] ^= buffer[index];
                //printf("actual: %d, target: %d\n", buffer[index], buffer[targ]);
                
                buffer[index] ^= buffer[targ];
                //printf("actual: %d, target: %d\n", buffer[index], buffer[targ]);
            }
        }
    }
}

void encode(FILE *in, FILE *out) {

    unsigned char buffer[BUFF_SIZE] = {0};
    size_t size = 0;

    struct Coordinates targets[9] = {
        {.X = 12, .Y = 7},
        {.X = 29, .Y = 6},
        {.X = 17, .Y= 3},
        {.X = 27, .Y = 10},
        {.X = 28, .Y = 23},
        {.X = 30, .Y = 29},
        {.X = 2, .Y = 13},
        {.X = 31, .Y = 26},
        {.X = 11, .Y = 0},
    };

    do {
        size = fread(buffer, 1, BUFF_SIZE, in);

        // fill the buffer with zeros
        memset(buffer + size, 0, BUFF_SIZE - size);

        flipper(targets, buffer, GRID_SIZE);

        fwrite(buffer, 1, size, out);
    
    } while (size == BUFF_SIZE);
}

int main(int argc, char **argv) {

    char path[128] = {0};
    FILE *in, *out = NULL;

    if (argc != 2) {
        printf("Usage : %s [FILE]\n", argv[0]);
        return EXIT_FAILURE;
    }

    snprintf(path, sizeof(path)-1, "%s.flip", argv[1]);

    if ((in = fopen(argv[1], "r")) == NULL) {
        perror("fopen (in) ");
        return EXIT_FAILURE;
    }

    if ((out = fopen(path, "w")) == NULL) {
        perror("fopen (out) ");
        return EXIT_FAILURE;
    }

    encode(in, out);

    return EXIT_SUCCESS;
}
