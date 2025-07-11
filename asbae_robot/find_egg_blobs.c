#include "find_egg_blobs.h"
#define PIXEL_RATIO_H 80
#define PIXEL_RATIO_L 60
#define PIXEL_THRESH_H 15000
#define PIXEL_THRESH_L 300
#define ASPECT_H 5
#define ASPECT_L  0.5

void reset_queue(PointQueue *q)
{
    q->front = 0;
    q->back = 0;
}
int queue_empty(PointQueue *q) {
    return q->front >= q->back;
}

bool enqueue(PointQueue *q, int x, int y) {
    if (q->back < MAX_QUEUE) {
        q->data[q->back++] = (Point){x, y};
        return true;
    }
    else{
        return false;
    }
}

bool dequeue(PointQueue *q, Point *out) {
    if (queue_empty(q)) {
        return false;  // underflow
    }
    *out = q->data[q->front++];
    return true;
}


int is_white(struct pixel_format_RGB p) {
    return p.R == 255 && p.G == 255 && p.B == 255;
}
int find_egg_blobs(struct pixel_format_RGB* img, EggBlob* eggs, int max_eggs, int width, int height) {
    assert(img != NULL && eggs != NULL && max_eggs >= 0 && width > 0 && height > 0);

    int (*visited)[width] = calloc(height, sizeof(*visited));
    int egg_count = 0;
    PointQueue que;
    reset_queue(&que);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (visited[y][x] == 0 && is_white(img[y * width + x])) {
                reset_queue(&que);
                if (!enqueue(&que, x, y)) {
                    while (!queue_empty(&que)) {
                        Point dummy;
                        dequeue(&que, &dummy);  // drain queue
                    }
                    printf("Queue overflow at blob starting (%d,%d)\n", x,y);
                    
                }
                visited[y][x] = 1;

                int min_x = x, max_x = x, min_y = y, max_y = y;
                int sum_x = 0, sum_y = 0, pixels = 0;
                int white_pixels = 0;
                while (!queue_empty(&que)) {
                    Point p;
                    dequeue(&que, &p);
                    sum_x += p.x;
                    sum_y += p.y;
                    pixels++;

                    int dx[] = {0, 0, -1, 1};
                    int dy[] = {-1, 1, 0, 0};

                    for (int i = 0; i < 4; i++) {
                        int nx = p.x + dx[i];
                        int ny = p.y + dy[i];

                        if (nx >= 0 && nx < width && ny >= 0 && ny < height &&
                            !visited[ny][nx] && is_white(img[ny * width + nx])) {
                            visited[ny][nx] = 1;
                            if (!enqueue(&que, nx, ny)) {
                                while (!queue_empty(&que)) {
                                    Point dummy;
                                    dequeue(&que, &dummy);  // drain queue
                                }
                            }
                            white_pixels++;
                            if (nx < min_x) min_x = nx;
                            if (ny < min_y) min_y = ny;
                            if (nx > max_x) max_x = nx;
                            if (ny > max_y) max_y = ny;
                        }
                    }
                }
                
                

                int w = max_x - min_x + 1;
                int h = max_y - min_y + 1;
                int total_pixels = w*h;
                float aspect = (float)w / h;
                float white_pixels_ratio = 100*(float)white_pixels/total_pixels;
                //printf("eggs at %d , %d pixels %d, aspect: %f ,ratio %f\n",sum_x / pixels,sum_y / pixels,pixels,aspect,white_pixels_ratio);
                if (white_pixels_ratio> PIXEL_RATIO_L && white_pixels_ratio< PIXEL_RATIO_H 
                    && pixels >PIXEL_THRESH_L 
                     && aspect > ASPECT_L && aspect < ASPECT_H) {
                    if (egg_count < max_eggs) {
                        
                        eggs[egg_count].min_x = min_x;
                        eggs[egg_count].max_x = max_x;
                        eggs[egg_count].min_y = min_y;
                        eggs[egg_count].max_y = max_y;
                        eggs[egg_count].center_x = sum_x / pixels;
                        eggs[egg_count].center_y = sum_y / pixels;
                        eggs[egg_count].size = pixels;
                        egg_count++;
                    }
                }
            }
        }
    }
    free(visited);
    //printf("egg count: %d \n",egg_count);
    return egg_count;
}
