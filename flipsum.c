#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <signal.h>

#define GRID_SIZE 4096
#define BUFF_SIZE (GRID_SIZE * GRID_SIZE)

typedef struct {
    int X;
    int Y;
} coordinates_t;

/*
 * NOTES
 *
 * what it's underneath is far too complex but it's my reflexions i've got first so i'm leaving them there to reming me of how i could do it better than first thoughts lol.
 * 
 * For now, the algorithm is pretty much fine, it's shuffling very well but there is some problems.
 * The most problematic is that sometimes there can be untouched information and it's not what we want.
 * Maybe the solution to this is just to find the adequate ruleset to shuffle everything. As i'm not able yet to define an adequate one, i've thinked about some alternative methods :
 *
 *      One method would be to automatically calculate a ruleset from a given value, this will make the algorithm seeded. I really don't like the seeded method very much because it's more of a cipher than a pseudo random algorithm. I also want to use it to make ZKP protocol handshakes so if it's seeded with a non static value, it'll be harder to make it work well.
 *      Maybe the solution would be to factorial the coordinate found with the relative value, and then, wrap around the GRID_SIZE to make it fit into the grid. Factorial is generally good for hashing algorithms as i've read in the past, but also for pseudo-random generation. In my case, it's adding a layer of randomness and will (maybe ?) augment entropy of the output.
 *
 * ACTUAL PROBLEMS:
 *      If the GRID_SIZE is larger than the file size, the buffer is empty, swapping existing values with nothing. Maybe this is why the file comes empty after processing, it seem's not to write the whole buffer afterwards encoding.
*/

void print_buffer(unsigned char *buffer) {
    for (int index = 0; index < BUFF_SIZE; index++) {
        for (int stopper = 0; stopper < GRID_SIZE; stopper++) {
            printf("%u", buffer[index]);
            if (stopper == GRID_SIZE-1) {
                printf("\n");
            }
        }
    }
    printf("\n\nNEXT\n\n");
}

coordinates_t get_coordinates(int relative_source, coordinates_t offset) {

    // converting a 1D coordinates (X alone) into a 2D one (X and Y)
    coordinates_t source_coordinates = {0};
    source_coordinates.X = relative_source % GRID_SIZE;
    source_coordinates.Y = relative_source / GRID_SIZE;

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
                swap(index_grid, destination, index_grid, buffer);
            }
        }
    }
}

void encode(FILE *in, FILE *out) {

    unsigned char buffer[BUFF_SIZE] = {0};
    size_t size = 0;

    coordinates_t targets[] = {
        /*
        {13, -1},
        {-1, 31},
        {-11, 11},
        {-1, -1},
        {0, -7},
        {1, 0},
        {0, 10},
        {-5, 0},
        */
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
        
        /*
        // side swapping
        {.X = -3, .Y = 0},
        {.X = 0, .Y = 3},
        {.X = 3, .Y = 0},
        {.X = 0, .Y = -3},
        // diagonal swapping
        {.X = -3, .Y = -3},
        {.X = 3, .Y = 3},
        {.X = -3, .Y = 3},
        {.X = 3, .Y = -3},
        */
    };
    
    int targets_count = sizeof(targets) / sizeof(targets[0]);

    do {
        size = fread(buffer, 1, BUFF_SIZE, in);

        // fill the buffer with zeros
        memset(buffer + size, '.', BUFF_SIZE - size);
        
        //print_buffer(buffer);
        
        flipper(targets, targets_count, buffer);
        fwrite(buffer, 1, size, out);

        //printf("AFTER SCRAMBLING");

        //print_buffer(buffer);

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
