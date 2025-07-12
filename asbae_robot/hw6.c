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
        {
          if(change)
          {
            param->mode = 1; 
            cmd2.command = 's';
            cmd2.argument = 0;
            if(!FIFO_FULL(param->img_cmd_fifo))
            {FIFO_INSERT(param->img_cmd_fifo,cmd2);}
            printf("switched to mode 1 \n");
          } 
          change = false;
          break;
        }
        case '2':
        {
          if(change)
          {
            param->mode = 2;
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
        }
        case '3':
        {
          if(change)
          {
            param->mode = 3;
            cmd2.command = 's';
            cmd2.argument = 0;
            // if(!FIFO_FULL(param->img_cmd_fifo))
            // {FIFO_INSERT(param->img_cmd_fifo,cmd2);}
            if(!FIFO_FULL(param->motor_control_fifo))
            {
              FIFO_INSERT(param->motor_control_fifo,cmd2);
            }
            printf("switched to mode 3 \n");change = false;
          }
          break;
        }
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
          if (param->mode == 1 || param->mode == 2) {
            cmd2.command =cmd1.command;
            cmd2.argument = 0;
            if(!FIFO_FULL(param->img_cmd_fifo))
            {
              FIFO_INSERT(param->img_cmd_fifo,cmd2);
            }break;
          } else if (param->mode == 3) {
            cmd2.command = 'e';
            cmd2.argument = 0; // not needed for pwm servo left turn
            if(!FIFO_FULL(param->pwm_servo_fifo)) FIFO_INSERT(param->pwm_servo_fifo, cmd2);
            else printf("pwm_servo_fifo queue full\nHW6> ");
            break;
          }
          // other modes omitted for simplicity
          break;
        }
        case 'c':
        {
          cmd2.command ='c';
          cmd2.argument = 0;
          if (param->mode == 1 || param->mode == 2) {
            if(!FIFO_FULL(param->img_cmd_fifo))
            {
              FIFO_INSERT(param->img_cmd_fifo,cmd2);
            }break;
          } else if (param->mode == 3) {
            if (!(FIFO_FULL(param->claw_fifo))) FIFO_INSERT( param->claw_fifo, cmd2 );
            break;
          }
          break;
        }
        case 'r': //turn pwm servo right
        {
          if (param->mode == 3) {
            cmd2.command = 'r';
            cmd2.argument = 0; // not needed for pwm servo right turn
            if(!FIFO_FULL(param->pwm_servo_fifo)) FIFO_INSERT(param->pwm_servo_fifo, cmd2);
            else printf("pwm_servo_fifo queue full\nHW6> ");
            break;
          }
          // other modes omitted for simplicity
          break;
        }
        case 'v': case 'l':
        {
          cmd2.command =cmd1.command;
          cmd2.argument = 0;
          if(!FIFO_FULL(param->img_cmd_fifo))
          {
            FIFO_INSERT(param->img_cmd_fifo,cmd2);
          }break;
        }       
        case 'b':
        {
          cmd2.command ='b';
          cmd2.argument = 0;
          if(!FIFO_FULL(param->img_cmd_fifo))
          {
            FIFO_INSERT(param->img_cmd_fifo,cmd2);
          }
          break;
        }
        case 'n':
        {          
          cmd2.command ='n';
          cmd2.argument = 0;
          if(!FIFO_FULL(param->img_cmd_fifo))
          {
            FIFO_INSERT(param->img_cmd_fifo,cmd2);
          }break;
        }
        case 'w':
        {
          if(param->mode == 1)
          {
            cmd2.command = 'w';
            cmd2.argument = 0;
            if(!FIFO_FULL(param->motor_control_fifo))
            {
              FIFO_INSERT(param->motor_control_fifo,cmd2);
            }
            break;
          }
          else if(param->mode == 2) { //mode2
            cmd2.command = 'w';
            cmd2.argument = 0;
            if(!FIFO_FULL(param->img_cmd_fifo))
            {FIFO_INSERT(param->img_cmd_fifo,cmd2);}
          } 
          else if(param->mode == 3) { //mode3
            cmd2.command = 'w';
            cmd2.argument = 2; // move forward/backward servo
            if(!FIFO_FULL(param->arm_fifo)) FIFO_INSERT(param->arm_fifo, cmd2);
            else printf("arm_fifo queue full\nHW6> ");
            break;
          }
          break;             
        }
        case 'a':
        {
          if(param->mode == 1) { 
              cmd2.command = 'a';
              cmd2.argument = 0;
              if(!FIFO_FULL(param->motor_control_fifo))
             {FIFO_INSERT(param->motor_control_fifo,cmd2);}
             break;
          }
          else if (param->mode == 3) {
            cmd2.command = 'a';
            cmd2.argument = 1; // move spin servo
            if(!FIFO_FULL(param->arm_fifo)) FIFO_INSERT(param->arm_fifo, cmd2);
            else printf("arm_fifo queue full\nHW6> ");
            break;
          }
          break;
        }
        case 's':
        {
          cmd2.command = 's';
          cmd2.argument = 0;
          if(param->mode == 1){
            if(!FIFO_FULL(param->motor_control_fifo)){
              {FIFO_INSERT(param->motor_control_fifo,cmd2);}
              break;
            }
          }
          else if (param->mode == 2) { //mode2
            if(!FIFO_FULL(param->img_cmd_fifo)){
              {FIFO_INSERT(param->img_cmd_fifo,cmd2);}
              break;
            }
          } 
          else if (param->mode == 3) { //mode3
            if (!(FIFO_FULL(param->arm_fifo))) FIFO_INSERT( param->arm_fifo, cmd2 );
            else {
                printf( "arm_fifo queue full\nHW6> " );
                break;
            }
            if (!(FIFO_FULL(param->pwm_servo_fifo))) FIFO_INSERT( param->pwm_servo_fifo, cmd2 );
            else {
                printf( "pwm_servo_fifo queue full\nHW6> " );
                break;
            }
            if (!(FIFO_FULL(param->claw_fifo))) FIFO_INSERT( param->claw_fifo, cmd2 );
            else printf( "claw_fifo queue full\nHW6> " );
            break;
          }
          break;
        }
        case 'd':
        {
          if(param->mode == 1)
          {  
            cmd2.command = 'd';
            cmd2.argument = 0;
            if(!FIFO_FULL(param->motor_control_fifo))
            {FIFO_INSERT(param->motor_control_fifo,cmd2);}
            break;
          }
          else if (param->mode == 3) {
            cmd2.command = 'd';
            cmd2.argument = 1; // move spin servo
            if(!FIFO_FULL(param->arm_fifo)) FIFO_INSERT(param->arm_fifo, cmd2);
            else printf("arm_fifo queue full\nHW6> ");
            break;
          }
          break;
        }
        case 'i':
        {
          if (param->mode == 1 || param->mode == 2) {
            cmd2.command = 'i';
            cmd2.argument = 0;
            if(!FIFO_FULL(param->motor_control_fifo))
            {FIFO_INSERT(param->motor_control_fifo,cmd2);}
            break;
          } else if (param->mode == 3) {
            cmd2.command = 'i';
            cmd2.argument = 3; // move up/down servo
            if(!FIFO_FULL(param->arm_fifo)) FIFO_INSERT(param->arm_fifo, cmd2);
            else printf("arm_fifo queue full\nHW6> ");
            break;
          }
          break;
        }
        case 'j':
        {
          if (param->mode == 1 || param->mode == 2) {
            cmd2.command = 'j';
            cmd2.argument = 0;
            if(!FIFO_FULL(param->motor_control_fifo))
            {FIFO_INSERT(param->motor_control_fifo,cmd2);}
            break;
          } else if (param->mode == 3) {
            cmd2.command = 'j';
            cmd2.argument = 3; // move up/down servo
            if(!FIFO_FULL(param->arm_fifo)) FIFO_INSERT(param->arm_fifo, cmd2);
            else printf("arm_fifo queue full\nHW6> ");
            break;
          }
        }
        case 'x':
        {
          if (param->mode == 1 || param->mode == 2) {
            cmd2.command = 'x';
            cmd2.argument = 0;
            if(!FIFO_FULL(param->motor_control_fifo))
            {FIFO_INSERT(param->motor_control_fifo,cmd2);}
            break;
          } else if (param->mode == 3) {
            cmd2.command = 'x';
            cmd2.argument = 2; // move forward/backward servo
            if(!FIFO_FULL(param->arm_fifo)) FIFO_INSERT(param->arm_fifo, cmd2);
            else printf("arm_fifo queue full\nHW6> ");
            break;
          }
        }
        case 'o':
        { 
          if (param->mode == 1 || param->mode == 2) {
            cmd2.command = 'o';
            cmd2.argument = 0;
            if(!FIFO_FULL(param->motor_control_fifo))
            {FIFO_INSERT(param->motor_control_fifo,cmd2);}
            break;
          } else if (param->mode == 3) {
            cmd2.command = 'o';
            cmd2.argument = 5;
            if(!FIFO_FULL(param->arm_fifo)) FIFO_INSERT(param->arm_fifo, cmd2);
            else {
                printf("arm_fifo queue full\nHW6> ");
                break;
            }
            if(!FIFO_FULL(param->pwm_servo_fifo)) FIFO_INSERT(param->pwm_servo_fifo, cmd2);
            else printf("pwm_servo_fifo queue full\nHW6> ");
            break;
          }
        }
        case 'k':
        {
          if (param->mode == 1 || param->mode == 2) {
            cmd2.command = 'k';
            cmd2.argument = 0;
            if(!FIFO_FULL(param->motor_control_fifo))
            {FIFO_INSERT(param->motor_control_fifo,cmd2);}
            break;
          } else if (param->mode == 3) {
            cmd2.command = 'k';
            cmd2.argument = 5;
            if(!FIFO_FULL(param->arm_fifo)) FIFO_INSERT(param->arm_fifo, cmd2);
            else {
                printf("arm_fifo queue full\nHW6> ");
                break;
            }
            if(!FIFO_FULL(param->pwm_servo_fifo)) FIFO_INSERT(param->pwm_servo_fifo, cmd2);
            else printf("pwm_servo_fifo queue full\nHW6> ");
            break;
          }
      }
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
        {
          //pause the car first when switching mode
          cmd2.command = 's';
          cmd2.argument= 0;
          if(!FIFO_FULL(param->motor_control_fifo))
          {FIFO_INSERT(param->motor_control_fifo,cmd2);}
          change = true;
          break;
        }
        case 'f': //turn and drop eggs to the right
        {
          if (param->mode == 3) {
            cmd2.command = 'f';
            cmd2.argument = 0; // not needed for egg drop
            if(!FIFO_FULL(param->arm_fifo)) FIFO_INSERT(param->arm_fifo, cmd2);
            else printf("arm_fifo queue full\nHW6> ");
            if(!FIFO_FULL(param->pwm_servo_fifo)) FIFO_INSERT(param->pwm_servo_fifo, cmd2);
            else printf("pwm_servo_fifo queue full\nHW6> ");
            if(!FIFO_FULL(param->claw_fifo)) FIFO_INSERT(param->claw_fifo, cmd2);
            else printf("claw_fifo queue full\nHW6> ");
            break;
          }
          break;
        }
        case 'g': //turn and drop eggs to the left
        {
          if (param->mode == 3) {
            cmd2.command = 'g';
            cmd2.argument = 0; // not needed for egg drop
            if(!FIFO_FULL(param->arm_fifo)) FIFO_INSERT(param->arm_fifo, cmd2);
            else printf("arm_fifo queue full\nHW6> ");
            if(!FIFO_FULL(param->pwm_servo_fifo)) FIFO_INSERT(param->pwm_servo_fifo, cmd2);
            else printf("pwm_servo_fifo queue full\nHW6> ");
            if(!FIFO_FULL(param->claw_fifo)) FIFO_INSERT(param->claw_fifo, cmd2);
            else printf("claw_fifo queue full\nHW6> ");
            break;
          }
          break;
        }
        default: //if no command entered
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

void *Arm_Thread(void * args)
{
  struct arm_thread_param * param = (struct arm_thread_param * )args;
  struct  thread_command cmd = {0, 0};
  int angles[3] = {
      SPIN_RESET, // 90 degrees
      BACK_FORTH_RESET, // 135 degrees
      UP_DOWN_RESET // 75 degrees
  }; // initiail angles for the arm
  int angle_change = 5; // angle change for each command
  bool left_right = false; // true for left, false for right
  // used to wake up every 10ms with wait_period() function,
  // similar to interrupt occuring every 10ms
  struct  timespec  timer_state;

  // start 10ms timed wait, ie. set interrupt
  wait_period_initialize( &timer_state );
  wait_period( &timer_state, 10u ); /* 10ms */
  while (!*(param->quit_flag))
  {
    if (*(param->drop_stage) == 0) { // not dropping egg
      if (!(FIFO_EMPTY(param->fifo)))
      {
        FIFO_REMOVE(param->fifo, &cmd);  // read once every 10ms
        printf( "\n %s= %c  %d\n", param->name, cmd.command, cmd.argument);
        switch (cmd.command)
        {
          case 'w': // move arm forward
          {
            if (angles[cmd.argument - 1] - angle_change < BACK_FORTH_MIN) angles[cmd.argument - 1] = BACK_FORTH_MIN; // set to minimum angle
            else angles[cmd.argument - 1] -= angle_change; // decrease angle by angle_change degrees
            printf("Turning left, new angles: [%d, %d, %d]\n", angles[0], angles[1], angles[2]);
            set_angles(param->uart_fd, angles, ARM_TIMEOUT);
            break;
          }
          case 'x': // move arm backward
          {
            if (angles[cmd.argument - 1] + angle_change > BACK_FORTH_MAX) angles[cmd.argument - 1] = BACK_FORTH_MAX; // set to maximum angle
            else angles[cmd.argument - 1] += angle_change; // increase angle by angle_change degrees
            printf("Turning right, new angles: [%d, %d, %d]\n", angles[0], angles[1], angles[2]);
            set_angles(param->uart_fd, angles, ARM_TIMEOUT);
            break;
          }
          case 'd': // turn right
          {
            if (angles[cmd.argument - 1] - angle_change < SPIN_MIN) angles[cmd.argument - 1] = SPIN_MIN; // set to minimum angle
            else angles[cmd.argument - 1] -= angle_change; // decrease angle by angle_change degrees
            printf("Turning left, new angles: [%d, %d, %d]\n", angles[0], angles[1], angles[2]);
            set_angles(param->uart_fd, angles, ARM_TIMEOUT);
            break;
          }
          case 'a': // turn left
          {
            if (angles[cmd.argument - 1] + angle_change > SPIN_MAX) angles[cmd.argument - 1] = SPIN_MAX; // set to maximum angle
            else angles[cmd.argument - 1] += angle_change; // increase angle by angle_change degrees
            printf("Turning right, new angles: [%d, %d, %d]\n", angles[0], angles[1], angles[2]);
            set_angles(param->uart_fd, angles, ARM_TIMEOUT);
            break;
          }
          case 'i': // move arm up
          {
            if (angles[cmd.argument - 1] - angle_change < UP_DOWN_MIN) angles[cmd.argument - 1] = UP_DOWN_MIN; // set to minimum angle
            else angles[cmd.argument - 1] -= angle_change; // decrease angle by angle_change degrees
            printf("Moving up, new angles: [%d, %d, %d]\n", angles[0], angles[1], angles[2]);
            set_angles(param->uart_fd, angles, ARM_TIMEOUT);
            break;
          }
          case 'j': // move arm down
          {
            if (angles[cmd.argument - 1] + angle_change > UP_DOWN_MAX) angles[cmd.argument - 1] = UP_DOWN_MAX; // set to maximum angle
            else angles[cmd.argument - 1] += angle_change; // increase angle by angle_change degrees
            printf("Moving down, new angles: [%d, %d, %d]\n", angles[0], angles[1], angles[2]);
            set_angles(param->uart_fd, angles, ARM_TIMEOUT);
            break;
          }
          case 'o': // increase angle by 5
          {
            if (angle_change + cmd.argument > 180) angle_change = 180; // set to maximum angle
            else angle_change += cmd.argument; // increase angle by cmd.argument degrees
            printf("Increasing angle change to %d degrees\n", angle_change);
            break;
          }
          case 'k': // decrease angle by 5
          {
            if (angle_change - cmd.argument < 0) angle_change = 0; // set to minimum angle
            else angle_change -= cmd.argument; // decrease angle by cmd.argument degrees
            printf("Decreasing angle change to %d degrees\n", angle_change);
            break;
          }
          case 's': // reset arm to initial angles
          {
            angles[0] = SPIN_RESET;
            angles[1] = BACK_FORTH_RESET;
            angles[2] = UP_DOWN_RESET;
            printf("Resetting arm to initial angles\n");
            set_angles(param->uart_fd, angles, ARM_TIMEOUT);
            break;
          }
          case 'f':
          {
            left_right = true; // set left/right flag to true for egg drop
            // set angles to 90-90-90
            angles[0] = SPIN_MOTOR_TEMP_REST;
            angles[1] = BACK_FORTH_MOTOR_TEMP_REST;
            angles[2] = UP_DOWN_MOTOR_TEMP_REST; // set temporary rest angles for egg drop
            set_angles(param->uart_fd, angles, ARM_TIMEOUT);
            sleep(1); // assure arm does not move foe 1 second
            *(param->drop_stage) = 1; // set drop stage to 1
            break;
          }
          case 'g':
          {
            left_right = false; // set left/right flag to false for egg drop
            angles[0] = SPIN_MOTOR_TEMP_REST;
            angles[1] = BACK_FORTH_MOTOR_TEMP_REST;
            angles[2] = UP_DOWN_MOTOR_TEMP_REST; // set temporary rest angles for egg drop
            set_angles(param->uart_fd, angles, ARM_TIMEOUT);
            sleep(1); // assure arm does not move foe 1 second
            *(param->drop_stage) = 1; // set drop stage to 1
            break;
          }
          default:
          {
            printf("Invalid command for arm thread: %c\n", cmd.command);
          }
        }
      }
    }
    else if (*(param->drop_stage) == 1) { // stage 1 (move to basket)
      if (left_right) { // drop egg to the left
        angles[0] = SPIN_MOTOR_LEFT;
        angles[1] = BACK_FORTH_MOTOR_LEFT;
        angles[2] = UP_DOWN_MOTOR_LEFT; // set angles for left drop
      }
      else { // drop egg to the right
        angles[0] = SPIN_MOTOR_RIGHT;
        angles[1] = BACK_FORTH_MOTOR_RIGHT;
        angles[2] = UP_DOWN_MOTOR_RIGHT;
      }
      set_angles(param->uart_fd, angles, ARM_TIMEOUT*2);
      //sleep(1); // assure arm does not move for 1 second
      *(param->drop_stage) = 2; // set drop stage to 2
    } // no cases for stage 2 (wait for other threads to update)
    wait_period( &timer_state, 10u ); /* 10ms */
  }
  printf( "Arm thread function done\n" );
  return NULL;
}

void *Claw_Thread(void * args)
{
  struct arm_thread_param * param = (struct arm_thread_param * )args;
  struct  thread_command cmd = {0, 0};
  int claw_pos = CLAW_OPEN; // initial claw position
  // used to wake up every 10ms with wait_period() function,
  // similar to interrupt occuring every 10ms
  struct  timespec  timer_state;

  // start 10ms timed wait, ie. set interrupt
  wait_period_initialize( &timer_state );
  wait_period( &timer_state, 10u ); /* 10ms */
  while (!*(param->quit_flag))
  {
    if (*(param->drop_stage) == 0) { // not dropping egg
      if (!(FIFO_EMPTY(param->fifo)))
      {
        FIFO_REMOVE(param->fifo, &cmd);  // read once every 10ms
        printf( "\n %s= %c  %d\n", param->name, cmd.command, cmd.argument);
        switch (cmd.command)
        {
          case 'c': // toggle claw
          {
            claw_pos = (claw_pos == CLAW_OPEN) ? CLAW_CLOSE : CLAW_OPEN; // toggle claw position
            printf("Toggling claw, new position: %d\n", claw_pos);
            set_claw(param->uart_fd, claw_pos, ARM_CLAW_TIMEOUT);
            break;
          }
          case 's': // reset claw to open position
          {
            claw_pos = CLAW_OPEN;
            printf("Resetting claw to open position\n");
            set_claw(param->uart_fd, claw_pos, ARM_CLAW_TIMEOUT);
            break;
          }
          default:
          {
            printf("Invalid command for claw thread: %c\n", cmd.command);
          }
        }
      }
    }
    else if (*(param->drop_stage) == 2) { // stage 2 (open claw to drop egg)
      if (claw_pos != CLAW_OPEN) { // open claw if not already open
        claw_pos = CLAW_OPEN;
        printf("Opening claw to drop egg\n");
        set_claw(param->uart_fd, claw_pos, ARM_CLAW_TIMEOUT);
        sleep(1); // assure claw is open for 1 second
      }
      *(param->drop_stage) = 0; // reset drop stage to 0 (drop done)
     
    } // no cases for stage 1 (wait for other threads to update)
    wait_period( &timer_state, 10u ); /* 10ms */
  }
  printf( "Claw thread function done\n" );
  return NULL;
}

void *PWM_Servo_Thread(void * args)
{
  struct arm_thread_param * param = (struct arm_thread_param * )args;
  struct  thread_command cmd = {0, 0};
  int pwm_servo_angle = PWM_SERVO_RESET; // initial pwm servo angle
  int angle_change = 5; // angle change for each command
  bool left_right = false; // true for left, false for right
  // used to wake up every 10ms with wait_period() function,
  // similar to interrupt occuring every 10ms
  struct  timespec  timer_state;

  // start 10ms timed wait, ie. set interrupt
  wait_period_initialize( &timer_state );
  wait_period( &timer_state, 10u ); /* 10ms */
  while (!*(param->quit_flag))
  {
    if (*(param->drop_stage) == 0) { // not dropping egg
      if (!(FIFO_EMPTY(param->fifo)))
      {
        FIFO_REMOVE(param->fifo, &cmd);  // read once every 10ms
        printf( "\n %s= %c  %d\n", param->name, cmd.command, cmd.argument);
        switch (cmd.command)
        {
          case 'e': // turn pwm servo left
          {
            if (pwm_servo_angle - angle_change < PWM_SERVO_MIN) pwm_servo_angle = PWM_SERVO_MIN; // set to minimum angle
            else pwm_servo_angle -= angle_change; // decrease angle by angle_change degrees
            printf("Turning pwm servo left, new angle: %d\n", pwm_servo_angle);
            set_pwmservo(param->uart_fd, pwm_servo_angle, PWM_SERVO_TIMEOUT);
            break;
          }
          case 'r': // turn pwm servo right
          {
            if (pwm_servo_angle + angle_change > PWM_SERVO_MAX) pwm_servo_angle = PWM_SERVO_MAX; // set to maximum angle
            else pwm_servo_angle += angle_change; // increase angle by angle_change degrees
            printf("Turning pwm servo right, new angle: %d\n", pwm_servo_angle);
            set_pwmservo(param->uart_fd, pwm_servo_angle, PWM_SERVO_TIMEOUT);
            break;
          }
          case 'o': // increase pwm servo angle by 5 degrees
          {
            if (angle_change + cmd.argument > PWM_SERVO_MAX) angle_change = PWM_SERVO_MAX; // set to maximum angle
            else angle_change += cmd.argument; // increase angle by angle_change degrees
            printf("Increasing pwm servo angle to %d degrees\n", angle_change);
            break;
          }
          case 'k': // decrease pwm servo angle by 5 degrees
          {
            if (angle_change - cmd.argument < PWM_SERVO_MIN) angle_change = PWM_SERVO_MIN; // set to minimum angle
            else angle_change -= cmd.argument; // decrease angle by angle_change degrees
            printf("Decreasing pwm servo angle to %d degrees\n", angle_change);
            break;
          }
          case 's': // reset pwm servo to initial angle
          {
            pwm_servo_angle = PWM_SERVO_RESET;
            printf("Resetting pwm servo to initial angle\n");
            set_pwmservo(param->uart_fd, pwm_servo_angle, PWM_SERVO_TIMEOUT);
            break;
          }
          case 'f': // turn and drop eggs to the left
          {
            left_right = false; // set left/right flag to false for egg drop
            pwm_servo_angle = PWM_SERVO_RESET; // set angle for left drop
            printf("Turning pwm servo left for egg drop\n");
            set_pwmservo(param->uart_fd, pwm_servo_angle, PWM_SERVO_TIMEOUT);
            //sleep(1); // assure servo does not move for 1 second
            *(param->drop_stage) = 1; // set drop stage to 1 (either the arm does it first or the pwm servo does it first)
            break;
          }
          case 'g': // turn and drop eggs to the right
          {
            left_right = true; // set left/right flag to true for egg drop
            pwm_servo_angle = PWM_SERVO_RESET; // set angle for right drop
            printf("Turning pwm servo right for egg drop\n");
            set_pwmservo(param->uart_fd, pwm_servo_angle, PWM_SERVO_TIMEOUT);
            //sleep(1); // assure servo does not move for 1 second
            *(param->drop_stage) = 1; // set drop stage to 1 (either the arm does it first or the pwm servo does it first)
            break;
          }
          default:
          {
            printf("Invalid command for pwm servo thread: %c\n", cmd.command);
          }
        }
      }
    }
    else if (*(param->drop_stage) == 1) { // stage 1 (move to basket)
      if (left_right) { // drop egg to the left
        pwm_servo_angle = PWM_SERVO_LEFT; // set angle for left drop
      }
      else { // drop egg to the right
        pwm_servo_angle = PWM_SERVO_RIGHT; // set angle for right drop
      }
      set_pwmservo(param->uart_fd, pwm_servo_angle, ARM_TIMEOUT*2);
      sleep(1); // assure servo does not move for 1 second
      *(param->drop_stage) = 2; // set drop stage to 2 (wait for claw to open)
    } // no cases for stage 2 (wait for other threads to update)
    wait_period( &timer_state, 10u ); /* 10ms */
  }
  printf( "PWM Servo thread function done\n" );
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
  int speed;
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
          speed = cmd.argument;
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
  int busy2; // used to store the busy time for turning left/righ
  int busy1; // used to store the busy time for turning left/right
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
      {
      //change the direction first
        busy2 = (int)(param->angle*1.5);
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
      }
      case 's':
        if(!FIFO_FULL(param->dir_fifo)){
          cmd2.command   = 's';
          cmd2.argument = 0;
          FIFO_INSERT(param->dir_fifo,cmd2);
        }
        prev_dir = 's';
        break;
      case 'd':
        busy1 = (int)(param->angle*1.5);
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
  struct video_interface_handle_t *         handle_video2;

  handle_video1 = video_interface_open( "/dev/video0" );
  video_interface_print_modes(handle_video1);

  if(!video_interface_set_mode_auto(handle_video1)){
    printf("failed to configure dev/video0 \n");
    return NULL;
  }
  
  
  handle_video2 = video_interface_open( "/dev/video2" );
  video_interface_print_modes(handle_video2);
  if(!video_interface_set_mode_auto(handle_video2)){
    printf("failed to configure dev/video2 \n");
    return NULL;
  }


  // handle_video2 = video_interface_open( "/dev/video2");
  // video_interface_print_modes(handle_video2);
  // if(!video_interface_set_mode_manual(handle_video2,3)){
  //   printf("failed to configure dev/video2 \n");
  //   return NULL;
  // }

  struct  timespec  timer_state; 
             // used to wake up every 10ms with wait_period() function,
             // similar to interrupt occuring every 10ms

  // start 10ms timed wait
  wait_period_initialize( &timer_state );
  wait_period( &timer_state, 100u ); /* 500 ms */
  int scaled_width      = handle_video1->configured_width/SCALE_REDUCTION_PER_AXIS;
  int scaled_height     = handle_video1->configured_height/SCALE_REDUCTION_PER_AXIS;
  printf("img size = %ld x %ld\n", handle_video1->configured_width,handle_video1->configured_height);
  printf("size of image_t %ld\n", IMAGE_SIZE);
  
  int argc = 0;
  char *argv[3];
  int counter = 0;
  printf("scaled height: %d scaled width %d", scaled_height,scaled_width);
  while(! *param->quit_flag){
    counter++;
    if(counter % 10 == 0)
      {
        if (video_interface_get_image(handle_video1, param->image1) 
        ){//
         

        
          // scale_image_data(
          //   (struct pixel_format_RGB *)param->image1,
          //   handle_video1->configured_height,
          //   handle_video1->configured_width,
          //   param->img_data1,
          //   SCALE_REDUCTION_PER_AXIS,
          //   SCALE_REDUCTION_PER_AXIS
          // );
         
              
            struct thread_command cmd = {'u',2};
            memcpy(param->rgb_raw,(unsigned char *)param->image1,IMAGE_SIZE);
            memcpy(param->greyscale_raw,(unsigned char*)param->image1,IMAGE_SIZE);
            memcpy(param->bw_raw,(unsigned char *)param->image1,IMAGE_SIZE);
            memcpy(param->reduced_raw,(unsigned char *)param->image1,IMAGE_SIZE);
            memcpy(param->arm_img_raw, (unsigned char *)param->image2,IMAGE_SIZE);

            FIFO_INSERT(param->rgb_cmd_fifo,cmd);
            FIFO_INSERT(param->greyscale_cmd_fifo,cmd);
            FIFO_INSERT(param->bw_cmd_fifo,cmd);
            FIFO_INSERT(param->reduced_cmd_fifo,cmd);
            FIFO_INSERT(param->hist_fifo,cmd);
            FIFO_INSERT(param->egg_fifo,cmd);
          

          
          
        }
        else
        {
          printf("robot did not image \n");
        }
        if(video_interface_get_image(handle_video2, param->image2)){
          struct thread_command cmd = {'u',1};
          
          memcpy(param->arm_img_raw, (unsigned char *)param->image2,IMAGE_SIZE);
          FIFO_INSERT(param->egg_fifo,cmd);

        }
        else{
          printf("arm camera did not get image");
        }
      }
      
    struct thread_command cmd1 = {0,0};
    struct thread_command cmd2 = {0,0};
    if(!FIFO_EMPTY(param->img_cmd_fifo))
    {
      FIFO_REMOVE(param->img_cmd_fifo,&cmd1);
      printf( "\n %s= %c  %c\n", param->name, cmd1.command, cmd1.argument);
      switch(cmd1.command)
      {
        
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
    struct draw_bitmap_multiwindow_handle_t *arm_handle = NULL;

    unsigned char * egg_buffer_1 = (unsigned char *)malloc(IMAGE_SIZE+1);
    struct pixel_format_RGB * egg_data_1 = (struct pixel_format_RGB *)egg_buffer_1;

    unsigned char * egg_buffer_2 = (unsigned char *)malloc(IMAGE_SIZE +1);
    struct pixel_format_RGB *egg_data_2 = (struct pixel_format_RGB *)egg_buffer_2;

    int x = 0;
    int y = 0;
    bool pause_thread = true;
    int min_x = IMG_WIDTH, min_y = IMG_HEIGHT;
    int max_x = 0, max_y = 0;

    int robot_decision_q[MAX_DECISION_SIZE] = {0}; 
    point arm_decision_q[MAX_DECISION_SIZE] = {0};

    bool robot_queue_filled = false;
    int robot_frames_seen = 0;

    bool arm_queue_filled = false;
    int arm_frames_seen = 0;

    bool robot_stopped = false;
    bool robot_centered = false;

    bool arm_centered_x = false;
    bool arm_centered_y = false;
    bool arm_stopped = false;

    bool egg_found = false;
    int turn_cool_down = 0;

    int arm_cool_down_x = 0;
    int arm_cool_down_y = 0;
    bool mode3 = false;
    wait_period(&timer_state, 10u);

    while(!(*param->quit_flag))
    {
      struct thread_command cmd = {0, 0};
      if(!FIFO_EMPTY(param->egg_fifo))
      {
        FIFO_REMOVE(param->egg_fifo,&cmd);
        switch (cmd.command)
        {
          case 'w':{
            pause_thread = false;
            printf( "\n %s= %c  %c\n", param->name, cmd.command, cmd.argument);//robot image
            printf("starting egg detection \n");
            break;
          }
          case 's':{
            printf( "\n %s= %c  %c\n", param->name, cmd.command, cmd.argument);//arm image
            printf("stop egg detection \n");
            break;
          }
          case 'e':{
            if(handle == NULL)
            {
              handle = draw_bitmap_create_window(IMG_WIDTH,IMG_HEIGHT);
              arm_handle = draw_bitmap_create_window(IMG_WIDTH,IMG_HEIGHT);    
            }
            else{
              draw_bitmap_close_window(handle);
              draw_bitmap_close_window(arm_handle);
              handle = NULL;
              arm_handle = NULL;
            }
            break;
          }
          case 'u':{
            if(cmd.argument == 1){
             memcpy(egg_buffer_1,param->RGB_IMG_raw,IMAGE_SIZE);
            }
            else{
              memcpy(egg_buffer_2,param->ARM_IMG_RAW,IMAGE_SIZE);
            }    
            if(!mode3){
              // ---------------Robot Mode-----------------
              robot_frames_seen++;
              if (robot_frames_seen >= MAX_DECISION_SIZE) {
                robot_queue_filled = true;
              }
              memcpy(egg_buffer_1,param->RGB_IMG_raw,IMAGE_SIZE);
              to_black_white(egg_data_1, IMAGE_SIZE/3, EGG_THRESHOLD);
              x = 0;
              y = 0;
              min_x = IMG_WIDTH;min_y = IMG_HEIGHT; max_x =0;max_y =0;
              int count = 0;
              EggBlob eggs[MAX_EGGS];
              int found = find_egg_blobs(egg_data_1,eggs,MAX_EGGS,IMG_WIDTH,IMG_HEIGHT);
              //printf("found %d eggs \n", found);
              int max_egg = 0;

              //draw red and green bbox 
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
              //move the que forward by one position
              for (int i = 0; i < MAX_DECISION_SIZE - 1; i++) {
                  robot_decision_q[i] = robot_decision_q[i + 1];
              }
              if(found > 0){
                int min_x = eggs[max_egg].min_x;
                int max_x = eggs[max_egg].max_x;
                int min_y = eggs[max_egg].min_y;
                int max_y = eggs[max_egg].max_y;
                draw_bbox(min_x,min_y,max_x,max_y,egg_data_1,(struct pixel_format_RGB){0, 255, 0});
                robot_decision_q[MAX_DECISION_SIZE-1] = eggs[max_egg].center_x;
                printf("largest egg size is is at %d \n", eggs[max_egg].size);
                egg_found = true;
              }
              else{
                robot_decision_q[MAX_DECISION_SIZE-1] = -1;
                egg_found = false;
                //printf("did not find any eggs \n");
              }
              

              if(!pause_thread && robot_queue_filled){
                int left = 0, right = 0, center,not_found = 0;
                for (int i = 0; i < MAX_DECISION_SIZE; i++) {
                    if (robot_decision_q[i] > 0 && robot_decision_q[i] < CENTER_L) left++;
                    else if (robot_decision_q[i] > CENTER_R) right++;
                    else if (robot_decision_q[i] == -1) not_found++;
                    else center++;
                }
                
                if(eggs[max_egg].size > STOP_THRESH){ //check if the egg is close enough
                  if(!FIFO_FULL(param->dir_fifo)){
                    printf("robot: egg size is %d egg is close to robot\n",eggs[max_egg].size);
                    cmd.command = 's';
                    if(!robot_stopped)
                    {
                      FIFO_INSERT(param->dir_fifo,cmd);
                    }
                    robot_stopped = true;
                  }
                }

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
                    printf("robot: robot did not find any egg \n");
                    robot_stopped = false;
                  }  
                }
                else if (left >= MAX_DECISION_SIZE/2) {
                  if (!FIFO_FULL(param->dir_fifo)) {
                      
                      printf("robot queue decision: largest egg detected on the left\n");                    
                      cmd.command = 'd';
                      FIFO_INSERT(param->dir_fifo, cmd);
                      //centered = false;
                      turn_cool_down = TURN_COOLDOWN_FRAMES;                      
                  }
                } else if (right >= MAX_DECISION_SIZE/2) {
                  if (!FIFO_FULL(param->dir_fifo)) {
                      printf("robot queue decision: largest egg detected on the right\n");
                      cmd.command = 'a';
                      FIFO_INSERT(param->dir_fifo, cmd);
                      turn_cool_down = TURN_COOLDOWN_FRAMES;
                  }
                }
                else if(center >=MAX_DECISION_SIZE/2){
                  robot_centered = true;
                  printf("robot: egg is at the center of the robot\n");
                }
                if(robot_centered && robot_stopped){
                  printf("robot is close enough to grab the egg \n");
                  if(!FIFO_FULL(param->dir_fifo)){
                    cmd.command ='s';
                    FIFO_INSERT(param->dir_fifo,cmd);
                  }
                  if(!FIFO_FULL(param->control_fifo)){
                    cmd.command = 'm';
                    cmd.argument = 0;
                    FIFO_INSERT(param->control_fifo,cmd);
                    cmd.command = '3';
                    cmd.argument = 0;
                    FIFO_INSERT(param->control_fifo,cmd);

                    printf("switched robot mode to mode3 \n");
                    mode3 = true;
                    //move the arm forward to try to get closer to the egg
                    cmd.command = 'w';
                    cmd.argument = 0; //move the arm forward
                    fifo_insert(param->control_fifo,cmd);
                    cmd.command = 'w';
                    cmd.argument = 0; //move the arm forward
                    fifo_insert(param->control_fifo,cmd);
                  }
                }
              }
            }
            else{
              //-----------------------arm mode --------------------------
              arm_frames_seen++;
              if (arm_frames_seen >= MAX_DECISION_SIZE) {
                arm_queue_filled = true;
              }
              to_black_white(egg_data_2, IMAGE_SIZE/3, EGG_THRESHOLD);
              int count = 0;
              EggBlob arm_eggs[1];
              int found = find_egg_blobs(egg_data_2,arm_eggs,MAX_EGGS,IMG_WIDTH,IMG_HEIGHT);

              int min_x = arm_eggs[0].min_x;
              int max_x = arm_eggs[0].max_x;
              int min_y = arm_eggs[0].min_y;
              int max_y = arm_eggs[0].max_y;

              // Draw red bounding box
              draw_bbox(min_x,min_y,max_x,max_y, egg_data_2,(struct pixel_format_RGB){255, 0, 0});

              for (int i = 0; i < MAX_DECISION_SIZE - 1; i++) {
                arm_decision_q[i] = arm_decision_q[i + 1];
              }
              if(found){
                arm_decision_q[MAX_DECISION_SIZE-1].x = arm_eggs[0].center_x;
                arm_decision_q[MAX_DECISION_SIZE-1].y = arm_eggs[0].center_y;
              }          
              else{
                arm_decision_q[MAX_DECISION_SIZE-1].x = -1;
                arm_decision_q[MAX_DECISION_SIZE-1].y = -1;
               }

              if(arm_queue_filled && !pause_thread){ 
                int left = 0, right = 0; int center_x =0 ;int not_found = 0; int front = 0; int back = 0,center_y =0;
                for (int i = 0; i < MAX_DECISION_SIZE; i++) {
                    if (arm_decision_q[i].x > 0 && arm_decision_q[i].x < CENTER_L) left++;
                    else if (arm_decision_q[i].x > CENTER_R) right++;
                    else if (arm_decision_q[i].x && arm_decision_q[i].y == -1) not_found++;
                    else center_x++;

                    int cur_y = arm_decision_q[i].y;
                    if(cur_y > 0 && cur_y < CENTER_F) front ++;
                    else if(cur_y > CENTER_B) back++;
                    else center_y++;
                }
                
                if(arm_eggs[0].size > ARM_STOP_THRESH){ //check if the egg is close enough
                  printf("arm: egg size is %d egg is close to robot\n",arm_eggs[0].size);
                  arm_stopped = true;    
                }
                else{

                }
                //check x direction
                if(arm_cool_down_x > 0)
                {
                  arm_cool_down_x--;
                }
                else if(not_found > MAX_DECISION_SIZE/2){
                  if(!FIFO_FULL(param->control_fifo)){
                    //move the robot robot to the front a little bit
                    //move the arm to the front a little bit
                    cmd.command ='w';
                    FIFO_INSERT(param->control_fifo,cmd);
                    cmd.command ='j';
                    FIFO_INSERT(param->control_fifo,cmd);
                    cmd.command = 'm';
                    FIFO_INSERT(param->control_fifo,cmd);
                    cmd.command = '1';
                    FIFO_INSERT(param->control_fifo,cmd);
                    cmd.command = 'w';
                    FIFO_INSERT(param->control_fifo,cmd);
                    cmd.command = 'w';
                    FIFO_INSERT(param->control_fifo,cmd);
                    cmd.command = 's';
                    FIFO_INSERT(param->control_fifo,cmd);
                    cmd.command = 'm';
                    FIFO_INSERT(param->control_fifo,cmd);
                    cmd.command = '3';
                    FIFO_INSERT(param->control_fifo,cmd);
                    turn_cool_down = TURN_COOLDOWN_FRAMES;
                    printf("arm: robot did not find any egg \n");
                    arm_stopped = false;
                    break;
                  }  
                }
                else if (left >= MAX_DECISION_SIZE/2) {
                  if (!FIFO_FULL(param->control_fifo)) {
                      
                    printf("arm queue decision: largest egg detected on the left\n");                    
                    cmd.command = 'd';
                    FIFO_INSERT(param->control_fifo, cmd);
                    //centered = false;
                    turn_cool_down = TURN_COOLDOWN_FRAMES;                      
                  }
                } else if (right >= MAX_DECISION_SIZE/2) {
                  if (!FIFO_FULL(param->control_fifo)) {
                      printf("arm queue decision: largest egg detected on the right\n");
                      cmd.command = 'a';
                      FIFO_INSERT(param->control_fifo, cmd);
                      turn_cool_down = TURN_COOLDOWN_FRAMES;
                  }
                }
                else if(center_x >= MAX_DECISION_SIZE/2){
                  arm_centered_x = true;
                  printf("arm: egg is at the center of the of arm in the x direction \n");
                }
                //check y direction
                if(arm_cool_down_y > 0){
                  arm_cool_down_y--;
                }
                if(front >= MAX_DECISION_THRESHOLD){
                  printf("arm: arm is too far front");
                  cmd.command = 'x';
                  cmd.command =0;
                  fifo_insert(param->control_fifo,cmd);
                  arm_cool_down_y = TURN_COOLDOWN_FRAMES;
                }
                else if(back >= MAX_DECISION_THRESHOLD){
                  printf("arm: arm is too back");
                  cmd.command ='w';
                  cmd.argument = 0;
                  fifo_insert(param->control_fifo,cmd);
                  arm_cool_down_y = TURN_COOLDOWN_FRAMES;
                }
                else if(center_y >= MAX_DECISION_THRESHOLD){
                  printf("arm: egg is at the center of the arm in the y direction \n");
                  arm_centered_y =true;
                }

                if(arm_centered_x && arm_centered_y){
                  if(arm_stopped)
                  {
                    printf("arm: arm is close enough to grab the egg \n");
                  //todo: grab the egg
                    cmd.command = 'c';
                    cmd.argument = 0;
                    //grab the egg
                    fifo_insert(param->control_fifo,cmd);
                    #ifdef SINGLE_EGG
                      //flip 180 degree
                      cmd.command = 'w';
                      cmd.argument =0;
                      fifo_insert(param->dir_fifo,cmd);
                      usleep(100000); //needs to be modified
                      //flip 180 degree
                    #endif
                    //release the egg
                     
                    //
                    cmd.command = 'c';
                    cmd.argument = 0;
                    //grab the egg
                    fifo_insert(param->control_fifo,cmd);
                    cmd.command = 'm';
                    cmd.argument = 2;
                    fifo_insert(param->control_fifo,cmd);
                    printf("switched robot mode to mode 2\n");
                    mode3 = false;
                  }
                  else{
                    if(!FIFO_FULL(param->control_fifo)){
                    cmd.command= 'j';
                    cmd.argument =0;
                    FIFO_INSERT(param->control_fifo,cmd);
                    }
                  }
                }
              }
            }
            break;
          }
        }
      }
        //draw the center of the egg
      if(handle) draw_bitmap_display(handle,egg_data_1);
      if(arm_handle) draw_bitmap_display(arm_handle, egg_data_2);
      wait_period(&timer_state, 10u);
    }
    if (handle) draw_bitmap_close_window(handle);
    if(arm_handle) draw_bitmap_close_window(arm_handle);
    free(egg_buffer_1);
    free(egg_buffer_2);
    handle = NULL;
    arm_handle = NULL;
    printf("%s exited \n",param->name);
    return NULL;
}

void *bin_detector(void * arg){
  struct egg_detector_thread_param *param = (struct egg_detector_thread_param*)arg;
  struct  timespec  timer_state; 
  printf("%s thread started \n",param->name);
  wait_period_initialize( &timer_state );

  struct draw_bitmap_multiwindow_handle_t *handle =NULL;

  unsigned char * bin_buffer = (unsigned char *)malloc(IMAGE_SIZE+1);
  struct pixel_format_RGB * bin_data = (struct pixel_format_RGB *)bin_buffer;

  bool thread_active = false;
  wait_period(&timer_state, 10u);
  while(!(*param->quit_flag)){
    struct thread_command cmd = {0,0};
    if(!FIFO_EMPTY(param->egg_fifo)){
      FIFO_REMOVE(param->dir_fifo,&cmd);
      switch (cmd.command){
        case 'f':
          /* code */
          if(handle == NULL)
          {
            handle = draw_bitmap_create_window(IMG_WIDTH,IMG_HEIGHT);
            thread_active = true;
          }
          else{
            draw_bitmap_close_window(handle);
            thread_active = false;
            handle = NULL;
          }
          break;
        case 'u':
          if(thread_active){
            edge_to_red(param->RGB_IMG_data,bin_data);
          }
          
          break;
        default:
          break;
      }
    }
    if(handle) draw_bitmap_display(handle,bin_data);
    wait_period(&timer_state,10u);
  }
  
  
  free(bin_buffer);
  return NULL;
}


void fifo_insert(struct fifo_t* fifo, struct thread_command cmd){
  if(!FIFO_FULL(fifo)){
    FIFO_INSERT(fifo,cmd);
  }
  else{
    printf("fifo full \n");
  }
}