#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <signal.h>

#define GRID_SIZE 32
#define BUFF_SIZE (GRID_SIZE * GRID_SIZE)

typedef struct {
    int X;
    int Y;
} coordinates_t;

void print_buffer(unsigned char *buffer) {
    for (int Y = 0; Y < GRID_SIZE; Y++) {
        for (int X = 0; X < GRID_SIZE; X++) {
            printf("%c", (char) buffer[X + Y*GRID_SIZE]);
        }
        printf("\n");
    }
}
/*
int count_non_null_chars(unsigned char *buffer) {

    int count = 0;

    for (int index = 0; index < BUFF_SIZE; index++) {
        switch (buffer[index]) {
            case 0:
                continue;
            default:
                count++;
        }
    }
    return count;
}*/


void remove_null_chars(unsigned char *buffer, unsigned char *new_buffer) {

    for (int index = 0; index < BUFF_SIZE; index++) {
        switch (buffer[index]) {
            case 0:
                continue;
            default:
                new_buffer[index] = buffer[index];
        }
    }
}

coordinates_t get_coordinates(int relative_source, coordinates_t offset) {

    // converting a 1D coordinates (X alone) into a 2D one (X and Y)
    coordinates_t source_coordinates = {0};
    source_coordinates.X = relative_source % GRID_SIZE;
    source_coordinates.Y = relative_source / GRID_SIZE;
    assert(source_coordinates.X >= 0);
    assert(source_coordinates.Y >= 0);

    // calculating the target's position relatives to the relative_source's
    coordinates_t out = {0};
    out.X = (source_coordinates.X + offset.X) % GRID_SIZE;
    out.X += out.X < 0 ? GRID_SIZE : 0;

    out.Y = (source_coordinates.Y + offset.Y) % GRID_SIZE;
    out.Y += out.Y < 0 ? GRID_SIZE : 0;

    return out;
}

void swap(int source, coordinates_t destination, int offset, unsigned char *buffer) {
 
    assert(destination.X >= 0);
    assert(destination.Y >= 0);

    int targ = GRID_SIZE * destination.X + destination.Y;

    assert(source < BUFF_SIZE);
    assert(targ < BUFF_SIZE);

    unsigned char temp = buffer[source];
    buffer[source] = buffer[targ];
    buffer[targ] = temp;
}

void flipper(coordinates_t *targets, int size_targets, unsigned char *buffer) {

    coordinates_t destination = {0};
    
    for (int rep = 0; rep < 129; rep++) {
        for (int index_grid = 0; index_grid < GRID_SIZE; index_grid++) {
            for (int index_coo = 0; index_coo < size_targets; index_coo++) {
                
                // getting destination position
                destination = get_coordinates(index_grid, targets[index_coo]);
                // swapping the values
                swap(index_grid, destination, index_grid, buffer);
            }
        }
    }
}

void encode(FILE *in, FILE *out) {

    unsigned char buffer[BUFF_SIZE] = {0};
    size_t size = 0;

    coordinates_t targets[] = {
        // side swapping
        {-5, 0},
        {0, 10},
        {1, 0},
        {0, -7},
        // diagonal swapping
        {-1, -1},
        {-11, 11},
        {-1, 31},
        {13, -1},
        /*/ other
        {-3, 3},
        {-1, 5},
        {-11, -11},
        {1021, -3},
        {-3, -3},
        {243, -132},
        {-11, 27},
        {-1, 31},
        {-12, 299},
        {-31, 31},
        {-91, -63},
        {31, 27},
        */
    };
    
    int targets_count = sizeof(targets) / sizeof(targets[0]);

    do {

        size = fread(buffer, 1, BUFF_SIZE, in);
        
        // used to prevent the '\0' char to broke the program
        memset(buffer + size, '.', BUFF_SIZE - size);
        //memset(buffer + size, 0, BUFF_SIZE - size);

        print_buffer(buffer);
        
        flipper(targets, targets_count, buffer);

        fwrite(buffer, 1, size, out);

        printf("AFTER\n");

        print_buffer(buffer);

    } while (size == BUFF_SIZE);

    /*
    int new_size = count_non_null_chars(buffer);
    // maybe create a function for this to be out of this scope ?
    unsigned char new_buffer[new_size] = {0};
    remove_null_chars(buffer, new_buffer);

    for (int index = 0; index < new_size; index++) {
        fwrite(new_buffer, 1, new_size, out);
    }
    */
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
