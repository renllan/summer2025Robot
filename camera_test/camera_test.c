#include "../asbae_robot/hw6.h"

int main(int argc, char * argv[]){
    struct video_interface_handle_t *         handle_video1;
    struct video_interface_handle_t*          handle_video2;
    
    static struct image_t       image;
    static struct image_t       image1; //arm camera feed

    unsigned char               IMG_RAW1[IMAGE_SIZE];//arm image
    struct pixel_format_RGB     * IMG_DATA1;
    unsigned char               IMG_RAW[IMAGE_SIZE];
    struct pixel_format_RGB     * IMG_DATA;

    IMG_DATA1 = (struct pixel_format_RGB*)&IMG_RAW1;
    IMG_DATA = (struct pixel_format_RGB*)&IMG_RAW;



    handle_video1 = video_interface_open( "/dev/video0" );
    video_interface_print_modes(handle_video1);

    if(!video_interface_set_mode_manual(handle_video1,3)){
        printf("failed to configure dev/video0 \n");
        return 1;
    }
    


    handle_video2 = video_interface_open( "/dev/video2" );
    video_interface_print_modes(handle_video1);

    if(!video_interface_set_mode_manual(handle_video1,3)){
        printf("failed to configure dev/video0 \n");
        return 1;
    }

    if(handle_video1 && handle_video2)
    {
        video_interface_get_image(handle_video1,&image);
        video_interface_get_image(handle_video2,&image1);
    }
}

