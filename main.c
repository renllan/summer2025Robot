#include "hw6.h"
int main(int argc, char * argv[] )
{
    pthread_t tk; //key input thread
    pthread_t tc; //motor controll thread
    pthread_t ts; //speed control thread
    pthread_t td; //directin control thread
    pthread_t ti; //IR_sensor thread
    pthread_t tp; //video capture thread 
    pthread_t t_rbg; //rgb img with green cross
    pthread_t t_grey;
    pthread_t t_bw;
    pthread_t t_reduced;
    pthread_t t_hist;
    pthread_t t_egg;
    bool quit_flag = false;

    struct fifo_t key_fifo   = {{}, 0, 0, PTHREAD_MUTEX_INITIALIZER};
    struct fifo_t speed_fifo   = {{}, 0, 0, PTHREAD_MUTEX_INITIALIZER};
    struct fifo_t dir_fifo = {{}, 0, 0, PTHREAD_MUTEX_INITIALIZER};
    struct fifo_t IR_sensor_fifo = {{},0,0,PTHREAD_MUTEX_INITIALIZER};
    struct fifo_t motor_control_fifo = {{},0,0,PTHREAD_MUTEX_INITIALIZER};
    struct fifo_t img_fifo = {{},0,0,PTHREAD_MUTEX_INITIALIZER};
    struct fifo_t rgb_img_fifo = {{},0,0,PTHREAD_MUTEX_INITIALIZER};
    struct fifo_t greyscale_img_fifo = {{},0,0,PTHREAD_MUTEX_INITIALIZER};
    struct fifo_t bw_img_fifo = {{},0,0,PTHREAD_MUTEX_INITIALIZER};
    struct fifo_t reduced_img_fifo = {{},0,0,PTHREAD_MUTEX_INITIALIZER};
    struct fifo_t hist_fifo = {{},0,0,PTHREAD_MUTEX_INITIALIZER};
    struct fifo_t egg_fifo = {{},0,0,PTHREAD_MUTEX_INITIALIZER};

    static struct image_t       image;
    unsigned char               * IMG_RAW;
    struct pixel_format_RGB     * IMG_DATA;
    unsigned char               * RGB_IMG_raw;
    struct pixel_format_RGB     * RGB_IMG_data;
    unsigned char               * GREYSCALE_IMG_raw;
    struct pixel_format_RGB     * GREYSCALE_IMG_data;
    unsigned char               * BW_IMG_raw;
    struct pixel_format_RGB     * BW_IMG_data;
    unsigned char               * REDUCED_IMG_raw;
    struct pixel_format_RGB     * REDUCED_IMG_data;

    IMG_RAW = (unsigned char *)malloc(IMAGE_SIZE+1);
    RGB_IMG_raw = (unsigned char *)malloc(IMAGE_SIZE+1);
    GREYSCALE_IMG_raw = (unsigned char *)malloc(IMAGE_SIZE+1);
    BW_IMG_raw = (unsigned char *)malloc(IMAGE_SIZE+1);
    REDUCED_IMG_raw = (unsigned char *)malloc(IMAGE_SIZE+1);

    IMG_DATA = (struct pixel_format_RGB*)IMG_RAW;
    RGB_IMG_data = (struct pixel_format_RGB *)RGB_IMG_raw;
    GREYSCALE_IMG_data = (struct pixel_format_RGB*)GREYSCALE_IMG_raw;
    BW_IMG_data = (struct pixel_format_RGB*)BW_IMG_raw;
    REDUCED_IMG_data = (struct pixel_format_RGB*)REDUCED_IMG_raw;

    /*intial the parameter of motor_speed_thread    */
    struct motor_speed_thread_param motor_speed_param= 
    {"speed", NULL,NULL,12,13,&speed_fifo,&quit_flag,25};
    /*intial the parameter of motor_direction_thread    */
    struct motor_direction_thread_param motor_direction_param = 
    {"direction", NULL,5,6,22,23,&dir_fifo,&quit_flag, 's'};
    /*intial the parameter of key_input_thread   q */
    struct key_thread_param key_param = {
      "key",
      &key_fifo,    
      &quit_flag};
    /*intial the parameter of motor_control_thread    */
    struct motor_control_thread_param     con_param             = {"control", &key_fifo, &speed_fifo, &dir_fifo, &reduced_img_fifo,&img_fifo,&quit_flag, true};
    /*intialize the parameter of IR_sensor_thread
      - initialize to line tracing mode
    */
    struct IR_Sensor_param                IR_sensor_param      =  {"IR Senssor", &IR_sensor_fifo,&dir_fifo,NULL,24,25,&quit_flag};
    struct egg_detector_thread_param      egg_param = {
        "egg datection", 
        &egg_fifo, 
        &dir_fifo, 
        BW_IMG_raw,
        BW_IMG_data, 
        &quit_flag};              
    struct img_capture_thread_param  img_capture_param = 
    {
      "img capture",
      &img_fifo,
      &rgb_img_fifo,
      &greyscale_img_fifo,
      &bw_img_fifo,
      &reduced_img_fifo,
      &hist_fifo,
      &egg_fifo,
      &image,
      IMG_RAW,
      IMG_DATA,
      RGB_IMG_raw,
      GREYSCALE_IMG_raw,
      BW_IMG_raw,
      REDUCED_IMG_raw,
      &quit_flag
    };     

    struct img_process_thread_param       rgb_param =
    {
      "rgb img",
      &rgb_img_fifo,
      RGB_IMG_raw,
      RGB_IMG_data,
      320,
      240,
      &quit_flag,
      true
    };
    struct img_process_thread_param       greyscale_param=
    {
      "greyscale img",
      &greyscale_img_fifo,
      GREYSCALE_IMG_raw,
      GREYSCALE_IMG_data,
      320,
      240,
      &quit_flag,
      true
    } ;
    struct img_process_thread_param       bw_param=
    {
      "black and white img",
      &bw_img_fifo,
      BW_IMG_raw,
      BW_IMG_data,
      320,
      240,
      &quit_flag,
      true
    };

    struct reduced_img_thread_param reduced_param=
    {
      "reduced img",
      &reduced_img_fifo,
      &dir_fifo,
      REDUCED_IMG_raw,
      REDUCED_IMG_data,
      32,
      24,
      &quit_flag,
      true
    };

    struct img_process_thread_param hist_param = {
      "histogram thread",
      &hist_fifo,
      RGB_IMG_raw,
      RGB_IMG_data,
      320,
      240,
      &quit_flag,
      true
    };
    struct io_peripherals *io;
    io = import_registers();
    if (io != NULL)
    {
        /* print where the I/O memory was actually mapped to */
        printf( "mem at 0x%8.8X\n", (unsigned int*)io );

        set_gpio_context(io->gpio);  // save gpio context
        signal(SIGINT, sigint_handler);

        enable_pwm_clock(io->cm, io->pwm);
        set_gpio(io);
        /*enables pwm function*/
        enable_pwm(io);
       

        motor_direction_param.gpio   = io->gpio;
        motor_speed_param.gpio       = io->gpio;
        motor_speed_param.pwm        = io->pwm;   
        IR_sensor_param.gpio         = io->gpio;

        /* set initial direction  - stop */
        GPIO_CLR(io->gpio, 22); 
        GPIO_CLR(io->gpio, 23);
        GPIO_CLR(io->gpio, 05);
        GPIO_CLR(io->gpio, 06);
        //start the gui thread
        draw_bitmap_start(argc,argv);

        // Create three threads td, ts, tk, and tc, and run them in parallel
        pthread_create(&ts, NULL, Motor_Speed_Thread, (void *)&motor_speed_param);
        pthread_create(&td, NULL, Motor_Direction_Thread, (void *)&motor_direction_param);
        pthread_create(&tk, NULL, KeyRead,   (void *)&key_param);
        pthread_create(&tc, NULL, Motor_Control,   (void *)&con_param);
        pthread_create(&ti, NULL, IR_Sensor, (void *)&IR_sensor_param);
        pthread_create(&tp,NULL,  video_capture, (void* )&img_capture_param);
        pthread_create(&t_rbg, NULL, video_with_cross, (void *)&rgb_param);
        pthread_create(&t_grey,NULL, greyscale_video, (void*)&greyscale_param);
        pthread_create(&t_bw,NULL, black_and_white, (void*)&bw_param);
        pthread_create(&t_reduced,NULL,reduced_video, (void*)&reduced_param);
        pthread_create(&t_hist,NULL, video_histogram, (void*)&hist_param );
        pthread_create(&t_egg, NULL, egg_detector, (void * )&egg_param);

        // Wait to finish ts, td, tc, and tk threads
        pthread_join(ts, NULL);
        pthread_join(td, NULL);
        pthread_join(tk, NULL);
        pthread_join(tc, NULL);
        pthread_join(ti, NULL);
        pthread_join(tp, NULL);
        pthread_join(t_rbg,NULL);
        pthread_join(t_grey,NULL);
        pthread_join(t_bw,NULL);
        pthread_join(t_reduced,NULL);
        pthread_join(t_hist,NULL);
        pthread_join(t_egg,NULL);
        
        printf("trying to free buffers \n");
        free(RGB_IMG_raw);
        printf("rgb buffer freed succesfully \n");
        free(IMG_RAW);
        printf("original image freed succesfully \n");
        free(GREYSCALE_IMG_raw);
        printf("greyscale image buffer freed succesfully \n");
        free(BW_IMG_raw);
        printf("bw image buffer freed successsfully \n");
        free(REDUCED_IMG_raw);
        printf("reduced img buffer freed successfully");
    

        /* main task finished  */

        /* clean the GPIO pins */
        io->gpio->GPFSEL2.field.FSEL2 = GPFSEL_INPUT;
        io->gpio->GPFSEL2.field.FSEL3 = GPFSEL_INPUT;


        io->gpio->GPFSEL0.field.FSEL5 = GPFSEL_INPUT;
        io->gpio->GPFSEL0.field.FSEL6 = GPFSEL_INPUT;

        io->gpio->GPFSEL1.field.FSEL2 = GPFSEL_INPUT;
        io->gpio->GPFSEL1.field.FSEL3 = GPFSEL_OUTPUT;
  }
  else
  {
    ; /* warning message already issued */
  }

  printf( "main function done\n" );

  return 0;
}

