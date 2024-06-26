#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define GRID_SIZE 32
#define BUFF_SIZE (GRID_SIZE * GRID_SIZE)

typedef struct {
    int X;
    int Y;
} coordinates_t;

coordinates_t get_coordinates(int relative_source, coordinates_t destination) {
    // formula to find coordinates relatives to a specific element in the 2D list
    // x = (x + offset)%width
    // x += x < 0 ? width : 0;
 
    //destination.X = relative_source / GRID_SIZE;
    destination.X = (2 * destination.X) % GRID_SIZE;
    destination.X += destination.X < 0 ? GRID_SIZE : 0;

    //destination.Y = relative_source % GRID_SIZE;
    destination.Y = (destination.Y + destination.X) % GRID_SIZE;
    destination.Y += destination.Y < 0 ? GRID_SIZE : 0;

    return destination;
}

void swap(int source, coordinates_t destination, unsigned char *buffer) {
 
    int targ = GRID_SIZE * destination.X + destination.Y;

    unsigned char temp = buffer[source];
    buffer[source] = buffer[targ];
    buffer[targ] = temp;
}

// flipping the bytes should be with XOR operation (a ^ b) ^ b = a
// to make the swap from buffer the equation is array[length * row + col] = value
void flipper(coordinates_t *targets, int size_targets, unsigned char *buffer) {

    coordinates_t destination = {0};
    
    for (int rep = 0; rep < 65536; rep++) {
        for (int index_grid = 0; index_grid < GRID_SIZE; index_grid++) {
            for (int index_coo = 0; index_coo < size_targets; index_coo++) {
                // getting destination position
                destination = get_coordinates(index_grid, targets[index_coo]);
                // swapping the values
                swap(index_grid, destination, buffer);
            }
        }
    }
}

void encode(FILE *in, FILE *out) {

    unsigned char buffer[BUFF_SIZE] = {0};
    size_t size = 0;
    int targets_count = 18;

    coordinates_t targets[18] = {
        // should cumulate a lot of rules to makes it more "random"
        {.X = 31, .Y = 7},
        {.X = 17, .Y = 19},
        {.X = 23, .Y = 3},
        {.X = 13, .Y = 13},
        {.X = 13, .Y = 3},
        {.X = 11, .Y = 5},
        {.X = 29, .Y = 2},
        {.X = 1, .Y = 21},
        {.X = 7, .Y = 23},
        {.X = 16, .Y = 17},
        {.X = 3, .Y = 3},
        {.X = 29, .Y = 29},
        {.X = 294, .Y = 23941},
        {.X = 9288, .Y = 111},
        {.X = 127, .Y = 2},
        {.X = 29, .Y = 2},
        {.X = 13, .Y = 3},
        {.X = 11, .Y = 1111},
    };

    do {
        size = fread(buffer, 1, BUFF_SIZE, in);

        // fill the buffer with zeros
        memset(buffer + size, 0, BUFF_SIZE - size);

        flipper(targets, targets_count, buffer);

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
