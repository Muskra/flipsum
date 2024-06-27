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

/*
 * NOTES
 *
 * what it's underneath is far too complex but it's my reflexions i've got first so i'm leaving them there to reming me of how i could do it better than first thoughts lol.
 * 
 * For now, the algorithm is pretty much fine, it's shuffling very well but there is some problems.
 * The most problematic is that sometimes there can be untouched information and it's not what we want.
 * Maybe the solution to this is just to find the adequate ruleset to shuffle everything. As i'm not able yet to define an adequate one, i've thinked about some alternative methods :
 *
 *      To prevent this, i can add an offset to the calculated destinations from the relative cell. This will prevent that since it will apply the rule on the same range the cell is, but not every time.
 *      One other method would be to automatically calculate a ruleset from a given value, this will make the algorithm seeded. I really don't like the seeded method very much because it's more of a cipher than a pseudo random algorithm. I also want to use it to make ZKP protocol handshakes so if it's seeded with a non static value, it'll be harder to make it work well.
 *      Maybe the solution would be to factorial the coordinate found with the relative value, and then, wrap around the GRID_SIZE to make it fit into the grid. Factorial is generally good for hashing algorithms as i've read in the past, but also for pseudo-random generation. In my case, it's adding a layer of randomness and will (maybe ?) augment entropy of the output.
 *
*/

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

void swap(int source, coordinates_t destination, int offset, unsigned char *buffer) {
 
    // if the integer is positive, then nothing, else negates to be positive
    destination.X = destination.X ? destination.X : -destination.X;
    destination.Y = destination.Y ? destination.Y : -destination.Y;

    // tried to add an offset to force the generation to process farther in large files. Makes the program crash for now.
    destination.X += offset;
    destination.Y += offset;

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
                //printf("x: %d, y: %d", destination.X, destination.Y);
                // swapping the values
                swap(index_grid, destination, index_grid, buffer);
            }
        }
    }
}

void encode(FILE *in, FILE *out) {

    unsigned char buffer[BUFF_SIZE] = {0};
    size_t size = 0;
    int targets_count = 18;

    coordinates_t targets[18] = {
        // side swapping
        {.X = -1, .Y = 0},
        {.X = 0, .Y = 1},
        {.X = 1, .Y = 0},
        {.X = 0, .Y = -1},
        // diagonal swapping
        {.X = -1, .Y = -1},
        {.X = 1, .Y = 1},
        {.X = -1, .Y = 1},
        {.X = 1, .Y = -1},
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
