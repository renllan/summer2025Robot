#include "egg_sort.h"
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
        printf("Average RGB inside bbox: (R: %d, G:%d, B:%d)\n", avg_r, avg_g, avg_b);
    } else {
        printf("No pixels in bbox\n");
    }


}
int main(int argc, char * argv[])
{
    struct video_interface_handle_t *         handle_video1  = NULL;
    unsigned char               IMG_RAW1[IMAGE_SIZE];//arm image
    struct pixel_format_RGB     * IMG_DATA1;
    static struct image_t       image;

    unsigned char               IMG_RAW2[IMAGE_SIZE];//arm image
    struct pixel_format_RGB     * IMG_DATA2;

    struct draw_bitmap_multiwindow_handle_t * handle_GUI_RGB
     = draw_bitmap_create_window(IMG_WIDTH,IMG_HEIGHT);

    struct draw_bitmap_multiwindow_handle_t * handle_GUI_grey 
    =  draw_bitmap_create_window(IMG_WIDTH,IMG_HEIGHT);
   
    if(!video_interface_set_mode_auto(handle_video1)){
        printf("failed to configure dev/video0 \n");
        return 1;
    }
    
    while(handle_video1){
        video_interface_get_image(handle_video1, &image);
        memcpy(IMG_RAW1, (unsigned char *)&image, IMAGE_SIZE);
        memcpy(IMG_RAW2, (unsigned char *)&image, IMAGE_SIZE)
        draw_bitmap_display(handle_GUI_RGB,IMG_DATA1);
        int max_egg = 0; //max egg index
        EggBlob eggs[MAX_EGGS];
        to_black_white(IMG_DATA2, IMAGE_SIZE/3, EGG_THRESHOLD);
        
        int found = find_egg_blob(IMG_DATA2,MAX_EGGS,IMG_WIDTH,IMG_HEIGHT);
        for (int i = 0; i < found; i++) {
                //for drawing the red bbox
            int min_x = eggs[i].min_x;
            int max_x = eggs[i].max_x;
            int min_y = eggs[i].min_y;
            int max_y = eggs[i].max_y;

            // Draw red bounding box
            draw_bbox(min_x,min_y,max_x,max_y, egg_data_1,(struct pixel_format_RGB){255, 0, 0});
            if(eggs[i].size > eggs[max_egg].size){
                max_egg = i;
            }
            //printf("Egg %d at center (%d, %d), size %d\n", i+1, eggs[i].center_x, eggs[i].center_y, eggs[i].size);
        } 
        if(found > 0){
            int min_x = eggs[max_egg].min_x;
            int max_x = eggs[max_egg].max_x;
            int min_y = eggs[max_egg].min_y;
            int max_y = eggs[max_egg].max_y;
            draw_bbox(min_x,min_y,max_x,max_y,egg_data_1,(struct pixel_format_RGB){0, 255, 0});
            robot_decision_q[MAX_DECISION_SIZE-1] = eggs[max_egg].center_x;
            //printf("largest egg size is is at %d \n", eggs[max_egg].size);
            // egg_sort(eggs[max_egg].center_x, eggs[max_egg].center_y,param->RGB_IMG_DATA);
        }
        int center_x = eggs[max_egg].center_x;
        int center_y = eggs[max_egg].center_y;
        egg_sort(center_x,center_y,IMG_DATA_1);//display the egg sort
        draw_bitmap_display(handle_GUI_grey,IMG_DATA2);
    }


}
