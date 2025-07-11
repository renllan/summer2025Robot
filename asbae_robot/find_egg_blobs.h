#include "pixel_format_RGB.h"
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#define MAX_QUEUE 100000
typedef struct {
    int min_x, min_y;
    int max_x, max_y;
    int center_x, center_y;
    int size;
} EggBlob;
typedef struct {
    int x, y;
} Point;

typedef struct{
    Point data[MAX_QUEUE];
    int front;
    int back;
}PointQueue;


void reset_queue(PointQueue *q);
bool enqueue(PointQueue *q, int x, int y) ;
bool dequeue(PointQueue *q, Point *out) ;
int is_white(struct pixel_format_RGB p);
int find_egg_blobs(struct pixel_format_RGB* img, EggBlob* eggs, int max_eggs, int width,int height);


