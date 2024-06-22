#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFF_SIZE 1024

struct Coordinates {
    uint8_t X;
    uint8_t Y;
};

// flipping the bytes should be with XOR operation (a ^ b) ^ b = a
// to make the swap from buffer the equation is array[length * row + col] = value
void flipper(struct Coordinates targets[7], unsigned char *buffer, size_t size) {
    int coo;
    int index;
    for (coo = 0; coo < 7; coo++) {
        for (index = 0; index < size; index++) {
            buffer[index] ^= buffer[size * targets[coo].X + targets[coo].Y];
            buffer[size * targets[coo].X + targets[coo].Y] ^= buffer[index];
            buffer[index] ^= buffer[size * targets[coo].X + targets[coo].Y];
        }
    }
}

void encode(FILE *in, FILE *out) {
    unsigned char buffer[BUFF_SIZE];
    size_t size;

    struct Coordinates targets[7] = {
        {.X = 251, .Y = 7},
        {.X = 29, .Y = 61},
        {.X = 97, .Y= 3},
        {.X = 127, .Y = 101},
        {.X = 181, .Y = 13},
        {.X = 47, .Y = 67},
        {.X = 229, .Y = 73},
    };

    do {
        size = fread(buffer, 1, BUFF_SIZE, in);

        flipper(targets, buffer, size);

        fwrite(buffer, 1, size, out);
    
    } while (size == BUFF_SIZE);
}

int main(int argc, char **argv) {

    char path[128];
    FILE *in, *out;

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
