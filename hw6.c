
#include "hw6.h"

void *KeyRead(void * arg)
{
  struct  key_thread_param * param = (struct key_thread_param *)arg;
  struct  thread_command cmd = {0, 0};
  int     keyhit1 = 0;
  int     keyhit2 = 0;
  struct  timespec  timer_state; 
             // used to wake up every 10ms with wait_period() function, 
             // similar to interrupt occuring every 10ms

  // start 10ms timed wait, ie. set interrupt
  bool print = true;
  wait_period_initialize( &timer_state );
  wait_period( &timer_state, 10u ); /* 10ms */
  
  while (!*(param->quit_flag))
  {
    
    keyhit1 = get_pressed_key(); // read once every 10ms
    
    if ( keyhit1 != -1)
    {
      print = true;
      printf("\n");
      switch (keyhit1)
      {
        case 10:      // 'Enter' key hit => no action
        {
                      // nothing placed on param->key_fifo
        }
        break;
        case 'm':
        {
          usleep(100000);
          keyhit2 = get_pressed_key();
          cmd.command  =  keyhit1;
          cmd.argument = keyhit2;
          if (!(FIFO_FULL( param->key_fifo )))
          {FIFO_INSERT( param->key_fifo, cmd );}
          else {printf( "key_fifo queue full\n" );}
          break;
        }
        case 113:  // 'q' for quit
        {
          cmd.command = 113;
          cmd.argument = 0;
          if (!(FIFO_FULL( param->key_fifo )))
          {FIFO_INSERT( param->key_fifo, cmd );}
          else {printf( "key_fifo queue full\n" );}
          *param->quit_flag = true;
          break;
        }
        

        default:
        {
          cmd.command  = keyhit1;  // other key hit
          cmd.argument = keyhit2;
          if (!(FIFO_FULL( param->key_fifo )))
          {FIFO_INSERT( param->key_fifo, cmd );}
          else {printf( "key_fifo queue full\n" );}
        }

      }
    }
    if(print)
    {
        printf("<hw5>");
        print = false;
    }
    
    wait_period( &timer_state, 10u ); /* 10ms */

  }

  printf( "KeyRead function done\n" );

  return NULL;

}
/*
*/
void *IR_Sensor(void* arg)
{
  struct IR_Sensor_param *param = (struct IR_Sensor_param *)arg;
  bool pause_thread = false;
  struct  timespec  timer_state; 
             // used to wake up every 10ms with wait_period() function, 
             // similar to interrupt occuring every 10ms
             
  struct thread_command cmd = {0,0};
  struct thread_command cmd2 = {0,0};
  // start 10ms timed wait, ie. set interrupt
  wait_period_initialize( &timer_state );
  wait_period( &timer_state, 10u ); /* 10ms */

  while(!(*param->quit_flag))
  {
    if(!pause_thread)
    {
      int left_val = GPIO_READ(param->gpio,param->pin_1);
      int right_val = GPIO_READ(param->gpio,param->pin_2);
      if(left_val != 0){
        cmd.command = 'd';
        cmd.argument = 0;
        if(!FIFO_FULL(param->dir_fifo)){FIFO_INSERT(param->dir_fifo,cmd);}
        usleep(250000);

      }
      if(right_val != 0){
        cmd.command = 'a';
        cmd.argument = 0;
        if(!FIFO_FULL(param->dir_fifo)){FIFO_INSERT(param->dir_fifo,cmd);}
        usleep(250000);
      }
    }
    //check for mode switching
    if(!FIFO_EMPTY(param->IR_sensor_fifo))
    {
      FIFO_REMOVE(param->IR_sensor_fifo,&cmd2);
      printf( "\n %s= %c\n", param->name, cmd2.command);
      switch(cmd2.command)
      {
        case 'w':{pause_thread = false;break;}
        case 's':{pause_thread = true; break;}
        default:{ printf("invalid mode \n");}
      }
    }

    wait_period( &timer_state, 10u );
  }
  printf("IR Sensor thread Quit\n");
  return NULL;
}

void *Motor_Control(void * arg)
{
  struct  motor_control_thread_param * param = (struct motor_control_thread_param *)arg;
  struct  thread_command cmd1 = {0, 0};  // copy of input, cmd from key_thread
  struct  thread_command cmd2 = {0, 0};  // copy of output, cmd to put on LED_thread
  struct  timespec  timer_state; 
             // used to wake up every 10ms with wait_period() function, 
             // similar to interrupt occuring every 10ms
             

  // start 10ms timed wait, ie. set interrupt
  wait_period_initialize( &timer_state );
  wait_period( &timer_state, 10u ); /* 10ms */

/* Pick up incoming key presses every 10ms, 100 times/sec,
*  from the key input FIFO queues, and generate LED on/off commands, 
*  and fill the Red LED command FIFO queue and Green LED command FIFO queue,
*  for the LED on/off control.  
*  A simple and short LED on/off control programming is possible   */

  while (!*(param->quit_flag))
  {
    if (!(FIFO_EMPTY( param->key_fifo )))
    {
      FIFO_REMOVE( param->key_fifo, &cmd1 );  // read once every 10ms
      printf( "\n %s= %c  %c\n", param->name, cmd1.command, cmd1.argument);
      switch (cmd1.command)
      {
        case 'c':
        {
          cmd2.command ='c';
          cmd2.argument = 0;
          if(!FIFO_FULL(param->img_cmd_fifo))
          {
            FIFO_INSERT(param->img_cmd_fifo,cmd2);
          }break;
        }
        case 'v':{
          cmd2.command ='v';
          cmd2.argument = 0;
          if(!FIFO_FULL(param->img_cmd_fifo))
          {
            FIFO_INSERT(param->img_cmd_fifo,cmd2);
          }break;
        }
        case 'b':{
          cmd2.command ='b';
          cmd2.argument = 0;
          if(!FIFO_FULL(param->img_cmd_fifo))
          {
            FIFO_INSERT(param->img_cmd_fifo,cmd2);
          }break;
        }
        case 'n':{
          cmd2.command ='n';
          cmd2.argument = 0;
          if(!FIFO_FULL(param->img_cmd_fifo))
          {
            FIFO_INSERT(param->img_cmd_fifo,cmd2);
          }break;
        }
        case 'w':
        {
          if(param->mode)
          {
            cmd2.command = 'w';
            cmd2.argument = 0;
            if(!FIFO_FULL(param->dir_fifo))
            {FIFO_INSERT(param->dir_fifo,cmd2);}
            break;
          }
          else{
            cmd2.command = 'w';
            cmd2.argument = 0;
            if(!FIFO_FULL(param->IR_sensor_fifo))
            {FIFO_INSERT(param->IR_sensor_fifo,cmd2);}
          }              
        }
        case 'a':{
          if(param->mode) { 
            cmd2.command = 'a';
            cmd2.argument = 0;
            if(!FIFO_FULL(param->dir_fifo))
            {FIFO_INSERT(param->dir_fifo,cmd2);}
          }
            break;
        }
        case 's':{
            cmd2.command = 's';
            cmd2.argument = 0;
            if(param->mode){
              if(!FIFO_FULL(param->dir_fifo)){
                {FIFO_INSERT(param->dir_fifo,cmd2);}
                break;
              }
            }
            else{
              if(!FIFO_FULL(param->IR_sensor_fifo)){
                {FIFO_INSERT(param->IR_sensor_fifo,cmd2);}
                break;
              }
            } 
          }
          case 'd':
          {
            if(param->mode)
            {  
              cmd2.command = 'd';
              cmd2.argument = 0;
              if(!FIFO_FULL(param->dir_fifo))
              {FIFO_INSERT(param->dir_fifo,cmd2);}
            }
              break;
          }
          case 'i':
          {
              cmd2.command = 'i';
              cmd2.argument = 0;
              if(!FIFO_FULL(param->speed_fifo))
              {FIFO_INSERT(param->speed_fifo,cmd2);}
              break;
          }
          case 'j':
          {
            cmd2.command = 'j';
            cmd2.argument = 0;
            if(!FIFO_FULL(param->speed_fifo))
            {FIFO_INSERT(param->speed_fifo,cmd2);}
            break;
          }
          case 'x':
          { 
            cmd2.command = 'x';
            cmd2.argument = 0;
            if(!FIFO_FULL(param->dir_fifo))
            {FIFO_INSERT(param->dir_fifo,cmd2);}
            break;
          }
          case 'q':
          {
              cmd2.command = 'q';
              cmd2.argument = 0;
              *param->quit_flag = true;
              if(! (FIFO_FULL(param->dir_fifo)) && ! (FIFO_FULL(param->speed_fifo)))
              {
                FIFO_INSERT(param->dir_fifo,cmd2);
                FIFO_INSERT(param->speed_fifo,cmd2);
              }
              break;
          }  
          case 'm':
          {
            //pause the car first when switching mode
            cmd2.command = 's';
            cmd2.argument= 0;
            if(!FIFO_FULL(param->dir_fifo))
            {FIFO_INSERT(param->dir_fifo,cmd2);}
            cmd2.command = 'w';
            cmd2.argument = 0;
            if(!FIFO_FULL(param->dir_fifo))
            {FIFO_INSERT(param->dir_fifo,cmd2);}
            
            if(cmd1.argument == '1'){
              param->mode = true; 
              cmd2.command = 's';
              cmd2.argument = 0;
              if(!FIFO_FULL(param->IR_sensor_fifo))
              {FIFO_INSERT(param->IR_sensor_fifo,cmd2);}
              printf("switched to mode 1 \n");
              break;
            }
            else if(cmd1.argument == '2'){
              
              param->mode = false;
              cmd2.command = 'w';
              cmd2.argument = 0;
              if(!FIFO_FULL(param->IR_sensor_fifo))
              {FIFO_INSERT(param->IR_sensor_fifo,cmd2);}
              printf("switched to mode 2 \n");
              break;
              
            }
            else{printf("invalid mode \n");}
          }
          default: //if no command ente
          {
              printf("invalid command \n");
          }
      }
    }
    wait_period( &timer_state, 10u ); /* 10ms */
  }

  printf( "Control function done\n" );

  return NULL;

}
void *Motor_Speed_Thread(void * args)
{
  struct motor_speed_thread_param * param = (struct motor_speed_thread_param * )args;
  struct  thread_command cmd = {0, 0};
  struct  timespec  timer_state; 
             // used to wake up every 10ms with wait_period() function,
             // similar to interrupt occuring every 10ms

  // start 10ms timed wait
  wait_period_initialize( &timer_state );
  wait_period( &timer_state, 10u ); /* 10ms */
  while(!(*(param->quit_flag)))
  {
    if(!FIFO_EMPTY(param->speed_fifo))
    {
      FIFO_REMOVE(param->speed_fifo, &cmd);
      printf( " %s= %d  %c\n", param->name, cmd.command, cmd.command);

      switch(cmd.command)
      {
        case 'i':
          if(param->pwm_val < 100)
          {param->pwm_val = param->pwm_val +5;}
          else{printf("Motor at is already at maximum speed\n");}
          break;
        case 'j':
          if(param->pwm_val >40)
          {param->pwm_val = param->pwm_val -5;}
          else{printf("motor is already at minimum speed \n");}
          break;
        default:
          printf("invalid speed command\n");
          break;
      }
      if(param->pwm_val >=40)
      {
        param->pwm->DAT1 = param->pwm_val;
        param->pwm->DAT2 = param->pwm_val;
      }
      else{
        param->pwm->DAT1 = 0;
        param->pwm->DAT2 = 0;
      }
      
    }
    
  }
  printf("speed thread exit \n");
  return NULL;
}

void *Motor_Direction_Thread(void * arg)
{
  struct  motor_direction_thread_param * param = (struct motor_direction_thread_param *)arg;
  struct  thread_command cmd = {0, 0};
  struct  timespec  timer_state; 
             // used to wake up every 10ms with wait_period() function,
             // similar to interrupt occuring every 10ms

  // start 10ms timed wait
  wait_period_initialize( &timer_state );
  wait_period( &timer_state, 10u ); /* 10ms */

  while(! (*param->quit_flag))
  {
    if(!(FIFO_EMPTY(param->dir_fifo)))
    {
      FIFO_REMOVE(param->dir_fifo,&cmd);
      printf( " %s= %d  %c\n", param->name, cmd.command, cmd.command);

      switch(cmd.command)
      {
        case 'w':{
          if(param->state == 'x')
          {
            GPIO_SET(param->gpio,param->pin_1);
            GPIO_SET(param->gpio,param->pin_2);
            GPIO_SET(param->gpio,param->pin_3);
            GPIO_SET(param->gpio,param->pin_4);
            usleep(100000); //pause for 0.1sec
          }
          param->forward= 'w'; 
          param->state = 'w';
          
          break;
        }
        case 'a':
        {
          param->state = 'a';
          break;
        }
        case 's':
        {
          param->state = 's';
          param->forward = 's';
          break;
        }
        case 'd':
        {
          param->state = 'd';
          break;
        }
        case 'x':
        {
          if(param->state == 'w')
          {
            GPIO_CLR(param->gpio,param->pin_1);
            GPIO_CLR(param->gpio,param->pin_2);
            GPIO_CLR(param->gpio,param->pin_3);
            GPIO_CLR(param->gpio,param->pin_4);
            usleep(100000); //pause for 0.1sec
          }
          param->forward = 'x';
          param->state = 'x';
          break;
        }
        default:
        {
          printf("invalid direction command\n");
        }

      }
    }
    switch(param->state)
    {
      case 'w':{
        GPIO_SET(param->gpio,param->pin_1);
        GPIO_CLR(param->gpio,param->pin_2);
        GPIO_SET(param->gpio,param->pin_3);
        GPIO_CLR(param->gpio,param->pin_4);
        break;
      }
      case 'a':{
        GPIO_SET(param->gpio,param->pin_1);
        GPIO_CLR(param->gpio,param->pin_2);
        GPIO_CLR(param->gpio,param->pin_3);
        GPIO_SET(param->gpio,param->pin_4);
        usleep(1500000);
        param->state = param->forward;
        break;
      }
      case 's':{
        GPIO_CLR(param->gpio,param->pin_1);
        GPIO_CLR(param->gpio,param->pin_2);
        GPIO_CLR(param->gpio,param->pin_3);
        GPIO_CLR(param->gpio,param->pin_4);
        break;
      }
      case 'd':{
        GPIO_CLR(param->gpio,param->pin_1);
        GPIO_SET(param->gpio,param->pin_2);
        GPIO_SET(param->gpio,param->pin_3);
        GPIO_CLR(param->gpio,param->pin_4);
        usleep(250000);
        param->state = param->forward;
        break;
      }
      case 'x':{
        GPIO_CLR(param->gpio,param->pin_1);
        GPIO_SET(param->gpio,param->pin_2);
        GPIO_CLR(param->gpio,param->pin_3);
        GPIO_SET(param->gpio,param->pin_4);
        break;
      }
    }
    
    wait_period( &timer_state, 10u ); 
  }
  return NULL;
}
void set_gpio_context(volatile struct gpio_register * gpio) {
    static volatile struct gpio_register * saved_gpio = NULL;
    if (gpio != NULL) {
        saved_gpio = gpio;  // set once
    } else if (saved_gpio != NULL) {
        GPIO_CLR(gpio,22);
        GPIO_CLR(gpio,23);  
        GPIO_CLR(gpio,12);
        GPIO_CLR(gpio,13); 
        GPIO_CLR(gpio,05);
        GPIO_CLR(gpio,06); 

        gpio->GPFSEL0.field.FSEL2 = GPFSEL_INPUT;
        gpio->GPFSEL0.field.FSEL3 = GPFSEL_INPUT;
        gpio->GPFSEL2.field.FSEL2 = GPFSEL_INPUT; //gpio  22
        gpio->GPFSEL2.field.FSEL3 = GPFSEL_INPUT;  //gpio 23
        gpio->GPFSEL3.field.FSEL5 = GPFSEL_INPUT; //gpio  22
        gpio->GPFSEL3.field.FSEL6 = GPFSEL_INPUT;
        exit(0);
    }
}
void sigint_handler(int signum){
    set_gpio_context(NULL);
}

void *video_capture(void * arg){
  struct img_capture_thread_param *param = (struct img_capture_thread_param*)arg;
  printf("%s thread starting\n", param->name);
  struct video_interface_handle_t *         handle_video;
  struct draw_bitmap_multiwindow_handle_t * handle_GUI_RGB = NULL;

  handle_video = video_interface_open( "/dev/video0" );
  struct  timespec  timer_state; 
             // used to wake up every 10ms with wait_period() function,
             // similar to interrupt occuring every 10ms

  // start 10ms timed wait
  wait_period_initialize( &timer_state );
  wait_period( &timer_state, 100u ); /* 500 ms */
  video_interface_print_modes( handle_video);
  if (video_interface_set_mode_auto( handle_video ))
  {
    int scaled_width      = handle_video->configured_width/SCALE_REDUCTION_PER_AXIS;
    int scaled_height     = handle_video->configured_height/SCALE_REDUCTION_PER_AXIS;
    printf("img size = %d x %d\n", handle_video->configured_width,handle_video->configured_height);
    printf("original img size = %d x %d\n", scaled_width,scaled_height);
    printf("size of image_t %d\n", IMAGE_SIZE);
    // param->img_raw = (unsigned char *)malloc(IMAGE_SIZE);
    // param->img_data = (struct pixel_format_RGB*)param->img_raw;
    int argc = 0;
    char *argv[3];
    
    while(! *param->quit_flag)
    {
      if (video_interface_get_image(handle_video, param->image))
      {
      
        scale_image_data(
          (struct pixel_format_RGB *)param->image,
          handle_video->configured_height,
          handle_video->configured_width,
          param->img_data,
          SCALE_REDUCTION_PER_AXIS,
          SCALE_REDUCTION_PER_AXIS );
       
        //draw_bitmap_display(handle_GUI_RGB, param->img_data);
      }
      else
      {
        printf("did not get image \n");
      }
      wait_period(&timer_state, 300u);
      struct thread_command cmd1 = {0,0};
      struct thread_command cmd2 = {0,0};
      if(!FIFO_EMPTY(param->img_cmd_fifo))
      {
        FIFO_REMOVE(param->img_cmd_fifo,&cmd1);
        printf( "\n %s= %c  %c\n", param->name, cmd1.command, cmd1.argument);
        switch(cmd1.command)
        {
          
          case 'c':{
            cmd2 = cmd1;
            if(!FIFO_FULL(param->rgb_cmd_fifo))
            {
              FIFO_INSERT(param->rgb_cmd_fifo,cmd1);
            }
            break;
          }
          case 'v':{
            cmd2 = cmd1;
            if(!FIFO_FULL(param->greyscale_cmd_fifo))
            {
              FIFO_INSERT(param->greyscale_cmd_fifo,cmd1);
            }
            break;
          }
          case 'b':{
            cmd2 = cmd1;
            if(!FIFO_FULL(param->bw_cmd_fifo))
            {
              FIFO_INSERT(param->bw_cmd_fifo,cmd1);
            }
            break;
          }
          case 'n':{
            cmd2 = cmd1;
            if(!FIFO_FULL(param->reduced_cmd_fifo))
            {
              FIFO_INSERT(param->reduced_cmd_fifo,cmd1);
            }
            break;
          }
          default:
        }
      }
    }

  }
  else
  {
    printf( "failed to configure\n" );
  }
  printf("video capture thread exit\n");
  return NULL;
}

void *video_with_cross(void * arg){
  
  struct img_process_thread_param *param = (struct img_process_thread_param*)arg;
  printf("%s thread started \n",param->name);
  struct draw_bitmap_multiwindow_handle_t *handle = draw_bitmap_create_window(param->width,param->height);
  struct thread_command cmd = {0,0};
  struct  timespec  timer_state; 
  wait_period_initialize( &timer_state );
  wait_period( &timer_state, 100u ); /* 500 ms */

  while(!(*param->quit_flag))
  {
    
    memcpy(param->RGB_IMG_raw,param->ORIG_IMG_raw, IMAGE_SIZE);
    
    
    //draw cross
    for(size_t i= 160; i< IMAGE_SIZE/3; i+=320)
    {
      param->RGB_IMG_data[i].R = 0;
      param->RGB_IMG_data[i].G = 255;
      param->RGB_IMG_data[i].B = 0;
    }
    for(size_t i = 320*120; i<320*121;i++)
    {
      param->RGB_IMG_data[i].R = 0;
      param->RGB_IMG_data[i].G = 255;
      param->RGB_IMG_data[i].B=0;
    }

    if(!(FIFO_EMPTY(param->img_cmd_fifo)))
    {
      FIFO_REMOVE(param->img_cmd_fifo,&cmd);
      printf( "\n %s= %c  %c\n", param->name, cmd.command, cmd.argument);
      switch(cmd.command)
      {
        case'c':{
          if(handle == NULL)
          {
            handle = draw_bitmap_create_window(param->width,param->height);
          }
          else{
            draw_bitmap_close_window(handle);
            handle = NULL;
          }
        }
        default:{break;}
      }
    }
    
    if(handle != NULL){draw_bitmap_display(handle,param->RGB_IMG_data);}
    
    wait_period(&timer_state, 300u);
  }
  draw_bitmap_close_window(handle);
  printf("%s exited \n", param->name);
  return NULL;
}

void *greyscale_video(void * arg){
  struct img_process_thread_param *param = (struct img_process_thread_param*)arg;
  printf("%s thread started \n",param->name);
  struct draw_bitmap_multiwindow_handle_t *handle = draw_bitmap_create_window(param->width,param->height);

  struct thread_command cmd = {0,0};

  struct  timespec  timer_state; 
  wait_period_initialize( &timer_state );
  wait_period( &timer_state, 100u ); /* 500 ms */

  while(!(*param->quit_flag))
  {
    memcpy(param->RGB_IMG_raw, param->ORIG_IMG_raw, IMAGE_SIZE);
    for(size_t i= 0; i< IMAGE_SIZE/3; i++)
    {
      int avg = (param->RGB_IMG_data[i].R +param->RGB_IMG_data[i].G +param->RGB_IMG_data[i].B)/3;
      param->RGB_IMG_data[i].R = avg;
      param->RGB_IMG_data[i].G = avg;
      param->RGB_IMG_data[i].B = avg;
    }
    if(!(FIFO_EMPTY(param->img_cmd_fifo)))
    {
      FIFO_REMOVE(param->img_cmd_fifo,&cmd);
      switch(cmd.command)
      {
        case'v':{
          if(handle == NULL)
          {
            handle = draw_bitmap_create_window(param->width,param->height);
          }
          else{
            draw_bitmap_close_window(handle);
            handle = NULL;
          }
        }
        default:{break;}
      }
    }
    
    if(handle != NULL){draw_bitmap_display(handle,param->RGB_IMG_data);}
    wait_period(&timer_state, 300u);
  }
  draw_bitmap_close_window(handle);
  handle =NULL;
  printf("%s exited \n",param->name);
  return NULL;
}

// void reduced_video(void * arg){}

void *black_and_white(void * arg){
  struct img_process_thread_param *param = (struct img_process_thread_param*)arg;
  printf("%s thread started \n",param->name);
  struct draw_bitmap_multiwindow_handle_t *handle = draw_bitmap_create_window(param->width,param->height);
  struct thread_command cmd = {0,0};
  struct  timespec  timer_state; 
  wait_period_initialize( &timer_state );
  wait_period( &timer_state, 100u ); /* 500 ms */

  while(!(*param->quit_flag))
  {
    memcpy(param->RGB_IMG_raw, param->ORIG_IMG_raw, IMAGE_SIZE);
    //draw cross
    for(size_t i= 0; i< IMAGE_SIZE/3; i++)
    {
      int avg = (param->RGB_IMG_data[i].R +param->RGB_IMG_data[i].G +param->RGB_IMG_data[i].B)/3;
      if(avg > 128)
      {
        param->RGB_IMG_data[i].R = 255;
        param->RGB_IMG_data[i].G = 255;
        param->RGB_IMG_data[i].B = 255;
      }
      else{
        param->RGB_IMG_data[i].R = 0;
        param->RGB_IMG_data[i].G = 0;
        param->RGB_IMG_data[i].B = 0;
      }
      
    }
    if(!(FIFO_EMPTY(param->img_cmd_fifo)))
    {
      FIFO_REMOVE(param->img_cmd_fifo,&cmd);
      switch(cmd.command)
      {
        case'b':{
          if(handle == NULL)
          {
            handle = draw_bitmap_create_window(param->width,param->height);
          }
          else{
            draw_bitmap_close_window(handle);
            handle = NULL;
          }
        }
        default:{break;}
      }
    }
    
    if(handle != NULL){draw_bitmap_display(handle,param->RGB_IMG_data);}
    wait_period(&timer_state, 100u);
  }
  draw_bitmap_close_window(handle);
  printf("%s exited \n",param->name);
  return NULL;
}

void enable_pwm(struct io_peripherals *io)
{
  io->gpio->GPFSEL1.field.FSEL2 = GPFSEL_ALTERNATE_FUNCTION0;
  io->gpio->GPFSEL1.field.FSEL3 = GPFSEL_ALTERNATE_FUNCTION0;

  io->pwm->RNG1 = PWM_RANGE;     /* the range value, 100 level steps */
  io->pwm->RNG2 = PWM_RANGE;     /* the range value, 100 level steps */
  io->pwm->DAT1 = 1;             /* initial beginning level=1/100=1% */
  io->pwm->DAT2 = 1;             /* initial beginning level=1/100=1% */
  io->pwm->CTL.field.MODE1 = 0;  /* PWM mode */
  io->pwm->CTL.field.MODE2 = 0;  /* PWM mode */
  io->pwm->CTL.field.RPTL1 = 1;  /* not using FIFO, but repeat the last byte anyway */
  io->pwm->CTL.field.RPTL2 = 1;  /* not using FIFO, but repeat the last byte anyway */
  io->pwm->CTL.field.SBIT1 = 0;  /* idle low */
  io->pwm->CTL.field.SBIT2 = 0;  /* idle low */
  io->pwm->CTL.field.POLA1 = 0;  /* non-inverted polarity */
  io->pwm->CTL.field.POLA2 = 0;  /* non-inverted polarity */
  io->pwm->CTL.field.USEF1 = 0;  /* do not use FIFO */
  io->pwm->CTL.field.USEF2 = 0;  /* do not use FIFO */
  io->pwm->CTL.field.MSEN1 = 1;  /* use M/S algorithm, level=pwm->DAT1/PWM_RANGE */
  io->pwm->CTL.field.MSEN2 = 1;  /* use M/S algorithm, level=pwm->DAT2/PWM_RANGE */
  io->pwm->CTL.field.CLRF1 = 1;  /* clear the FIFO, even though it is not used */
  io->pwm->CTL.field.PWEN1 = 1;  /* enable the PWM channel */
  io->pwm->CTL.field.PWEN2 = 1;  /* enable the PWM channel */
        
}
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
    bool quit_flag = false;

    struct fifo_t key_fifo   = {{}, 0, 0, PTHREAD_MUTEX_INITIALIZER};
    struct fifo_t speed_fifo   = {{}, 0, 0, PTHREAD_MUTEX_INITIALIZER};
    struct fifo_t dir_fifo = {{}, 0, 0, PTHREAD_MUTEX_INITIALIZER};
    struct fifo_t IR_sensor_fifo = {{},0,0,PTHREAD_MUTEX_INITIALIZER};
    
    struct fifo_t img_fifo = {{},0,0,PTHREAD_MUTEX_INITIALIZER};
    struct fifo_t rgb_img_fifo = {{},0,0,PTHREAD_MUTEX_INITIALIZER};
    struct fifo_t greyscale_img_fifo = {{},0,0,PTHREAD_MUTEX_INITIALIZER};
    struct fifo_t bw_img_fifo = {{},0,0,PTHREAD_MUTEX_INITIALIZER};
    struct fifo_t reduced_img_fifo = {{},0,0,PTHREAD_MUTEX_INITIALIZER};

    static struct image_t       image;
    unsigned char               * IMG_RAW;
    struct pixel_format_RGB     * IMG_DATA;
    unsigned char               * RGB_IMG_raw;
    struct pixel_format_RGB     * RGB_IMG_data;
    unsigned char               * GREYSCALE_IMG_raw;
    struct pixel_format_RGB     * GREYSCALE_IMG_data;
    unsigned char               * BW_IMG_raw;
    struct pixel_format_RGB     * BW_IMG_data;
    // unsigned char               * REDUCED_IMG_raw;
    // struct pixel_format_RGB     * REDUCED_IMG_data;

    IMG_RAW = (unsigned char *)malloc(IMAGE_SIZE+1);
    // free(IMG_RAW);
    // IMG_RAW = (unsigned char *)malloc(IMAGE_SIZE);
    RGB_IMG_raw = (unsigned char *)malloc(IMAGE_SIZE+1);
    GREYSCALE_IMG_raw = (unsigned char *)malloc(IMAGE_SIZE+1);
    BW_IMG_raw = (unsigned char *)malloc(IMAGE_SIZE+1);
    // REDUCED_IMG_raw = (unsigned char *)malloc(IMAGE_SIZE);

    IMG_DATA = (struct pixel_format_RGB*)IMG_RAW;
    RGB_IMG_data = (struct pixel_format_RGB *)RGB_IMG_raw;
    GREYSCALE_IMG_data = (struct pixel_format_RGB*)GREYSCALE_IMG_raw;
    BW_IMG_data = (struct pixel_format_RGB*)BW_IMG_raw;
    // REDUCED_IMG_data = (struct pixel_format_RGB)REDUCED_IMG_data;
    /*intial the parameter of motor_speed_thread    */
    struct motor_speed_thread_param       motor_speed_param     = {"speed", NULL,NULL,12,13,&speed_fifo,&quit_flag,25};
    /*intial the parameter of motor_direction_thread    */
    struct motor_direction_thread_param   motor_direction_param = {"direction", NULL,5,6,22,23,&dir_fifo,&quit_flag, 's'};
    /*intial the parameter of key_input_thread    */
    struct key_thread_param               key_param             = {"key",     &key_fifo, &quit_flag};
    /*intial the parameter of motor_control_thread    */
    struct motor_control_thread_param     con_param             = {"control", &key_fifo, &speed_fifo, &dir_fifo, &IR_sensor_fifo,&img_fifo,&quit_flag, true};
    /*intialize the parameter of IR_sensor_thread
      - initialize to line tracing mode
    */
    struct IR_Sensor_param                IR_sensor_param      =  {"IR Senssor", &IR_sensor_fifo,&dir_fifo,NULL,24,25,&quit_flag};
    
    struct img_capture_thread_param  img_capture_param = 
    {
      "img capture",
      &img_fifo,
      &rgb_img_fifo,
      &greyscale_img_fifo,
      &bw_img_fifo,
      &reduced_img_fifo,
      &image,
      IMG_RAW,
      IMG_DATA,
      &quit_flag
    };     

    struct img_process_thread_param       rgb_param =
    {
      "rgb img",
      &rgb_img_fifo,
      IMG_RAW,
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
      IMG_RAW,
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
      IMG_RAW,
      BW_IMG_raw,
      BW_IMG_data,
      320,
      240,
      &quit_flag,
      true
    };
    // struct img_process_thread_param       reduced_param
    // {
    //   "reduced img",
    //   &reduced_img_fifo,
    //   REDUCED_IMG_data,
    //   32,
    //   24,
    //   &reduced_lock,
    //   NULL,
    //   &quit_flag,
    //   true
    // };
    struct io_peripherals *io;
    io = import_registers();
    if (io != NULL)
    {
        /* print where the I/O memory was actually mapped to */
        printf( "mem at 0x%8.8X\n", (unsigned int*)io );

        set_gpio_context(io->gpio);  // save gpio context
        signal(SIGINT, sigint_handler);

        enable_pwm_clock(io->cm, io->pwm);
        /* set the pin function to OUTPUT for GPIO22 - */
        /* set the pin function to OUTPUT for GPIO23 -    */
        io->gpio->GPFSEL2.field.FSEL2 = GPFSEL_OUTPUT;
        io->gpio->GPFSEL2.field.FSEL3 = GPFSEL_OUTPUT;
        /* set the pin function to OUTPUT for GPIO24 - */
        /* set the pin function to OUTPUT for GPIO25 -    */
        io->gpio->GPFSEL2.field.FSEL4 = GPFSEL_INPUT;
         io->gpio->GPFSEL2.field.FSEL5 = GPFSEL_INPUT;
        /* set the pin function to OUTPUT for GPIO05 - */
        /* set the pin function to OUTPUT for GPIO06 -    */
        io->gpio->GPFSEL0.field.FSEL5 = GPFSEL_OUTPUT;
        io->gpio->GPFSEL0.field.FSEL6 = GPFSEL_OUTPUT;

        /* set the pin function to alternate function 0 for GPIO12, PWM for LED on GPIO12 */
        /* set the pin function to alternate function 0 for GPIO13, PWM for LED on GPIO13 */
        io->gpio->GPFSEL1.field.FSEL2 = GPFSEL_ALTERNATE_FUNCTION0;
        io->gpio->GPFSEL1.field.FSEL3 = GPFSEL_ALTERNATE_FUNCTION0;

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
        
        printf("trying to free buffers \n");
        free(RGB_IMG_raw);
        printf("rgb buffer freed succesfully \n");
        free(IMG_RAW);
        printf("original image freed succesfully \n");
        free(GREYSCALE_IMG_raw);
        printf("greyscale image buffer freed succesfully \n");
        free(BW_IMG_raw);
        printf("bw image buffer freed successsfully \n");
    

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
