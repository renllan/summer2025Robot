#include "edge_detection.h"
#define IDX(y, x) ((y) * WIDTH + (x))

void sobel_edge_detect(struct pixel_format_RGB * input, struct pixel_format_RGB*output) {
    int gx, gy, x, y, val;
    // Sobel kernels
    int Gx[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };
    int Gy[3][3] = {
        {-1, -2, -1},
        { 0,  0,  0},
        { 1,  2,  1}
    };
    unsigned char gray[HEIGHT][WIDTH];
    int gx, gy, x, y, val;

    // Convert to grayscale
    for (y = 0; y < HEIGHT; y++) {
        for (x = 0; x < WIDTH; x++) {
            int idx = IDX(y, x);
            gray[y][x] = (unsigned char)(
                0.299 * input[idx].R +
                0.587 * input[idx].G +
                0.114 * input[idx].B);
        }
    }
    // Apply Sobel edge detection
    for (y = 1; y < HEIGHT - 1; y++) {
        for (x = 1; x < WIDTH - 1; x++) {
            gx = gy = 0;
            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    gx += Gx[ky+1][kx+1] * gray[y+ky][x+kx];
                    gy += Gy[ky+1][kx+1] * gray[y+ky][x+kx];
                }
            }
            val = (int)sqrt(gx * gx + gy * gy);
            if (val > 255) val = 255;

            int idx = IDX(y, x);
            if (val > 100) { // Threshold for edge
                output[idx].R = 255;
                output[idx].G = 0;
                output[idx].B = 0;
            } else {
                output[idx] = input[idx];
            }
        }
    }

    // Copy the borders unchanged
    for (x = 0; x < WIDTH; x++) {
        output[IDX(0, x)] = input[IDX(0, x)];
        output[IDX(HEIGHT-1, x)] = input[IDX(HEIGHT-1, x)];
    }
    for (y = 0; y < HEIGHT; y++) {
        output[IDX(y, 0)] = input[IDX(y, 0)];
        output[IDX(y, WIDTH-1)] = input[IDX(y, WIDTH-1)];
    }
}

void edge_to_red(
    struct pixel_format_RGB *  input,
    struct pixel_format_RGB *  output)
{
    unsigned char* edge_raw = (unsigned char*)malloc(HEIGHT*WIDTH);
    struct pixel_format_RGB *edge = (struct pixel_format_RGB*)edge_raw;
    sobel_edge_detect(input, edge);

    int threshold = 100; // Set an appropriate threshold for edges

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            int idx = IDX(y,x);
            if (edge[idx].B > threshold) {
                // Edge pixel: make it red
                output[idx].R = 255; // R
                output[idx].G = 0;   // G
                output[idx].B = 0;   // B
            } else {
                // Non-edge: copy original, convert grayscale to RGB
                output[idx].R = input[idx].R;
                output[idx].G = input[idx].G;
                output[idx].B = input[idx].B;
            }
        }
    }
    free(edge_raw);
}