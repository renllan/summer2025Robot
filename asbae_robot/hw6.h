#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <termios.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>
#include <linux/videodev2.h>
#include <time.h>
#include <cairo/cairo.h>
#include <gtk/gtk.h>
#include "to_black_white.h"
#include "find_egg_blobs.h"
#include "../include/keypress.h"
#include "../include/import_registers.h"
#include "../include/cm.h"
#include "../include/gpio.h"
#include "../include/uart.h"
#include "../include/spi.h"
#include "../include/bsc.h"
#include "../include/pwm.h"
#include "../include/enable_pwm_clock.h"
#include "../include/io_peripherals.h"
#include "../include/wait_period.h"
#include "../include/FIFO.h"
#include "../include/MPU6050.h"
#include "../include/MPU9250.h"
#include "../include/wait_key.h"
#include "pixel_format_RGB.h"
#include "../include/video_interface.h"
#include "wait_key.h"
#include "scale_image_data.h"
#include "draw_bitmap_multiwindow.h"
#include "C_equiv_ctl.h"
#include "edge_detection.h"

#define SINGLE_EGG 

#define FIFO_LENGTH  1024
#define PWM_RANGE 100
#define SCALE_REDUCTION_PER_AXIS  2   /* the image size reduction ratio (note that 640*480*3*8*FPS = your bandwidth usage, at 24FPS, that is 177MPBS) */
#define GET_FRAMES                10  /* the number of frame times to average when determining the FPS */
#define IMAGE_SIZE                sizeof(struct image_t)
#define TURN_COOLDOWN_FRAMES 15
#define EGG_THRESHOLD 180  //
#define COUNT_THRESHOLD 500
#define IMG_WIDTH 320
#define IMG_HEIGHT 240
#define MAX_EGGS 10
#define STOP_THRESH 1600  //needs to be tuned
#define MAX_DECISION_SIZE 10
#define CENTER_L 140
#define CENTER_R 180
#define ARM_STOP_THRESH 10000  //needs to be tuned
#define CENTER_F 100
#define CENTER_B 140
#define MAX_DECISION_THRESHOLD MAX_DECISION_SIZE/2
#define DIFF_EGG_SIZE_THRESH 500

#define WOOD_L
#define WOOD_H

// Arm macros constants
#define SPIN_RESET 90
#define BACK_FORTH_RESET 125
#define UP_DOWN_RESET 85
#define SPIN_MIN 0 // max left
#define SPIN_MAX 180 // max right
#define BACK_FORTH_MIN 35 // max back
#define BACK_FORTH_MAX 125 // max forward
#define UP_DOWN_MIN 85 // max up
#define UP_DOWN_MAX 125 // max down
#define PWM_SERVO_RESET 90 // 180 for other car
#define PWM_SERVO_LEFT 180 // 120 for other car
#define PWM_SERVO_RIGHT 0 // 60 for other car
#define PWM_SERVO_MIN 0 // 0 for other car
#define PWM_SERVO_MAX 180 // 180 for other car
#define SPIN_REST 90
#define BACK_FORTH_REST 55
#define UP_DOWN_REST 90

#define SPIN_MOTOR_TEMP_REST 90
#define SPIN_MOTOR_LEFT1 130
#define SPIN_MOTOR_LEFT2 145 
#define SPIN_MOTOR_RIGHT1 50
#define SPIN_MOTOR_RIGHT2 25
#define BACK_FORTH_MOTOR_TEMP_REST 90
#define BACK_FORTH_MOTOR_RIGHT1 100
#define BACK_FORTH_MOTOR_RIGHT2 110
#define BACK_FORTH_MOTOR_LEFT1 100
#define BACK_FORTH_MOTOR_LEFT2 110
#define UP_DOWN_MOTOR_TEMP_REST 85
#define UP_DOWN_MOTOR_LEFT 85
#define UP_DOWN_MOTOR_RIGHT 85

#define CLAW_OPEN 325 // 500 for other car
#define CLAW_CLOSE 825 // 800 for other car
#define ARM_TIMEOUT 500 // Timeout for arm commands in milliseconds
#define PWM_SERVO_TIMEOUT 375 // Timeout for pwm servo commands in milliseconds
#define ARM_CLAW_TIMEOUT 375 // Timeout for arm claw commands in milliseconds
void *video_histogram(void *arg) ;
void set_gpio(struct io_peripherals *io);
void enable_pwm(struct io_peripherals *io) ;
void *black_and_white(void * arg);
void *reduced_video(void * arg);
void *video_with_cross(void * arg);
void *video_capture(void * arg);
void set_gpio_context(volatile struct gpio_register * gpio); 
void *Motor_Direction_Thread(void * arg);
void *Motor_Speed_Thread(void * args);
void *Motor_Control(void * arg);
void *IR_Sensor(void* arg);
void *KeyRead(void * arg);
void *greyscale_video(void * arg);
void *egg_detector(void * arg);
void *Control(void* arg);
void sigint_handler(int signum);
void *single_channel(void * arg);
void *Arm_Thread(void * args);
void *Claw_Thread(void * args);
void *PWM_Servo_Thread(void * args);

typedef struct{
  int x;
  int y;
}point;

struct thread_command
{
    uint8_t command;
    int argument;
};

FIFO_TYPE(struct thread_command, FIFO_LENGTH, fifo_t);

void fifo_insert(struct fifo_t* fifo,struct thread_command cmd);

struct arm_thread_param
{
  const char                    *name;
  struct fifo_t                 *fifo;
  int                            uart_fd; // UART file descriptor for arm control
  int                           *drop_stage; // 0 set 90-90-90, 1 move left or right & twist PWM, 2 open claw and rest position
  bool                          *quit_flag;
};

struct egg_detector_thread_param{

  const char                  *name;
  struct fifo_t               *egg_fifo;
  struct fifo_t               *dir_fifo;
  struct fifo_t               *control_fifo;
  unsigned char               *RGB_IMG_raw;
  struct pixel_format_RGB     *RGB_IMG_data;
  unsigned char               *ARM_IMG_RAW;
  struct pixel_format_RGB     *ARM_IMG_data;
  bool                        *quit_flag;
};

struct IR_Sensor_param
{
  const char                  *name;
  struct fifo_t              *IR_sensor_fifo;
  struct fifo_t              *dir_fifo;
  volatile struct gpio_register     *gpio;
  int                         pin_1;
  int                         pin_2;
  bool                       *quit_flag;
};

struct key_thread_param
{
  const char                    * name;
  struct fifo_t                 * key_fifo;
  bool                          * quit_flag;
};

struct motor_speed_thread_param
{
  const char                    * name;
  volatile struct gpio_register * gpio;
  volatile struct pwm_register  * pwm;
  int                             pin_1;
  int                             pin_2;
  struct fifo_t                 * speed_fifo;
  bool                          * quit_flag;
};

struct motor_direction_thread_param
{
  const char                     *name;
  volatile struct gpio_register  *gpio;
  int                            pin_1; //AI1
  int                            pin_2; //AI2
  int                            pin_3; //BI1
  int                            pin_4; //BI2 
  struct fifo_t                  *dir_fifo;             
  bool                           *quit_flag;
};

struct control_thread_param
{
  const char                    * name;
  struct fifo_t                 * key_fifo;
  struct fifo_t                 * control_fifo;
  struct fifo_t                 * IR_sensor_fifo;
  struct fifo_t                 * img_cmd_fifo;
  struct fifo_t                 * motor_control_fifo;
  struct fifo_t                 * arm_fifo; // FIFO for arm control commands
  struct fifo_t                 * pwm_servo_fifo; // FIFO for PWM control commands
  struct fifo_t                 * claw_fifo; // FIFO for claw control commands
  bool                          * quit_flag;
  int                           mode; //mode 1: mode1, 2: mode2, 3: mode3
};

struct motor_control_thread_param
{
  const char                    * name;
  struct fifo_t                 * motor_control_fifo;
  struct fifo_t                 * speed_fifo;
  struct fifo_t                 * dir_fifo;
  int                             angle;
  int                             pwm_val;
  bool                          * quit_flag;  
};

struct img_process_thread_param
{
  const char                  * name;
  struct fifo_t               * img_cmd_fifo;
  unsigned char               * RGB_IMG_raw;
  struct pixel_format_RGB     * RGB_IMG_data;
  unsigned int                  width;       
  unsigned int                  height;
  bool                        * quit_flag;
  bool                        status_flag;
};

struct reduced_img_thread_param{
  const char                  * name;
  struct fifo_t               * img_cmd_fifo;
  struct fifo_t               * dir_fifo;
  unsigned char              * RGB_IMG_raw;
  struct pixel_format_RGB     * RGB_IMG_data;
  unsigned int                  width;       
  unsigned int                  height;
  bool                        * quit_flag;
  bool                        status_flag;
};

struct img_capture_thread_param   
{
  const char                  * name;
  struct fifo_t               * img_cmd_fifo;
  struct fifo_t               * rgb_cmd_fifo;
  struct fifo_t               * greyscale_cmd_fifo;
  struct fifo_t               * bw_cmd_fifo;
  struct fifo_t               * reduced_cmd_fifo;
  struct fifo_t               * hist_fifo;
  struct fifo_t               * egg_fifo;
  struct image_t              *image1;
  struct image_t              *image2; //arm image
  unsigned char               *arm_img_raw;
  unsigned char               *rgb_raw;
  unsigned char               *greyscale_raw;
  unsigned char               *bw_raw;
  unsigned char               *reduced_raw;
  bool                       * quit_flag;
};

// int find_max(int hist[], int size) {
//     int max = 1;  // Minimum of 1 to avoid division by zero
//     for (int i = 0; i < size; i++) {
//         if (hist[i] > max) max = hist[i];
//     }
//     return max;
// }