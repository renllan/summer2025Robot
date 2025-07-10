#include <stdio.h>
#include <stdlib.h>
#include "pixel_format_RGB.h"
#include <stdint.h>  
#include <math.h>

#define WIDTH  320
#define HEIGHT 240

void edge_to_red(
    struct pixel_format_RGB *  input,
    struct pixel_format_RGB *  output);

void sobel_edge_detect(struct pixel_format_RGB * input, struct pixel_format_RGB*output);