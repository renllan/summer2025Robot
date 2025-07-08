#include "hw6.h"
void *KeyRead(void * arg)
{
  struct  key_thread_param * param = (struct key_thread_param *)arg;
  struct  thread_command cmd = {0, 0};
  int     keyhit1 = 0;
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
          break;            // nothing placed on param->key_fifo
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
          if (!(FIFO_FULL( param->key_fifo )))
          {FIFO_INSERT( param->key_fifo, cmd );}
          else {printf( "key_fifo queue full\n" );}
        }

      }
      printf("<hw6> \n");
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
  bool pause_thread = true;
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
    //   int left_val = GPIO_READ(param->gpio,param->pin_1);
    //   int right_val = GPIO_READ(param->gpio,param->pin_2);
    //   if(left_val != 0){
    //     cmd.command = 'd';
    //     cmd.argument = 0;
    //     if(!FIFO_FULL(param->dir_fifo)){FIFO_INSERT(param->dir_fifo,cmd);}
    //     usleep(250000);

    //   }
    //   if(right_val != 0){
    //     cmd.command = 'a';
    //     cmd.argument = 0;
    //     if(!FIFO_FULL(param->dir_fifo)){FIFO_INSERT(param->dir_fifo,cmd);}
    //     usleep(250000);
    //   }
    }
    //check for mode switching
    // if(!FIFO_EMPTY(param->IR_sensor_fifo))
    // {
    //   FIFO_REMOVE(param->IR_sensor_fifo,&cmd2);
    //   printf( "\n %s= %c\n", param->name, cmd2.command);
    //   switch(cmd2.command)
    //   {
    //     case 'w':{pause_thread = false;break;}
    //     case 's':{pause_thread = true; break;}
    //     default:{ printf("invalid mode \n");}
    //   }
    // }

    wait_period( &timer_state, 10u );
  }
  printf("IR Sensor thread Quit\n");
  return NULL;
}

void *Control(void * arg)
{
  struct  control_thread_param * param = (struct control_thread_param *)arg;
  struct  thread_command cmd1 = {0, 0};  // copy of input, cmd from key_thread
  struct  thread_command cmd2 = {0, 0};  // copy of output, cmd to put on LED_thread
  struct  timespec  timer_state; 
             // used to wake up every 10ms with wait_period() function, 
             // similar to interrupt occuring every 10ms
             

  // start 10ms timed wait, ie. set interrupt
  wait_period_initialize( &timer_state );
  wait_period( &timer_state, 10u ); /* 10ms */
  bool change = false;
/* Pick up incoming key presses every 10ms, 100 times/sec,
*  from the key input FIFO queues, and generate LED on/off commands, 
*  and fill the Red LED command FIFO queue and Green LED command FIFO queue,
*  for the LED on/off control.  
*  A simple and short LED on/off control programming is possible   */
  printf("control thread started \n");
  while (!*(param->quit_flag))
  {
    if (!(FIFO_EMPTY( param->key_fifo )))
    {
      FIFO_REMOVE( param->key_fifo, &cmd1 );  // read once every 10ms
      printf( "\n %s= %c  %c\n", param->name, cmd1.command, cmd1.argument);
      switch (cmd1.command)
      {
        case '1': 
          if(change)
          {
            param->mode = true; 
            cmd2.command = 's';
            cmd2.argument = 0;
            if(!FIFO_FULL(param->img_cmd_fifo))
            {FIFO_INSERT(param->img_cmd_fifo,cmd2);}
            printf("switched to mode 1 \n");
          } 
          change = false;
          break;
        case '2':
          if(change)
          {
            param->mode = false;
            cmd2.command = 'w';
            cmd2.argument = 0;
            if(!FIFO_FULL(param->img_cmd_fifo))
            {FIFO_INSERT(param->img_cmd_fifo,cmd2);}
            if(!FIFO_FULL(param->motor_control_fifo))
            {
              FIFO_INSERT(param->motor_control_fifo,cmd2);
            }
            printf("switched to mode 2 \n");change = false;
          }
          break;
        case 'h':
        {
          cmd2.command =cmd1.command;
          cmd2.argument = 0;
          if(!FIFO_FULL(param->img_cmd_fifo))
          {
            FIFO_INSERT(param->img_cmd_fifo,cmd2);
          }break;
        }
        case 'e':
        {
          cmd2.command =cmd1.command;
          cmd2.argument = 0;
          if(!FIFO_FULL(param->img_cmd_fifo))
          {
            FIFO_INSERT(param->img_cmd_fifo,cmd2);
          }break;
        }
        case 'c':
        
          cmd2.command ='c';
          cmd2.argument = 0;
          if(!FIFO_FULL(param->img_cmd_fifo))
          {
            FIFO_INSERT(param->img_cmd_fifo,cmd2);
          }break;
        
        case 'v': case 'l':
          cmd2.command =cmd1.command;
          cmd2.argument = 0;
          if(!FIFO_FULL(param->img_cmd_fifo))
          {
            FIFO_INSERT(param->img_cmd_fifo,cmd2);
          }break;
        
        case 'b':
          cmd2.command ='b';
          cmd2.argument = 0;
          if(!FIFO_FULL(param->img_cmd_fifo))
          {
            FIFO_INSERT(param->img_cmd_fifo,cmd2);
          }
          break;
        
        case 'n':
          cmd2.command ='n';
          cmd2.argument = 0;
          if(!FIFO_FULL(param->img_cmd_fifo))
          {
            FIFO_INSERT(param->img_cmd_fifo,cmd2);
          }break;
        
        case 'w':
        
          if(param->mode)
          {
            cmd2.command = 'w';
            cmd2.argument = 0;
            if(!FIFO_FULL(param->motor_control_fifo))
            {
              FIFO_INSERT(param->motor_control_fifo,cmd2);
            }
            break;
          }
          else{ //mode2
            cmd2.command = 'w';
            cmd2.argument = 0;
            if(!FIFO_FULL(param->img_cmd_fifo))
            {FIFO_INSERT(param->img_cmd_fifo,cmd2);}
          } 
          break;             
        
        case 'a':
          if(param->mode) { 
            

              cmd2.command = 'a';
              cmd2.argument = 0;
              if(!FIFO_FULL(param->motor_control_fifo))
             {FIFO_INSERT(param->motor_control_fifo,cmd2);}
            
          }
          break;
        
        case 's':
            cmd2.command = 's';
            cmd2.argument = 0;
            if(param->mode){
              if(!FIFO_FULL(param->motor_control_fifo)){
                {FIFO_INSERT(param->motor_control_fifo,cmd2);}
                break;
              }
            }
            else{ //mode2
              if(!FIFO_FULL(param->img_cmd_fifo)){
                {FIFO_INSERT(param->img_cmd_fifo,cmd2);}
                break;
              }
            } 
            break;
          
          case 'd':
          
            if(param->mode)
            {  
              cmd2.command = 'd';
              cmd2.argument = 0;
              if(!FIFO_FULL(param->motor_control_fifo))
              {FIFO_INSERT(param->motor_control_fifo,cmd2);}
              
            }
            break;
          
          case 'i':
          
            cmd2.command = 'i';
            cmd2.argument = 0;
            if(!FIFO_FULL(param->motor_control_fifo))
            {FIFO_INSERT(param->motor_control_fifo,cmd2);}
            break;
          
          case 'j':
          
            cmd2.command = 'j';
            cmd2.argument = 0;
            if(!FIFO_FULL(param->motor_control_fifo))
            {FIFO_INSERT(param->motor_control_fifo,cmd2);}
            
            break;
          
          case 'x':
          { 
            cmd2.command = 'x';
            cmd2.argument = 0;
            if(!FIFO_FULL(param->motor_control_fifo))
            {FIFO_INSERT(param->motor_control_fifo,cmd2);}
            break;
          }
          case 'o':
          { 
            cmd2.command = 'o';
            cmd2.argument = 0;
            if(!FIFO_FULL(param->motor_control_fifo))
            {FIFO_INSERT(param->motor_control_fifo,cmd2);}
            break;
          }
          case 'k':
            
            cmd2.command = 'k';
            cmd2.argument = 0;
            if(!FIFO_FULL(param->motor_control_fifo))
            {FIFO_INSERT(param->motor_control_fifo,cmd2);}
            break;
          
          case 'q':
          {
            cmd2.command = 'q';
            cmd2.argument = 0;
            *param->quit_flag = true;
            if(! (FIFO_FULL(param->motor_control_fifo)) && ! (FIFO_FULL(param->motor_control_fifo)))
            {
              FIFO_INSERT(param->motor_control_fifo,cmd2);
              FIFO_INSERT(param->motor_control_fifo,cmd2);
            }
            break;
          }  
          case 'm':
          
            //pause the car first when switching mode
            cmd2.command = 's';
            cmd2.argument= 0;
            if(!FIFO_FULL(param->motor_control_fifo))
            {FIFO_INSERT(param->motor_control_fifo,cmd2);}
            change = true;
            break;
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
  bool busy= false;
  int busy_count =0;
  wait_period_initialize( &timer_state );
  wait_period( &timer_state, 10u ); /* 10ms */
  printf("motor speed started \n");
  while(!(*(param->quit_flag)))
  {
    if(!busy){
      if(!FIFO_EMPTY(param->speed_fifo)){
        struct thread_command cmd = {0,0}; 
        FIFO_REMOVE(param->speed_fifo,&cmd);
        printf( "\n %s= %c  %d\n", param->name, cmd.command, cmd.argument);
        switch (cmd.command)
        {
        case 's':
          /* code */
          int speed = cmd.argument;
          param->pwm->DAT1 = speed;
          param->pwm->DAT2 = speed;
          break;
        
        case 'b':
          busy = true;
          busy_count = cmd.argument;
          printf("speed thread busy count = %d\n", busy_count);
          break;
        default:
          break;
        }
      }
      
    }
    else{
      if(busy_count == 0){
        busy =false;
      }else{
        busy_count --;
      }
    }
    wait_period( &timer_state, 10u );
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
  bool busy = false;
  int busy_count = 0;
  printf("direction thread started \n");
  while(! (*param->quit_flag))
  {
    if(!busy)
    {
      //pop from fifo
      struct thread_command cmd = {0,0};
      if(!FIFO_EMPTY(param->dir_fifo))
      {
        FIFO_REMOVE(param->dir_fifo,&cmd);
        printf( "\n %s= %c  %d\n", param->name, cmd.command, cmd.argument);
        switch (cmd.command)
        {
          case 'b':{
            busy = true;
            busy_count = cmd.argument;
            printf("direction thread busy count = %d \n", busy_count);
            break;
          }
          case 'p':{
            GPIO_CLR(param->gpio,param->pin_1);
            GPIO_CLR(param->gpio,param->pin_2);
            GPIO_CLR(param->gpio,param->pin_3);
            GPIO_CLR(param->gpio,param->pin_4);
            break;
          }
          case 'w':{
            GPIO_SET(param->gpio,param->pin_1);
            GPIO_CLR(param->gpio,param->pin_2);
            GPIO_SET(param->gpio,param->pin_3);
            GPIO_CLR(param->gpio,param->pin_4);
            break;
          }
          case 'a':{
            GPIO_CLR(param->gpio,param->pin_1);
            GPIO_SET(param->gpio,param->pin_2);
            GPIO_SET(param->gpio,param->pin_3);
            GPIO_CLR(param->gpio,param->pin_4);
            
            break;
          }
          case 's':{
            GPIO_SET(param->gpio,param->pin_1);
            GPIO_SET(param->gpio,param->pin_2);
            GPIO_SET(param->gpio,param->pin_3);
            GPIO_SET(param->gpio,param->pin_4);
            break;
          }
          case 'd':{
            GPIO_SET(param->gpio,param->pin_1);
            GPIO_CLR(param->gpio,param->pin_2);
            GPIO_CLR(param->gpio,param->pin_3);
            GPIO_SET(param->gpio,param->pin_4);
            
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
      }
    }
    else{
      if(busy_count == 0){
        busy =false;
      } 
      else{
        busy_count --;
      }
    }
    
    wait_period( &timer_state, 10u ); 
  }
  printf("%s thread ended \n",param->name);
  return NULL;
}
void *Motor_Control(void * arg){
  struct motor_control_thread_param *param = (struct motor_control_thread_param*)arg;
  struct thread_command cmd1 = {0};
  struct thread_command  cmd2 = {0};
  struct  timespec  timer_state; 
  wait_period_initialize( &timer_state );
  wait_period( &timer_state, 100u );
  printf("motor control thread started \n");
  char prev_dir = 's';
  while(!(*param->quit_flag)){
    if(!FIFO_EMPTY(param->motor_control_fifo))
    {
      FIFO_REMOVE(param->motor_control_fifo,&cmd1);
      printf( "\n %s= %c  %d\n", param->name, cmd1.command, cmd1.argument);
      switch (cmd1.command)
      {
      case 'w':
        /* code */
        if (prev_dir == 'x'){
          if(!FIFO_FULL(param->speed_fifo)){
            cmd2.command = 's';
            cmd2.argument = param->pwm_val/2;
            FIFO_INSERT(param->speed_fifo,cmd2);
            cmd2.argument = 0;
            FIFO_INSERT(param->speed_fifo,cmd2);
          }
        }
        if(!FIFO_FULL(param->dir_fifo)){
          cmd2.argument = 0;
          cmd2.command = 'p';
          FIFO_INSERT(param->dir_fifo,cmd2);
          cmd2.command  = 'w';
          FIFO_INSERT(param->dir_fifo,cmd2);
        }
        if(prev_dir == 'x'){
          if(!FIFO_FULL(param->speed_fifo)){
              cmd2.command = 's';
              cmd2.argument = param->pwm_val/2;
              FIFO_INSERT(param->speed_fifo,cmd2);
              cmd2.argument = param->pwm_val;
              FIFO_INSERT(param->speed_fifo,cmd2);
          }
        }
        prev_dir ='w';
        break;
      case 'a':
      //change the direction first
        int busy2 = (int)(param->angle*1.5);
        if(!FIFO_FULL(param->dir_fifo)){
          cmd2.command   = 'a';
          cmd2.argument = 0;
          FIFO_INSERT(param->dir_fifo,cmd2);
          cmd2.command= 'b';
          cmd2.argument = busy2;
          FIFO_INSERT(param->dir_fifo,cmd2);
          cmd2.command = prev_dir,
          cmd2.argument = 0;
          FIFO_INSERT(param->dir_fifo,cmd2);
        }
        if(!FIFO_FULL(param->speed_fifo)){
          
          cmd2.command = 's';
          cmd2.argument = 100;
          FIFO_INSERT(param->speed_fifo,cmd2);  
          cmd2.command= 'b';
          cmd2.argument = busy2;
          FIFO_INSERT(param->speed_fifo,cmd2);
          cmd2.command = 's';
          cmd2.argument = param->pwm_val;
          FIFO_INSERT(param->speed_fifo,cmd2);
        }
        break;
      case 's':
        if(!FIFO_FULL(param->dir_fifo)){
          cmd2.command   = 's';
          cmd2.argument = 0;
          FIFO_INSERT(param->dir_fifo,cmd2);
        }
        prev_dir = 's';
        break;
      case 'd':
        int busy1 = (int)(param->angle*1.5);
        if(!FIFO_FULL(param->dir_fifo)){
          cmd2.command   = 'd';
          cmd2.argument = 0;
          FIFO_INSERT(param->dir_fifo,cmd2);
          cmd2.command= 'b';cmd2.argument = busy1;
          FIFO_INSERT(param->dir_fifo,cmd2);
          cmd2.command = prev_dir,cmd2.argument = 0;
          FIFO_INSERT(param->dir_fifo,cmd2);
        }
        if(!FIFO_FULL(param->speed_fifo)){
          
          cmd2.command ='s';
          cmd2.argument = 100;
          FIFO_INSERT(param->speed_fifo,cmd2);  
          cmd2.command= 'b';cmd2.argument = busy1;
          FIFO_INSERT(param->speed_fifo,cmd2);
          cmd2.command = 's';
          cmd2.argument = param->pwm_val;
          FIFO_INSERT(param->speed_fifo,cmd2);
        }
        break;
      case 'x':
        if (prev_dir == 'd'){
          if(!FIFO_FULL(param->speed_fifo)){
            cmd2.command = 's';
            cmd2.argument = param->pwm_val/2;
            FIFO_INSERT(param->speed_fifo,cmd2);
            cmd2.argument = 0;
            FIFO_INSERT(param->speed_fifo,cmd2);
          }
        }
        if(!FIFO_FULL(param->dir_fifo)){
          cmd2.argument = 0;
          cmd2.command = 'p';
          FIFO_INSERT(param->dir_fifo,cmd2);
          cmd2.command  = 'x';
          FIFO_INSERT(param->dir_fifo,cmd2);
        }
        if(prev_dir == 'd'){
          if(!FIFO_FULL(param->speed_fifo)){
              cmd2.command = 's';
              cmd2.argument = param->pwm_val/2;
              FIFO_INSERT(param->speed_fifo,cmd2);
              cmd2.argument = param->pwm_val;
              FIFO_INSERT(param->speed_fifo,cmd2);
          }
        }
        prev_dir = 'x';
        break;
      case 'i':
        if(param->pwm_val < 95){
          param->pwm_val +=5;
          if(!FIFO_FULL(param->speed_fifo)){
            cmd2.command = 's';
            cmd2.argument = param->pwm_val;
            FIFO_INSERT(param->speed_fifo,cmd2);
          } 
        }
        else{
          printf("already at maximum speed \n");
        }
        break;
      case 'j':
        if(param->pwm_val > 5){
          param->pwm_val -=5;
          if(param->pwm_val > 5){
            param->pwm_val -=5;
            if(!FIFO_FULL(param->speed_fifo)){
              cmd2.command = 's';
              cmd2.argument = param->pwm_val;
              FIFO_INSERT(param->speed_fifo,cmd2);
            } 
          }
          else{
            printf("already at minimum speed \n");
          }
        }
        break;
      case 'o':
        if(param->angle <85){
          param->angle +=5;
          printf("current angle = %d", param->angle);
        }
        else{
          printf("already at maximum angle \n");
        }
        break;
        
      case 'k':
        if(param->angle >5){
          param->angle -=5;
          printf("current angle = %d", param->angle);
        }
        else{
          printf("already at min angle \n");
        }
        break;
      default:
        break;
      }
    }

    wait_period( &timer_state, 10u ); /* 10ms */
  }
  printf("%s thread exited \n",param->name);
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
  struct video_interface_handle_t *         handle_video1;
  struct draw_bitmap_multiwindow_handle_t * handle_GUI_RGB = NULL;

  handle_video1 = video_interface_open( "/dev/video0" );
  struct  timespec  timer_state; 
             // used to wake up every 10ms with wait_period() function,
             // similar to interrupt occuring every 10ms

  // start 10ms timed wait
  wait_period_initialize( &timer_state );
  wait_period( &timer_state, 100u ); /* 500 ms */
  video_interface_print_modes( handle_video1);
  if (video_interface_set_mode_auto( handle_video1))
  {
    int scaled_width      = handle_video1->configured_width/SCALE_REDUCTION_PER_AXIS;
    int scaled_height     = handle_video1->configured_height/SCALE_REDUCTION_PER_AXIS;
    printf("img size = %d x %d\n", handle_video1->configured_width,handle_video1->configured_height);
    printf("original img size = %d x %d\n", scaled_width,scaled_height);
    printf("size of image_t %d\n", IMAGE_SIZE);
    
    int argc = 0;
    char *argv[3];
    int counter = 0;
    printf("scaled height: %d scaled width %d", scaled_height,scaled_width);
    while(! *param->quit_flag)
    {
      counter++;
      
        if (video_interface_get_image(handle_video1, param->image))
        {

          //
          
          scale_image_data(
            (struct pixel_format_RGB *)param->image,
            handle_video1->configured_height,
            handle_video1->configured_width,
            param->img_data,
            SCALE_REDUCTION_PER_AXIS,
            SCALE_REDUCTION_PER_AXIS );

            struct thread_command cmd = {'u',0};
            memcpy(param->rgb_raw,param->img_raw,IMAGE_SIZE);
            memcpy(param->greyscale_raw,param->img_raw,IMAGE_SIZE);
            memcpy(param->bw_raw,param->img_raw,IMAGE_SIZE);
            memcpy(param->reduced_raw,param->img_raw,IMAGE_SIZE);

            FIFO_INSERT(param->rgb_cmd_fifo,cmd);
            FIFO_INSERT(param->greyscale_cmd_fifo,cmd);
            FIFO_INSERT(param->bw_cmd_fifo,cmd);
            FIFO_INSERT(param->reduced_cmd_fifo,cmd);
            FIFO_INSERT(param->hist_fifo,cmd);
            FIFO_INSERT(param->egg_fifo,cmd);
            FIFO_INSERT(param->single_channel_fifo,cmd);
            
            
          //draw_bitmap_display(handle_GUI_RGB, param->img_data);
        }
        else
        {
          printf("did not get image \n");
        }
      

      struct thread_command cmd1 = {0,0};
      struct thread_command cmd2 = {0,0};
      if(!FIFO_EMPTY(param->img_cmd_fifo))
      {
        FIFO_REMOVE(param->img_cmd_fifo,&cmd1);
        printf( "\n %s= %c  %c\n", param->name, cmd1.command, cmd1.argument);
        switch(cmd1.command)
        {
          case 'l':
            cmd2 =cmd1;
            if(!FIFO_FULL(param->single_channel_fifo))
            {
              FIFO_INSERT(param->single_channel_fifo,cmd2);
            }
            break;
          case 'e':
          {
            cmd2 = cmd1;
            if(!FIFO_FULL(param->egg_fifo)){
              FIFO_INSERT(param->egg_fifo,cmd2);
            }
          }
          case 'h':
          {
            cmd2 = cmd1;
            if(!FIFO_FULL(param->hist_fifo))
            {
              FIFO_INSERT(param->hist_fifo,cmd1);
            }
            break;
          }
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
          case 'w':{
            cmd2 = cmd1;
            if(!FIFO_FULL(param->egg_fifo) && !FIFO_FULL(param->reduced_cmd_fifo))
            {
              FIFO_INSERT(param->reduced_cmd_fifo,cmd2);
              FIFO_INSERT(param->egg_fifo,cmd2);

            }
            break;
          }
          case 's':
          {
            cmd2 = cmd1;
            if(!FIFO_FULL(param->egg_fifo) && !FIFO_FULL(param->reduced_cmd_fifo))
            {
              FIFO_INSERT(param->reduced_cmd_fifo,cmd2);
              FIFO_INSERT(param->egg_fifo,cmd2);
            }
            break;
          }
          default:{break;}
        }
      }
      wait_period( &timer_state,10u );
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
  struct draw_bitmap_multiwindow_handle_t *handle = NULL;//draw_bitmap_create_window(param->width,param->height);
  struct thread_command cmd = {0,0};
  struct  timespec  timer_state; 
  wait_period_initialize( &timer_state );
  wait_period( &timer_state, 100u ); /* 500 ms */

  while(!(*param->quit_flag))
  {
    if(!(FIFO_EMPTY(param->img_cmd_fifo)))
    {
      FIFO_REMOVE(param->img_cmd_fifo,&cmd);
      //printf( "\n %s= %c  %c\n", param->name, cmd.command, cmd.argument);
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
        case 'u':
        {
          //memcpy(param->RGB_IMG_raw,param->ORIG_IMG_raw, IMAGE_SIZE);
          //draw cross
          
          for(size_t i= 160; i< IMAGE_SIZE/3; i+=param->width)
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
        }
        default:{break;}
      }
    }
    //handle = NULL;
    if(handle != NULL){draw_bitmap_display(handle,param->RGB_IMG_data);}
    
    wait_period(&timer_state, 10u);
  }
  //draw_bitmap_close_window(handle);
  printf("%s exited \n", param->name);
  return NULL;
}

void *greyscale_video(void * arg){
  struct img_process_thread_param *param = (struct img_process_thread_param*)arg;
  printf("%s thread started \n",param->name);
  struct draw_bitmap_multiwindow_handle_t *handle = NULL;//draw_bitmap_create_window(param->width,param->height);

  struct thread_command cmd = {0,0};

  struct  timespec  timer_state; 
  wait_period_initialize( &timer_state );
  wait_period( &timer_state, 100u ); /* 500 ms */

  while(!(*param->quit_flag))
  {
    
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
        case 'u':
        {
          //memcpy(param->RGB_IMG_raw, param->ORIG_IMG_raw, IMAGE_SIZE);
          for(size_t i= 0; i< IMAGE_SIZE/3; i++)
          {
            int avg = (param->RGB_IMG_data[i].R +param->RGB_IMG_data[i].G +param->RGB_IMG_data[i].B)/3;
            param->RGB_IMG_data[i].R = avg;
            param->RGB_IMG_data[i].G = avg;
            param->RGB_IMG_data[i].B = avg;
          }
        }
        default:{break;}
      
      }
      
    }
    if(handle != NULL){draw_bitmap_display(handle,param->RGB_IMG_data);}
    wait_period(&timer_state, 10u);
  }
  printf("%s exited \n",param->name);
  return NULL;
}


void *reduced_video(void * arg){
  struct reduced_img_thread_param *param = (struct reduced_img_thread_param*)arg;
  printf("%s thread started \n",param->name);
  struct draw_bitmap_multiwindow_handle_t *handle = draw_bitmap_create_window(param->width,param->height);
  char *reduced_raw = (char *)malloc(param->height*param->width*3);
  
  struct pixel_format_RGB* reduced_img = (struct pixel_format_RGB*) reduced_raw;
  struct thread_command cmd= {0,0};
  struct thread_command cmd2 = {0,0};
  bool pause_thread = true;
  struct  timespec  timer_state; 
  wait_period_initialize( &timer_state );
  wait_period( &timer_state, 100u); /* 500 ms */
  int row =15;
  int offset = 6;
  int left_IR_Sensor = param->width*row + param->width/2- offset;
  int right_IR_sensor = param->width*row + param->width/2 +offset;
  char prev_dir = 'x';
  int counter = 0;
  while(!(*param->quit_flag))
  {
    
    //memcpy(param->RGB_IMG_raw, param->ORIG_IMG_raw, IMAGE_SIZE);
          
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
        case 'w':{
          printf("starting camera detection \n");
          pause_thread = false;
          break;}
        case 's':{
          printf("pausing camera detection \n");
          pause_thread = true; break;}
        case 'u':{
          scale_image_data(
                      param->RGB_IMG_data,
                      240,
                      320,
                      reduced_img,
                      10,
                      10);
          
          to_black_white(reduced_img,param->width*param->height,100);
          break;
        }
        default:{break;}
      }
    }
    
    
    //pixel count
    if(!pause_thread && counter % 250==0)
    {
      //printf("using camera to control car \n");
      //right have black line
      //if not white line detected, perform correction
      bool all_white = true;
      for(int i = row*param->width; i<(row+1)*param->width; i++)
      {
        
        if(reduced_img[i].G ==0){
          all_white = false;
          break;
        }
      } 
      if(all_white){
        //printf("did not detect black pixel\n");
        cmd.command = prev_dir;
        if(!FIFO_FULL(param->dir_fifo))
        {
          // struct thread_command stop_cmd = {'s',0};
          // //FIFO_INSERT(param->dir_fifo,stop_cmd);
          // FIFO_INSERT(param->dir_fifo,cmd);
          // //usleep(250000);
        }
      }
      for(int i = right_IR_sensor; i<(right_IR_sensor+3*param->width);i+=param->width)
      {
        if(reduced_img[i].B == 0)
        {
          cmd.command='d';
          prev_dir = 'a';
          //printf("detected blackline at the right \n");
          // if(!FIFO_FULL(param->dir_fifo))
          // {
          //   FIFO_INSERT(param->dir_fifo,cmd);
          //   //usleep(250000);
          //   break;
          // }
        }
      
      }
      
      //left 
      for(int i = left_IR_Sensor; i<left_IR_Sensor+(3*param->width);i+=param->width)
      {
        if(reduced_img[i].B == 0)
        {
          cmd.command = 'a';
          prev_dir = 'd';
          //printf("detected blackline at the left\n");
          // if(!FIFO_FULL(param->dir_fifo))
          // {
          //   FIFO_INSERT(param->dir_fifo,cmd);
          //   //printf("detection thread sleeping \n");
          //   //usleep(250000);
          //   break;
          // }
        }
      }
      
      
    }
    // reduced_img[left_IR_Sensor].G = 255;
    // reduced_img[left_IR_Sensor].R = 0;
    // reduced_img[left_IR_Sensor].B = 0;
  
    // reduced_img[right_IR_sensor].G = 255;
    // reduced_img[right_IR_sensor].B =0;
    // reduced_img[right_IR_sensor].R =0;
    for(int i = row*param->width; i<(row+1)*param->width; i++)
    {
      reduced_img[i].G = 255;
      reduced_img[i].B = 0;
      reduced_img[i].R = 0;
    }  
    for(int i =  param->width/2- offset; i< param->width*param->height; i+= param->width)
    {
      reduced_img[i].G = 0;
      reduced_img[i].B = 0;
      reduced_img[i].R = 255;
    }
    for(int i =  param->width/2 + offset; i< param->width*param->height; i+= param->width)
    {
      reduced_img[i].G = 255;
      reduced_img[i].B = 0;
      reduced_img[i].R = 0;
    }
    if(handle != NULL){draw_bitmap_display(handle,reduced_img);}
  
    wait_period(&timer_state, 10u);
  }
  free(reduced_raw);
  printf("%s exited \n", param->name);
  return NULL;
}
void *single_channel(void *arg){
  struct img_process_thread_param *param = (struct img_process_thread_param*)arg;
  printf("%s thread started \n",param->name);
  struct draw_bitmap_multiwindow_handle_t *r_handle = NULL;
  struct draw_bitmap_multiwindow_handle_t *g_handle = NULL;
  struct draw_bitmap_multiwindow_handle_t *b_handle = NULL;

  unsigned char r_img[IMAGE_SIZE];
  unsigned char g_img[IMAGE_SIZE];
  unsigned char b_img[IMAGE_SIZE];

  struct pixel_format_RGB * r_data = (struct pixel_format_RGB*)r_img;
  struct pixel_format_RGB * g_data = (struct pixel_format_RGB*)g_img;
  struct pixel_format_RGB * b_data = (struct pixel_format_RGB*)b_img;
  struct thread_command cmd = {0,0};
  printf("%s thread started\n", param->name);
  struct  timespec  timer_state; 
  wait_period_initialize( &timer_state );
  wait_period( &timer_state, 100u ); /* 500 ms */
  while(!(*param->quit_flag)) {
        // Process commands
    if(!(FIFO_EMPTY(param->img_cmd_fifo))) {
      FIFO_REMOVE(param->img_cmd_fifo, &cmd);
    
    switch(cmd.command){
      case 'l':
        printf("%s cmd = %c, %d",param->name,cmd.command,cmd.argument);
        if(!r_handle){
          r_handle = draw_bitmap_create_window(param->width,param->height);
          g_handle = draw_bitmap_create_window(IMG_WIDTH,IMG_HEIGHT);
          b_handle = draw_bitmap_create_window(IMG_WIDTH,IMG_HEIGHT);
        }
        else{
            draw_bitmap_close_window(r_handle);
            draw_bitmap_close_window(g_handle);
            draw_bitmap_close_window(b_handle);
            r_handle = NULL;
            g_handle = NULL;
            b_handle = NULL;
        }
        break;
      
      
      case 'u':
          memcpy(r_img,param->RGB_IMG_raw,IMAGE_SIZE);
          memcpy(g_img,param->RGB_IMG_raw,IMAGE_SIZE);
          memcpy(b_img,param->RGB_IMG_raw,IMAGE_SIZE);

          for(int i = 0; i < IMAGE_SIZE/3;i++){
            r_data[i] = (struct pixel_format_RGB ){r_data[i].R,0,0};
            g_data[i] = (struct pixel_format_RGB ){0,g_data[i].R,0};
            b_data[i] = (struct pixel_format_RGB ){0,0,b_data[i].G};
          }
          break;
      }

      if (r_handle) draw_bitmap_display(r_handle, r_data);
      if (g_handle) draw_bitmap_display(g_handle, g_data);
      if (b_handle) draw_bitmap_display(b_handle, b_data);
    }
    wait_period( &timer_state, 10u );
  } 
  if (r_handle) {draw_bitmap_close_window(r_handle); r_handle = NULL;}
  if (g_handle) {draw_bitmap_close_window(g_handle); g_handle = NULL;}
  if (b_handle) {draw_bitmap_close_window(b_handle); b_handle = NULL;}
  

  printf("%s exited\n", param->name);
  return NULL;
}

void *black_and_white(void * arg){
  struct img_process_thread_param *param = (struct img_process_thread_param*)arg;
  printf("%s thread started \n",param->name);
  struct draw_bitmap_multiwindow_handle_t *handle =NULL;
  struct thread_command cmd = {0,0};
  struct  timespec  timer_state; 
  wait_period_initialize( &timer_state );
  wait_period( &timer_state, 100u ); /* 500 ms */

  while(!(*param->quit_flag))
  {
    
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
        case 'u':
        {
          //memcpy(param->RGB_IMG_raw, param->ORIG_IMG_raw, IMAGE_SIZE);
          to_black_white(param->RGB_IMG_data,param->width*param->height,200);
          break;
        }
        default:{break;}
      }
    }
    
    if(handle != NULL){draw_bitmap_display(handle,param->RGB_IMG_data);}
    wait_period(&timer_state, 100u);
  }
  //draw_bitmap_close_window(handle);
  printf("%s exited \n",param->name);
  return NULL;
}

void enable_pwm(struct io_peripherals *io) {
    io->gpio->GPFSEL1.field.FSEL2 = GPFSEL_ALTERNATE_FUNCTION0;
    io->gpio->GPFSEL1.field.FSEL3 = GPFSEL_ALTERNATE_FUNCTION0;

    io->pwm->RNG1 = PWM_RANGE;
    io->pwm->RNG2 = PWM_RANGE;
    io->pwm->DAT1 = 1;
    io->pwm->DAT2 = 1;
    io->pwm->CTL.field.MODE1 = 0;
    io->pwm->CTL.field.MODE2 = 0;
    io->pwm->CTL.field.RPTL1 = 1;
    io->pwm->CTL.field.RPTL2 = 1;
    io->pwm->CTL.field.SBIT1 = 0;
    io->pwm->CTL.field.SBIT2 = 0;
    io->pwm->CTL.field.POLA1 = 0;
    io->pwm->CTL.field.POLA2 = 0;
    io->pwm->CTL.field.USEF1 = 0;
    io->pwm->CTL.field.USEF2 = 0;
    io->pwm->CTL.field.MSEN1 = 1;
    io->pwm->CTL.field.MSEN2 = 1;
    io->pwm->CTL.field.CLRF1 = 1;
    io->pwm->CTL.field.PWEN1 = 1;
    io->pwm->CTL.field.PWEN2 = 1;
}
void set_gpio(struct io_peripherals *io)
{
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
}
void *video_histogram(void *arg) 
{
    struct img_process_thread_param *param = (struct img_process_thread_param*)arg;
    printf("%s thread started\n", param->name);
    struct draw_bitmap_multiwindow_handle_t *grey_handle = NULL;
    struct draw_bitmap_multiwindow_handle_t *r_handle = NULL;
    struct draw_bitmap_multiwindow_handle_t *g_handle = NULL;
    struct draw_bitmap_multiwindow_handle_t *b_handle = NULL;

    struct thread_command cmd = {0,0};
    struct timespec timer_state;
    wait_period_initialize(&timer_state);
    wait_period(&timer_state, 100u);  // Initial delay
    
    // Histogram configuration
    #define HIST_WIDTH 256
    #define HIST_HEIGHT 100
    #define NUM_BINS 256

    int grey_hist[NUM_BINS] = {0};
    int r_hist[NUM_BINS]= {0};
    int g_hist[NUM_BINS] = {0};
    int b_hist[NUM_BINS] = {0};

    struct pixel_format_RGB grey_img[HIST_WIDTH * HIST_HEIGHT];
    struct pixel_format_RGB r_img[HIST_WIDTH * HIST_HEIGHT];
    struct pixel_format_RGB g_img[HIST_WIDTH * HIST_HEIGHT];
    struct pixel_format_RGB b_img[HIST_WIDTH * HIST_HEIGHT];

    while(!(*param->quit_flag)) {
        // Process commands
      if(!(FIFO_EMPTY(param->img_cmd_fifo))) {
        FIFO_REMOVE(param->img_cmd_fifo, &cmd);
        //printf( "\n %s= %c  %c\n", param->name, cmd.command, cmd.argument);
        switch(cmd.command) {
          case 'h':  // Toggle histogram window
            if(grey_handle == NULL) {
              grey_handle = draw_bitmap_create_window(HIST_WIDTH, HIST_HEIGHT);
              r_handle = draw_bitmap_create_window(HIST_WIDTH, HIST_HEIGHT);
              g_handle = draw_bitmap_create_window(HIST_WIDTH, HIST_HEIGHT);
              b_handle = draw_bitmap_create_window(HIST_WIDTH, HIST_HEIGHT);
            } else {
              draw_bitmap_close_window(grey_handle);
              draw_bitmap_close_window(r_handle);
              draw_bitmap_close_window(g_handle);
              draw_bitmap_close_window(b_handle);
              grey_handle = NULL;
              r_handle = NULL;
              g_handle = NULL;
              b_handle = NULL;
            }
            break;
                
          case 'u':  // Update histogram
              // Reset histogram counts
              memset(grey_hist, 0, sizeof(grey_hist));
              memset(r_hist, 0, sizeof(r_hist));
              memset(g_hist, 0, sizeof(g_hist));
              memset(b_hist, 0, sizeof(b_hist));
              for(size_t i= 0; i< IMAGE_SIZE/3; i++)
              {
                  int avg = (param->RGB_IMG_data[i].R +param->RGB_IMG_data[i].G +param->RGB_IMG_data[i].B)/3;
                  grey_hist[avg]++;
                  r_hist[param->RGB_IMG_data[i].R]++;
                  g_hist[param->RGB_IMG_data[i].G]++;
                  b_hist[param->RGB_IMG_data[i].B]++;
              }
              
              break;
              
          default:
              break;
        }
      }
        
        // Update and display histogram if window is open
        if(g_handle || grey_handle || b_handle ||r_handle)
        {
          int max_grey = 1;  // Initialize to 1 to avoid division by zero
          int max_r = 1;
          int max_g = 1;
          int max_b = 1;
          memset(grey_img, 0, sizeof(grey_img));
          memset(r_img, 0, sizeof(r_img));
          memset(g_img, 0, sizeof(g_img));
          memset(b_img, 0, sizeof(b_img));
          for(int i = 0; i < NUM_BINS; i++) {
              if(grey_hist[i] > max_grey) max_grey = grey_hist[i];
              if(r_hist[i] > max_r) max_r = r_hist[i];
              if(g_hist[i] > max_g) max_g = g_hist[i];
              if(b_hist[i] > max_b) max_b = b_hist[i];
          }
          for (int x = 0; x < HIST_WIDTH; x++) {
          // Map screen position to intensity bin (0-255)
            int bin = (x * NUM_BINS) / HIST_WIDTH;
            bin = bin > 255 ? 255 : bin;
            
            // Calculate bar heights (scale to histogram height)
            int grey_h = (grey_hist[bin] * HIST_HEIGHT) / max_grey;
            int r_h = (r_hist[bin] * HIST_HEIGHT) / max_r;
            int g_h = (g_hist[bin] *HIST_HEIGHT) / max_g;
            int b_h = (b_hist[bin] * HIST_HEIGHT) / max_b;
            
            // Draw vertical bars
            for (int y = 0; y < HIST_HEIGHT; y++) {
              int pos = (HIST_HEIGHT-1-y) * HIST_WIDTH + x;
              
              // Grayscale (white)
              if (y < grey_h) {
                  grey_img[pos] = (struct pixel_format_RGB){255, 255, 255};
              }
              
              // Red channel
              if (y < r_h) {
                  r_img[pos] = (struct pixel_format_RGB){255, 0, 0};
              }
              
              // Green channel
              if (y < g_h) {
                  g_img[pos] = (struct pixel_format_RGB){0, 255, 0};
              }
              
              // Blue channel
              if (y < b_h) {
                  b_img[pos] = (struct pixel_format_RGB){0, 0, 255};
              }
            }
          }
        }
        
        if (grey_handle) draw_bitmap_display(grey_handle, grey_img);
        if (r_handle) draw_bitmap_display(r_handle, r_img);
        if (g_handle) draw_bitmap_display(g_handle, g_img);
        if (b_handle) draw_bitmap_display(b_handle, b_img);
        wait_period(&timer_state, 10u);  // Update every 30ms
    }
    
    if(grey_handle) {draw_bitmap_close_window(grey_handle); grey_handle = NULL;}
    if(r_handle) {draw_bitmap_close_window(r_handle); r_handle = NULL;}
    if(g_handle) {draw_bitmap_close_window(g_handle); g_handle = NULL;}
    if(b_handle) {draw_bitmap_close_window(b_handle); b_handle = NULL;}


    printf("%s exited\n", param->name);
    return NULL;
}
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
void *egg_detector(void * arg)
{
    struct egg_detector_thread_param *param = (struct egg_detector_thread_param*)arg;
    struct  timespec  timer_state; 
    printf("%s thread started \n",param->name);
    wait_period_initialize( &timer_state );
    struct draw_bitmap_multiwindow_handle_t *handle =NULL;
    unsigned char * egg_buffer = (unsigned char *)malloc(IMAGE_SIZE+1);
    struct pixel_format_RGB * egg_data = (struct pixel_format_RGB *)egg_buffer;
    int x = 0;
    int y = 0;
    bool pause_thread = true;
    int min_x = IMG_WIDTH, min_y = IMG_HEIGHT;
    int max_x = 0, max_y = 0;
    int decision_q[MAX_DECISION_SIZE] = {0}; 
    bool queue_filled = false;
    int frames_seen = 0;
    bool stopped = false;
    bool centered = false;
    bool egg_found = false;
    int turn_cool_down =0;
    while(!(*param->quit_flag))
    {
      struct thread_command cmd = {0, 0};
      if(!FIFO_EMPTY(param->egg_fifo))
      {
        FIFO_REMOVE(param->egg_fifo,&cmd);
        switch (cmd.command)
        {
          case 'w': 
            pause_thread = false;
            printf( "\n %s= %c  %c\n", param->name, cmd.command, cmd.argument);
            printf("starting egg detection \n");
            break;
          case 's': 
            pause_thread = true;
            printf( "\n %s= %c  %c\n", param->name, cmd.command, cmd.argument);
            printf("stop egg detection \n");
            break;
          case 'e':
            if(handle == NULL)
            {
              handle = draw_bitmap_create_window(IMG_WIDTH,IMG_HEIGHT);
            }
            else{
              draw_bitmap_close_window(handle);
              handle = NULL;
            }
            break;
          case 'u':
          {
            frames_seen++;
            if (frames_seen >= MAX_DECISION_SIZE) {
              queue_filled = true;
            }
            memcpy(egg_buffer,param->RGB_IMG_raw,IMAGE_SIZE);
            to_black_white(egg_data, IMAGE_SIZE/3, EGG_THRESHOLD);
            x = 0;
            y = 0;
            min_x = IMG_WIDTH;min_y = IMG_HEIGHT; max_x =0;max_y =0;
            int count = 0;
            EggBlob eggs[MAX_EGGS];
            int found = find_egg_blobs(egg_data,eggs,MAX_EGGS,IMG_WIDTH,IMG_HEIGHT);
            //printf("found %d eggs \n", found);
            int max_egg = 0;

            for (int i = 0; i < found; i++) {
              int min_x = eggs[i].min_x;
              int max_x = eggs[i].max_x;
              int min_y = eggs[i].min_y;
              int max_y = eggs[i].max_y;

              // Draw red bounding box
              draw_bbox(min_x,min_y,max_x,max_y, egg_data,(struct pixel_format_RGB){255, 0, 0});
              if(eggs[i].size > eggs[max_egg].size)
              {
                max_egg = i;
              }
              //printf("Egg %d at center (%d, %d), size %d\n", i+1, eggs[i].center_x, eggs[i].center_y, eggs[i].size);
              for (int i = 0; i < MAX_DECISION_SIZE - 1; i++) {
                decision_q[i] = decision_q[i + 1];
              }

              
            }
            if(found > 0){
              int min_x = eggs[max_egg].min_x;
              int max_x = eggs[max_egg].max_x;
              int min_y = eggs[max_egg].min_y;
              int max_y = eggs[max_egg].max_y;
              draw_bbox(min_x,min_y,max_x,max_y,egg_data,(struct pixel_format_RGB){0, 255, 0});
              decision_q[MAX_DECISION_SIZE-1] = eggs[max_egg].center_x;
              //printf("largest egg size is is at %d \n", eggs[max_egg].size);
              egg_found = true;
            }
            else{
              decision_q[MAX_DECISION_SIZE-1] = -1;
              egg_found = false;
              //printf("did not find any eggs \n");
            }
            

            if(!pause_thread && frames_seen)
            {
              int left = 0, right = 0, center,not_found = 0;
              
              for (int i = 0; i < MAX_DECISION_SIZE; i++) {
                  if (decision_q[i] > 0 && decision_q[i] < CENTER_L) left++;
                  else if (decision_q[i] > CENTER_R) right++;
                  else if (decision_q[i] == -1) not_found++;
                  else center++;
              }
              
              if(eggs[max_egg].size > STOP_THRESH) //check if the egg is close enough
              {
                if(!FIFO_FULL(param->dir_fifo))
                {
                  printf("egg size is %d egg is close to robot\n",eggs[max_egg].size);
                  cmd.command = 's';
                  if(!stopped)
                  {
                    FIFO_INSERT(param->dir_fifo,cmd);
                  }
                  stopped = true;
                }
              }
              // else{
              //   if(!FIFO_FULL(param->dir_fifo))
              //   {
              //     printf("egg is not close to robot\n");
              //     cmd.command = 'w';
              //     if(stopped)
              //     {
              //       FIFO_INSERT(param->dir_fifo,cmd);
              //     }
              //     stopped = false;
              //   }
              // }
              if(turn_cool_down > 0)
              {
                turn_cool_down--;
              }
              else if(not_found > MAX_DECISION_SIZE/2){
                if(!FIFO_FULL(param->dir_fifo)){
                  
                  cmd.command = 'a';
                  FIFO_INSERT(param->dir_fifo,cmd);
                  cmd.command ='w';
                  FIFO_INSERT(param->dir_fifo,cmd);
                  turn_cool_down = TURN_COOLDOWN_FRAMES;
                  printf("did not find any egg \n");
                  stopped = false;
                }  
              }
               
              
              else if (left >= MAX_DECISION_SIZE/2) {
                if (!FIFO_FULL(param->dir_fifo)) {
                    
                    printf("queue decision: largest egg detected on the left\n");                    
                    
                    
                    cmd.command = 'a';
                    FIFO_INSERT(param->dir_fifo, cmd);
                    
                    //centered = false;
                    turn_cool_down = TURN_COOLDOWN_FRAMES;
                    
                   
                }
              } else if (right >= MAX_DECISION_SIZE/2) {
                if (!FIFO_FULL(param->dir_fifo)) {
                    printf("queue decision: largest egg detected on the right\n");
                    
                       
                    cmd.command = 'd';
                    FIFO_INSERT(param->dir_fifo, cmd);
                    
                    //centered= false;
                    // cmd.command = 's';
                    // FIFO_INSERT(param->dir_fifo, cmd);
                    // cmd.command = 'w';
                    // FIFO_INSERT(param->dir_fifo, cmd);
                    turn_cool_down = TURN_COOLDOWN_FRAMES;
                }
              }
              else if(center >=MAX_DECISION_SIZE/2){
                centered = true;
                // printf("egg is close to center of robot \n");
                // if (!FIFO_FULL(param->dir_fifo)) {
                    // cmd.command = 'w';
                    // FIFO_INSERT(param->dir_fifo, cmd);
                    // cmd.command = 'w';
                    // FIFO_INSERT(param->dir_fifo, cmd);
                    // cmd.command = 's';
                    // FIFO_INSERT(param->dir_fifo, cmd);

                // }
                printf("egg is at the center of the robot\n");
              }
              if(stopped && centered)
              {
                printf("robot is close enough to grab the egg \n");
                //todo: evan grab the egg
                //stopped = false;
               // centered = false;
                // if (!FIFO_FULL(param->dir_fifo)) {
                //     cmd.command = 'w';
                //     FIFO_INSERT(param->dir_fifoS, cmd);
                if(!FIFO_FULL(param->dir_fifo))
                {
                  cmd.command ='s';
                  FIFO_INSERT(param->dir_fifo,cmd);
                }
                

                // }
                break;
              }
            }
          }
        }
      }
        //draw the center of the egg
      if(handle) draw_bitmap_display(handle,egg_data);
      wait_period(&timer_state, 10u);
    }
    free(egg_buffer);
    printf("%s exited \n",param->name);
    return NULL;
}
