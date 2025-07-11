#include "../include/pixel_format_RGB.h"
#define IMG_WIDTH 320
#define IMG_HEIGHT 240
#define BBOX_W   8
#define BBOX_H   8
void egg_sort(int center_x, int center_y, struct pixel_format_RGB *img)
{
    /* data */
    int min_x =center_x-BBOX_W/2;
    int min_y = center_y-BBOX_H/2;
    int max_x =center_x + BBOX_W/2;
    int max_y = center_y + BBOX_H/2;

    if (min_x < 0) min_x = 0;
    if (min_y < 0) min_y = 0;
    if (max_x > IMG_WIDTH) max_x = IMG_WIDTH;
    if (max_y > IMG_HEIGHT) max_y = IMG_HEIGHT;

    long sum_r = 0, sum_g = 0, sum_b = 0;
    int count = 0;
    
    for (int y = min_y; y < max_y; y++) {
        for (int x = min_x; x < max_x; x++) {
            struct pixel_format_RGB p = img[y * IMG_WIDTH + x];
            sum_r += p.R;
            sum_g += p.G;
            sum_b += p.B;
            count++;
        }
    }
    if (count > 0) {
        int avg_r = sum_r / count;
        int avg_g = sum_g / count;
        int avg_b = sum_b / count;
        printf("Average RGB inside bbox: (%d, %d, %d)\n", avg_r, avg_g, avg_b);
    } else {
        printf("No pixels in bbox\n");
    }


}

int main(){

}
