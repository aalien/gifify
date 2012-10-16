#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    unsigned int r, g, b;
} Pixel;

typedef struct {
    char *magic;
    unsigned int height;
    unsigned int width;
    unsigned int maxval;
    Pixel **pixels;
} Image;

Pixel TRANSPARENT = {0xdd, 0xff, 0xff};

Image *init_image(FILE *fp)
{
    char *buf = NULL;
    size_t nbuf = 0;

    /* Allocate memory */
    Image *image = malloc(sizeof(Image));
    image->magic = malloc(3*sizeof(char));

    /* Read PPM header */
    getline(&buf, &nbuf, fp);
    sscanf(buf, "%s", image->magic);
    if (strcmp(image->magic, "P6") != 0) exit(1);

    /* Discard Gimp's comment */
    getline(&buf, &nbuf, fp);

    getline(&buf, &nbuf, fp);
    sscanf(buf, "%u %u", &(image->width), &(image->height));
    getline(&buf, &nbuf, fp);
    sscanf(buf, "%u", &(image->maxval));

    return image;
}

void read_image(Image *image, FILE *fp)
{
    int x;
    int y;

    /* Allocate memory */
    image->pixels = malloc(image->height*image->width*sizeof(Pixel *));

    for (y = 0; y < image->height; ++y) {
        for (x = 0; x < image->width; ++x) {
            Pixel *pixel = malloc(sizeof(Pixel));
            fread(&(pixel->r), 1, 1, fp);
            fread(&(pixel->g), 1, 1, fp);
            fread(&(pixel->b), 1, 1, fp);
            image->pixels[(y*image->width)+x] = pixel;
        }
    }
}

void write_image(Image *image, FILE *fp)
{
    int x;
    int y;

    /* Write header */
    fprintf(fp, "%s\n", image->magic);
    fprintf(fp, "%i %i\n", image->width, image->height);
    fprintf(fp, "%i\n", image->maxval);

    /* Write image data */
    for (y = 0; y < image->height; ++y) {
        for (x = 0; x < image->width; ++x) {
            Pixel *pixel = image->pixels[(y*image->width)+x];
            fwrite(&(pixel->r), 1, 1, fp);
            fwrite(&(pixel->g), 1, 1, fp);
            fwrite(&(pixel->b), 1, 1, fp);
        }
    }

    fprintf(fp, "\n");
}

void free_image(Image *image)
{
    free(image->pixels);
    free(image->magic);
    free(image);
}

int BITSET_WIDTH = (8 * sizeof(unsigned int));

void set_bit(unsigned int *bitset, size_t idx)
{
    bitset[idx / BITSET_WIDTH] |= (1 << (idx % BITSET_WIDTH));
}

int get_bit(unsigned int *bitset, size_t idx)
{
    return bitset[idx / BITSET_WIDTH] & (1 << (idx % BITSET_WIDTH));
}

int main()
{
    Image *image = NULL;
    image = init_image(stdin);
    read_image(image, stdin);

    int ncolors;
    int *done = calloc(image->width * image->height, sizeof(int));
    Image *reduced = NULL;
    int frame = 0;
    int end = 0;

    reduced = malloc(sizeof(Image));
    reduced->pixels = malloc(image->height*image->width*sizeof(Pixel *));
    reduced->magic = malloc(3*sizeof(char));
    reduced->magic = strdup("P6");
    reduced->maxval = image->maxval;
    reduced->width = image->width;
    reduced->height = image->height;

    while (!end) {
        /* reset colors for next frame */
        unsigned int *colors = calloc((256*256*256 / 8) + 1, sizeof(unsigned int));
        ncolors = 0;

        end = 1;
        for (int y = 0; y < image->height; ++y) {
            for (int x = 0; x < image->width; ++x) {
                int pos = (y*image->width) + x;
                Pixel *orig = image->pixels[pos];
                Pixel *new = NULL;
                unsigned int pixel_color = ((orig->r << 16) | (orig->g << 8) | (orig->b));

                if (done[pos]) {
                    reduced->pixels[pos] = &TRANSPARENT;
                    continue;
                }

                if (!(get_bit(colors, pixel_color)) && ncolors < 255) {
                    /* reserve new color */
                    set_bit(colors, pixel_color);
                    ++ncolors;
                }

                if (get_bit(colors, pixel_color)) {
                    /* color pixel */
                    done[pos] = 1;
                    new = orig;
                    end = 0;
                } else {
                    new = &TRANSPARENT;
                }

                reduced->pixels[pos] = new;
            }
        }

        char filename[100];
        sprintf(filename, "frame%04i.ppm", frame);
        FILE *fp = fopen(filename, "wb");
        write_image(reduced, fp);
        fclose(fp);
        free(colors);
        ++frame;
    }

    return 0;
}   
