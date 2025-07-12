#include "hw6.h"
int main(int argc, char * argv[] )
{
  // Open UART for communication
    int uart_fd = open_uart("/dev/ttyUSB0", B9600);  // Same baudrate as in the Python script

    // Variables for angles and XYZ positions
    int angles[3];
    //int xyz[3]; // Uncomment if you want to use XYZ positions

    /* 
     * Initialize angles to [90, 135, 75] (RESET POSITION)
     * [0] spins arm horizontally, 90 is front; below is turn right and above is turn left
     * [1] moves arm back and forth, 90 is upright; below is forward and above is backward
     * [2] moves arm up and down, 90 is level; below is up and above is down
     */
    printf("Setting angles to [90, 135, 75]\n\n");
    angles[0] = SPIN_RESET;
    angles[1] = BACK_FORTH_RESET;
    angles[2] = UP_DOWN_RESET;
    set_angles(uart_fd, angles, ARM_TIMEOUT);
    
    /*
     * Initialize claw to CLAW_OPEN (open position)
     * CLAW_OPEN is the open position for the claw, and CLAW_CLOSE is the closed position
     * How much the claw opens or closes can be modified by changing the values of CLAW_OPEN and CLAW_CLOSE
     * For now, CLAW_OPEN is set to 500 and CLAW_CLOSE is set to 800
     */
    printf("Setting claw to CLAW_OPEN\n\n");
    int claw_pos = CLAW_OPEN;
    set_claw(uart_fd, claw_pos, ARM_CLAW_TIMEOUT);

    /*
     * Initialize PWM servo to 180 degrees
     * This is equivalent to setting the servo to the parallel position with the claw motor facing backwards
     * Angle is turned clockwise
     * For example, servo will be turned 30 degrees clockwise from 0 (claw motor facing forward) if 30 degrees is set
     */
    printf("Setting PWM servo\n");
    set_pwmservo(uart_fd, PWM_SERVO_RESET, PWM_SERVO_TIMEOUT);
    sleep(1); // Delay to assure reset position is reached

    pthread_t tk; //key input thread
    pthread_t tc; //motor control thread
    pthread_t tmc;
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
    pthread_t t_sc;
    pthread_t tarm; // thread for arm control
    pthread_t tpwm; // thread for PWM servo control
    pthread_t tclaw; // thread for claw control
    pthread_t tedge;
    pthread_t t_arm_cam;

    bool quit_flag = false;

    struct fifo_t motor_control_fifo = {{}, 0, 0, PTHREAD_MUTEX_INITIALIZER};
    struct fifo_t key_fifo   = {{}, 0, 0, PTHREAD_MUTEX_INITIALIZER};
    struct fifo_t speed_fifo   = {{}, 0, 0, PTHREAD_MUTEX_INITIALIZER};
    struct fifo_t dir_fifo = {{}, 0, 0, PTHREAD_MUTEX_INITIALIZER};
    struct fifo_t IR_sensor_fifo = {{},0,0,PTHREAD_MUTEX_INITIALIZER};
    struct fifo_t control_fifo = {{},0,0,PTHREAD_MUTEX_INITIALIZER};
    struct fifo_t img_fifo = {{},0,0,PTHREAD_MUTEX_INITIALIZER};
    struct fifo_t rgb_img_fifo = {{},0,0,PTHREAD_MUTEX_INITIALIZER};
    struct fifo_t greyscale_img_fifo = {{},0,0,PTHREAD_MUTEX_INITIALIZER};
    struct fifo_t bw_img_fifo = {{},0,0,PTHREAD_MUTEX_INITIALIZER};
    struct fifo_t reduced_img_fifo = {{},0,0,PTHREAD_MUTEX_INITIALIZER};
    struct fifo_t hist_fifo = {{},0,0,PTHREAD_MUTEX_INITIALIZER};
    struct fifo_t egg_fifo = {{},0,0,PTHREAD_MUTEX_INITIALIZER};
    struct fifo_t single_channel_fifo = {{},0,0,PTHREAD_MUTEX_INITIALIZER};
    struct fifo_t arm_fifo = {{},0,0,PTHREAD_MUTEX_INITIALIZER}; // FIFO for arm control commands
    struct fifo_t pwm_servo_fifo = {{},0,0,PTHREAD_MUTEX_INITIALIZER}; // FIFO for PWM servo control commands
    struct fifo_t claw_fifo = {{},0,0,PTHREAD_MUTEX_INITIALIZER}; // FIFO for claw control commands
    struct fifo_t edge_fifo = {{},0,0,PTHREAD_MUTEX_INITIALIZER};

    static struct image_t       image;
    static struct image_t       image1; //arm camera feed
    unsigned char               * IMG_ARM_RAW1;//arm image
    struct pixel_format_RGB     * IMG_ARM_DATA1;
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
    
    IMG_ARM_RAW1 = (unsigned char *)malloc(IMAGE_SIZE+1);
    IMG_RAW = (unsigned char *)malloc(IMAGE_SIZE+1);
    RGB_IMG_raw = (unsigned char *)malloc(IMAGE_SIZE+1);
    GREYSCALE_IMG_raw = (unsigned char *)malloc(IMAGE_SIZE+1);
    BW_IMG_raw = (unsigned char *)malloc(IMAGE_SIZE+1);
    REDUCED_IMG_raw = (unsigned char *)malloc(IMAGE_SIZE+1);

    IMG_ARM_DATA1 = (struct pixel_format_RGB*)IMG_RAW;
    RGB_IMG_data = (struct pixel_format_RGB *)RGB_IMG_raw;
    GREYSCALE_IMG_data = (struct pixel_format_RGB*)GREYSCALE_IMG_raw;
    BW_IMG_data = (struct pixel_format_RGB*)BW_IMG_raw;
    REDUCED_IMG_data = (struct pixel_format_RGB*)REDUCED_IMG_raw;

    // struct img_process_thread_param edge ={
    //   "edge detection",
    //   &edge_fifo,
    //   &dir_fifo,
      
    // };

 


    /*initialize the parameter of motor control thread*/
    struct motor_control_thread_param motor_ctl_param = {
      "motor control",
      &motor_control_fifo,
      &speed_fifo,
      &dir_fifo,
      15,
      20,
      &quit_flag
    };
    /*intial the parameter of motor_speed_thread    */
    struct motor_speed_thread_param motor_speed_param= 
    {"speed", NULL,NULL,12,13,&speed_fifo,&quit_flag};
    /*intial the parameter of motor_direction_thread    */
    struct motor_direction_thread_param motor_direction_param = 
    {"direction", NULL,5,6,22,23,&dir_fifo,&quit_flag};
    /*intial the parameter of key_input_thread   q */
    struct key_thread_param key_param = {
      "key",
      &key_fifo,    
      &quit_flag};
    /*intial the parameter of motor_control_thread    */
    struct control_thread_param     con_param            
     = {"control", 
      &key_fifo,
       &control_fifo, 
       &IR_sensor_fifo,
       &img_fifo,
       &motor_control_fifo,
       &arm_fifo, // FIFO for arm control commands
       &pwm_servo_fifo, // FIFO for PWM servo control commands
       &claw_fifo, // FIFO for claw control commands
       &quit_flag, 
       true};

    /*initial the parameters of the arm*/
    struct arm_thread_param arm_param = {
      "arm",
      &arm_fifo,
      uart_fd, // pass the UART file descriptor to the arm control thread
      &quit_flag
    };
    /*initial the parameters of the pwm servo*/
    struct arm_thread_param pwm_param = {
      "pwm_servo",
      &pwm_servo_fifo,
      uart_fd, // pass the UART file descriptor to the PWM servo control thread
      &quit_flag
    };
    /*initial the parameters of the claw*/
    struct arm_thread_param claw_param = {
      "claw",
      &claw_fifo,
      uart_fd, // pass the UART file descriptor to the claw control thread
      &quit_flag
    };
    /*intialize the parameter of IR_sensor_thread
      - initialize to line tracing mode
    */
    struct IR_Sensor_param                IR_sensor_param      =  {"IR Senssor", &IR_sensor_fifo,&dir_fifo,NULL,24,25,&quit_flag};
    struct egg_detector_thread_param      egg_param = {
        "egg datection", 
        &egg_fifo, 
        &motor_control_fifo, 
        &key_fifo,
        BW_IMG_raw,
        BW_IMG_data,
        IMG_ARM_RAW1,
        IMG_ARM_DATA1,
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
      &image1,
      IMG_ARM_RAW1,
      RGB_IMG_raw,
      GREYSCALE_IMG_raw,
      BW_IMG_raw,
      REDUCED_IMG_raw,
      &quit_flag
    };     
    struct img_process_thread_param       single_channel_param = {
      "single channel",
      &single_channel_fifo,
      RGB_IMG_raw,
      RGB_IMG_data,
      320,
      240,
      &quit_flag,
      true
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
      &motor_control_fifo,
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
        //printf( "mem at 0x%8.8X\n", (unsigned int*)io );

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

        //temp
        // io->gpio->GPFSEL1.field.FSEL3 = GPFSEL_INPUT;
        // io->gpio->GPFSEL1.field.FSEL9 = GPFSEL_ALTERNATE_FUNCTION5;
        // io->pwm->DAT1 = 1; // testing
        // io->pwm->DAT2 = 1; // testing
        // printf ("pwm val: %d\n", io->pwm->DAT1);
        // printf ("pwm val: %d\n", io->pwm->DAT2);
        // sleep(5);
        // io->pwm->DAT1 = 50; // start pwm
        // io->pwm->DAT2 = 50; // start pwm
        // printf ("pwm val: %d\n", io->pwm->DAT1);
        // printf ("pwm val: %d\n", io->pwm->DAT2);
        // sleep(5);
        // io->pwm->DAT1 = 100; // stop pwm
        // io->pwm->DAT2 = 100; // stop pwm
        // printf ("pwm val: %d\n", io->pwm->DAT1);
        // printf ("pwm val: %d\n", io->pwm->DAT2);
        // sleep(5);
        // io->pwm->DAT1 = 1; // testing
        // io->pwm->DAT2 = 1; // testing
        // io->gpio->GPFSEL1.field.FSEL9 = GPFSEL_INPUT;
        // io->gpio->GPFSEL1.field.FSEL3 = GPFSEL_ALTERNATE_FUNCTION0;

        //start the gui thread
        draw_bitmap_start(argc,argv);

        // Create three threads td, ts, tk, and tc, and run them in parallel
        pthread_create(&ts, NULL, Motor_Speed_Thread, (void *)&motor_speed_param);
        pthread_create(&td, NULL, Motor_Direction_Thread, (void *)&motor_direction_param);
        pthread_create(&tk, NULL, KeyRead,   (void *)&key_param);
        pthread_create(&tc, NULL, Control,   (void *)&con_param);
        pthread_create(&ti, NULL, IR_Sensor, (void *)&IR_sensor_param);
        pthread_create(&tp,NULL,  video_capture, (void* )&img_capture_param);
        pthread_create(&t_rbg, NULL, video_with_cross, (void *)&rgb_param);
        // pthread_create(&t_grey,NULL, greyscale_video, (void*)&greyscale_param);
        // pthread_create(&t_bw,NULL, black_and_white, (void*)&bw_param);
        // pthread_create(&t_reduced,NULL,reduced_video, (void*)&reduced_param);
        pthread_create(&t_hist,NULL, video_histogram, (void*)&hist_param );
        pthread_create(&t_egg, NULL, egg_detector, (void * )&egg_param);
        pthread_create(&tmc, NULL,Motor_Control, (void*)&motor_ctl_param);
        pthread_create(&t_sc,NULL, single_channel, (void*)&single_channel_param);
        pthread_create(&tarm, NULL, Arm_Thread, (void *)&arm_param);
        pthread_create(&tclaw, NULL, Claw_Thread, (void *)&claw_param);
        pthread_create(&tpwm, NULL, PWM_Servo_Thread, (void *)&pwm_param);
        // Wait to finish ts, td, tc, and tk threads
        pthread_join(ts, NULL);
        pthread_join(td, NULL);
        pthread_join(tk, NULL);
        pthread_join(tc, NULL);
        pthread_join(ti, NULL);
        pthread_join(tp, NULL);
        pthread_join(t_rbg,NULL);
        // pthread_join(t_grey,NULL);
        // pthread_join(t_bw,NULL);
        // pthread_join(t_reduced,NULL);
        pthread_join(t_hist,NULL);
        pthread_join(t_egg,NULL);
        pthread_join(tmc,NULL);
        pthread_join(t_sc,NULL);
        pthread_join(tarm, NULL);
        pthread_join(tclaw, NULL);
        pthread_join(tpwm, NULL);
        
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
/* 
     * Initialize angles to [90, 135, 75] (RESET POSITION)
     * [0] spins arm horizontally, 90 is front; below is turn right and above is turn left
     * [1] moves arm back and forth, 90 is upright; below is forward and above is backward
     * [2] moves arm up and down, 90 is level; below is up and above is down
     */
    printf("Setting angles to [90, 135, 75]\n\n");
    angles[0] = SPIN_RESET;
    angles[1] = BACK_FORTH_RESET;
    angles[2] = UP_DOWN_RESET;
    set_angles(uart_fd, angles, ARM_TIMEOUT);
    
    /*
     * Initialize claw to CLAW_OPEN (open position)
     * CLAW_OPEN is the open position for the claw, and CLAW_CLOSE is the closed position
     * How much the claw opens or closes can be modified by changing the values of CLAW_OPEN and CLAW_CLOSE
     * For now, CLAW_OPEN is set to 500 and CLAW_CLOSE is set to 800
     */
    printf("Setting claw to CLAW_OPEN\n\n");
    claw_pos = CLAW_OPEN;
    set_claw(uart_fd, claw_pos, ARM_CLAW_TIMEOUT);

    /*
     * Initialize PWM servo to 180 degrees
     * This is equivalent to setting the servo to the parallel position with the claw motor facing backwards
     * Angle is turned clockwise
     * For example, servo will be turned 30 degrees clockwise from 0 (claw motor facing forward) if 30 degrees is set
     */
    printf("Setting PWM servo\n");
    set_pwmservo(uart_fd, PWM_SERVO_RESET, PWM_SERVO_TIMEOUT);
    sleep(1); // Delay to assure reset position is reached
  printf( "main function done\n" );

  return 0;
}

