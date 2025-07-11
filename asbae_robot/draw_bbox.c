#include "draw_bbox.h"
void draw_bbox(int min_x,int min_y, int max_x, int max_y, struct pixel_format_RGB *egg_data,struct pixel_format_RGB color)
{
  for (int x = min_x; x <= max_x; x++) {
    egg_data[min_y * IMG_WIDTH + x] = color;
    egg_data[max_y * IMG_WIDTH + x] = color;
  }
  for (int y = min_y; y <= max_y; y++) {
      egg_data[y * IMG_WIDTH + min_x] = color;
      egg_data[y * IMG_WIDTH + max_x] = color;
  }
}