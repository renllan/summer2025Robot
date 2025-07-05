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
#include <linux/videodev2.h>
#include <time.h>
#include <cairo/cairo.h>
#include <gtk/gtk.h>
#include "pixel_format_RGB.h"
#include "video_interface.h"
#include "wait_key.h"
#include "scale_image_data.h"
#include "draw_bitmap_multiwindow.h"


#define FIFO_LENGTH  1024
#define PWM_RANGE 100
#define SCALE_REDUCTION_PER_AXIS  2   /* the image size reduction ratio (note that 640*480*3*8*FPS = your bandwidth usage, at 24FPS, that is 177MPBS) */
#define GET_FRAMES                10  /* the number of frame times to average when determining the FPS */
#define IMAGE_SIZE                sizeof(struct image_t)/(SCALE_REDUCTION_PER_AXIS*SCALE_REDUCTION_PER_AXIS)

#define EGG_THRESHOLD 210
#define COUNT_THRESHOLD 500
#define IMG_WIDTH 320
#define IMG_HEIGHT 240
#define MAX_EGGS 10
#define STOP_THRESH 2500
#define MAX_DECISION_SIZE 4
#define CENTER_L 90
#define CENTER_R 150

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

void sigint_handler(int signum);

struct thread_command
{
    uint8_t command;
    int argument;
};

FIFO_TYPE(struct thread_command, FIFO_LENGTH, fifo_t);
struct egg_detector_thread_param{

  const char                  *name;
  struct fifo_t               *egg_fifo;
  struct fifo_t               *dir_fifo;
  unsigned char               * RGB_IMG_raw;
  struct pixel_format_RGB     * RGB_IMG_data;
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
  int                             pwm_val;
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
  char                           state; 
  char                           forward;//true forward, false backward
};

struct motor_control_thread_param
{
  const char                    * name;
  struct fifo_t                 * key_fifo;
  struct fifo_t                 * speed_fifo;
  struct fifo_t                 * dir_fifo;
  struct fifo_t                 * IR_sensor_fifo;
  struct fifo_t                 * img_cmd_fifo;
  bool                          * quit_flag;
  bool                          mode; //mode true: mode1,false: mode2                     
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
  struct image_t              *image;
  unsigned char               *img_raw;
  struct pixel_format_RGB     *img_data;
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