#include "to_black_white.h"

void to_black_white(struct pixel_format_RGB* img, int size, int thresh) {
    for(int i = 0; i < size; i++) {
        // Use weighted luminance conversion (standard formula)
        int luminance = (
            0.2126 * img[i].R + 
            0.7152 * img[i].G + 
            0.0722 * img[i].B
        );
        
        img[i].R = img[i].G = img[i].B = (luminance > thresh) ? 255 : 0;
    }
}