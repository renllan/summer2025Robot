#include "../include/pixel_format_RGB.h"
#include "../asbae_robot/hw6.h"
#include "../asbae_robot/draw_bbox.h"

#define IMG_WIDTH 320
#define IMG_HEIGHT 240
#define BBOX_W   8
#define BBOX_H   8
void egg_sort(int center_x, int center_y, struct pixel_format_RGB *img);
