#include "../asbae_robot/hw6.h"
#include <stdio.h>

// Save image as PPM (P6, binary)
void save_rgb_image_as_ppm(const char* filename, struct pixel_format_RGB* img_data, int width, int height) {
    FILE* f = fopen(filename, "wb");
    if (!f) {
        printf("Failed to open %s for writing\n", filename);
        return;
    }
    fprintf(f, "P6\n%d %d\n255\n", width, height);
    for (int i = 0; i < width * height; i++) {
        fputc(img_data[i].R, f);
        fputc(img_data[i].G, f);
        fputc(img_data[i].B, f);
    }
    fclose(f);
    printf("Saved %s\n", filename);
}

int main(int argc, char * argv[]){
    struct video_interface_handle_t *         handle_video1;
    struct video_interface_handle_t*          handle_video2;
    
    static struct image_t       image;
    static struct image_t       image1; //arm camera feed

    unsigned char               IMG_RAW1[IMAGE_SIZE];//arm image
    struct pixel_format_RGB     * IMG_DATA1;
    unsigned char               IMG_RAW0[IMAGE_SIZE];
    struct pixel_format_RGB     * IMG_DATA0;
    unsigned char               IMG_RAW2[IMAGE_SIZE];//arm image
    struct pixel_format_RGB     * IMG_DATA2;
    unsigned char               IMG_RAW3[IMAGE_SIZE];
    struct pixel_format_RGB     * IMG_DATA3;

    IMG_DATA1 = (struct pixel_format_RGB*)&IMG_RAW1;
    IMG_DATA0 = (struct pixel_format_RGB*)&IMG_RAW0;

    IMG_DATA2 = (struct pixel_format_RGB*)&IMG_RAW2;
    IMG_DATA3 = (struct pixel_format_RGB*)&IMG_RAW3;

    handle_video1 = video_interface_open( "/dev/video0" );
    video_interface_print_modes(handle_video1);

    if(!video_interface_set_mode_manual(handle_video1,7)){
        printf("failed to configure dev/video0 \n");
        return 1;
    }
    handle_video2 = video_interface_open( "/dev/video2" );
    video_interface_print_modes(handle_video2);

    if(!video_interface_set_mode_manual(handle_video2,7)){
        printf("failed to configure dev/video0 \n");
        return 1;
    }

    if(handle_video1 && handle_video2)
    {
        for(int  i = 0 ; i<4;i++)
        {

        }
        video_interface_get_image(handle_video1,&image);
        memcpy(IMG_RAW0, (unsigned char *)&image,IMAGE_SIZE);
        memcpy(IMG_RAW1, (unsigned char *)&image,IMAGE_SIZE);
        memcpy(IMG_RAW2, (unsigned char *)&image,IMAGE_SIZE);
        memcpy(IMG_RAW3, (unsigned char *)&image,IMAGE_SIZE);

        save_rgb_image_as_ppm("cam1_cp1.ppm",IMG_DATA1,IMG_WIDTH,IMG_HEIGHT);
        save_rgb_image_as_ppm("cam1_cp2.ppm",IMG_DATA2,IMG_WIDTH,IMG_HEIGHT);
        save_rgb_image_as_ppm("cam1_cp3.ppm",IMG_DATA3,IMG_WIDTH,IMG_HEIGHT);
        save_rgb_image_as_ppm("cam1_cp4.ppm",IMG_DATA0,IMG_WIDTH,IMG_HEIGHT);

        video_interface_get_image(handle_video2,&image1);
        memcpy(IMG_RAW1, (unsigned char*)&image1, IMAGE_SIZE);
        memcpy(IMG_RAW0, (unsigned char *)&image1,IMAGE_SIZE);
        memcpy(IMG_RAW2, (unsigned char *)&image1,IMAGE_SIZE);
        memcpy(IMG_RAW3, (unsigned char *)&image1,IMAGE_SIZE);

        save_rgb_image_as_ppm("cam2_cp1.ppm",IMG_DATA1,IMG_WIDTH,IMG_HEIGHT);
        save_rgb_image_as_ppm("cam2_cp2.ppm",IMG_DATA2,IMG_WIDTH,IMG_HEIGHT);
        save_rgb_image_as_ppm("cam2_cp3.ppm",IMG_DATA3,IMG_WIDTH,IMG_HEIGHT);
        save_rgb_image_as_ppm("cam2_cp4.ppm",IMG_DATA0,IMG_WIDTH,IMG_HEIGHT);

        


    }
}

