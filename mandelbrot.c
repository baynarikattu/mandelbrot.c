#include <assert.h>
#include <stdio.h>
#include <stdint.h>

#if 0
#define FORMAT_SCREEN
#else
#define FORMAT_PPM
#endif

#if defined(FORMAT_SCREEN)
#define WIDTH (151)
#define HEIGHT (37)
#elif defined(FORMAT_PPM)
#define WIDTH 800
#define HEIGHT 600
#endif
#define ASPECT ((double)WIDTH/HEIGHT)
#define MAX_ITERATIONS 2000
#define RES_BIT 29
#define RES (1 << RES_BIT)

#define ZOOM_FACTOR (280000)
#define ZOOM (1.0/ZOOM_FACTOR)
#define TRANS_X (-0.743643900055)
#define TRANS_Y (0.131825890901)

#define ARRAY_LEN(arr) sizeof(arr) / sizeof(*arr)

double grid[WIDTH*HEIGHT];
uint8_t pixels[WIDTH*HEIGHT*3];
char symbols[WIDTH*HEIGHT];

/* It returns a value from 0.0 to 1.0 */
double in_mandelbrot_set_blurred(double fCr, double fCi) {
    int64_t Cr = fCr * RES;
    int64_t Ci = fCi * RES;
    int64_t Cr2 = Cr * RES;
    int64_t Ci2 = Ci * RES;

    int64_t zi = 0, zr = 0, nzi = 0, nzr = 0;

    for (int i = 0; i < MAX_ITERATIONS; i++) {
        nzr = (zr * zr - zi * zi + Cr2) >> RES_BIT; /* f = R */
        nzi = (    2 * zr * zi   + Ci2) >> RES_BIT; /* f = R */
        zr = nzr;                                   /* f = R */
        zi = nzi;                                   /* f = R */

        if ((zr * zr + zi * zi) >= (INT64_C(4) << RES_BIT * 2)) return (double)i / MAX_ITERATIONS;
    }

    return 1.0;
}

int main(void) {
    for (int j = 0; j < HEIGHT; ++j) {
        double y = ((double)j / HEIGHT*2.0 - 1.0)*ZOOM + TRANS_Y;

        for (int i = 0; i < WIDTH; ++i) {
            double x = ((double)i / WIDTH*2.0 - 1.0)*ASPECT*ZOOM + TRANS_X;

            grid[i + j*WIDTH] = in_mandelbrot_set_blurred(x, y);
        }
    }

#if defined(FORMAT_PPM)
    for (int j = 0; j < HEIGHT; ++j) {
        for (int i = 0; i < WIDTH; ++i) {
            uint8_t brightness = grid[i + j*WIDTH]*255;
            pixels[(i + j*WIDTH)*3 + 0] = brightness;
            pixels[(i + j*WIDTH)*3 + 1] = brightness;
            pixels[(i + j*WIDTH)*3 + 2] = brightness;
        }
    }

    const char *filename = "output.ppm";

    FILE *f = fopen(filename, "wb");
    assert(f);

    fprintf(f, "P6 %d %d 255\n", WIDTH, HEIGHT);
    fwrite(pixels, 3, WIDTH*HEIGHT, f);
    fclose(f);

    fprintf(stderr, "%s was written.\n", filename);
#elif defined(FORMAT_SCREEN)
    char GRADIENT[] = " .:/a@";
    int GRADIENT_SIZE = ARRAY_LEN(GRADIENT) - 1; // excluding null
    for (int j = 0; j < HEIGHT; ++j) {
        for (int i = 0; i < WIDTH; ++i) {
            symbols[i + j*WIDTH] = GRADIENT[(int)(grid[i + j*WIDTH] * (GRADIENT_SIZE - 1))];
        }
    }
    printf("%.*s", WIDTH * HEIGHT, symbols);
#endif

    return 0;
}
