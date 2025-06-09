/*
 * video_interface_test.c
 *
 *  Created on: Jun 4, 2022
 *      Author: steveb
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <linux/videodev2.h>
#include <time.h>
#include <cairo/cairo.h>
#include <gtk/gtk.h>
#include "pixel_format_RGB.h"
#include "video_interface.h"
#include "wait_key.h"
#include "scale_image_data.h"
#include "draw_bitmap_multiwindow.h"

#define REPORT_TIMES              0   /* != 0 to print some timing statistics */
#define GET_FRAMES                10  /* the number of frame times to average when determining the FPS */
#define SCALE_REDUCTION_PER_AXIS  4   /* the image size reduction ratio (note that 640*480*3*8*FPS = your bandwidth usage, at 24FPS, that is 177MPBS) */
#define IMAGE_SIZE                sizeof(struct image_t)/(SCALE_REDUCTION_PER_AXIS*SCALE_REDUCTION_PER_AXIS)

#if REPORT_TIMES
static unsigned long calc_time_diff_in_us( struct timespec * start, struct timespec * end )
{
  return (((end->tv_sec - start->tv_sec) * 1000000) +
          ((end->tv_nsec - start->tv_nsec) / 1000));
}
#endif

int main( int argc, char * argv[] )
{
  struct video_interface_handle_t *         handle_video;
  struct draw_bitmap_multiwindow_handle_t * handle_GUI_RGB = NULL;
  struct draw_bitmap_multiwindow_handle_t * handle_GUI_grey = NULL;
  static struct image_t                     image;
  unsigned char *                           scaled_RGB_raw;
  struct pixel_format_RGB *                 scaled_RGB_data;
  unsigned char *                           scaled_grey_raw;
  struct pixel_format_RGB *                 scaled_grey_data;
  unsigned int                              scaled_height;
  unsigned int                              scaled_width;
  int                                       pressed_key = '\0';
#if REPORT_TIMES
  struct timespec                           start_time              = {0};
  struct timespec                           open_time               = {0};
  struct timespec                           print_time              = {0};
  struct timespec                           set_mode_time           = {0};
  struct timespec                           close_time              = {0};
  unsigned int                              frame_index             = 0;
  struct timespec                           frame_times[GET_FRAMES] = {0};

  clock_gettime( CLOCK_REALTIME, &start_time );
#endif

  handle_video = video_interface_open( "/dev/video0" );
#if REPORT_TIMES
  clock_gettime( CLOCK_REALTIME, &open_time );
#endif

  video_interface_print_modes( handle_video );
#if REPORT_TIMES
  clock_gettime( CLOCK_REALTIME, &print_time );
#endif

#if 0
  // just an example of how to manually select a video format
  if (video_interface_set_mode_manual( handle_video, 0 ))
#else
  if (video_interface_set_mode_auto( handle_video ))
#endif
  {
#if REPORT_TIMES
    clock_gettime( CLOCK_REALTIME, &set_mode_time );
#endif

    printf( "configured resolution: %zux%zu\n", handle_video->configured_width, handle_video->configured_height );
    // set up the buffer for scaled data
    scaled_width      = handle_video->configured_width/SCALE_REDUCTION_PER_AXIS;
    scaled_height     = handle_video->configured_height/SCALE_REDUCTION_PER_AXIS;
    scaled_RGB_raw    = (unsigned char *)malloc( IMAGE_SIZE );
    scaled_RGB_data   = (struct pixel_format_RGB *)scaled_RGB_raw;
    scaled_grey_raw   = (unsigned char *)malloc( IMAGE_SIZE );
    scaled_grey_data  = (struct pixel_format_RGB *)scaled_grey_raw;

    // start the GUI thread
    draw_bitmap_start( argc, argv );

    printf( "press 'q' to quit\n" );
    printf( "press 'c' to enable color display (with a green line)\n" );
    printf( "press 'g' to enable grey-scale display\n" );
    for (wait_key( 0, &pressed_key ); pressed_key != 'q'; wait_key( 0, &pressed_key ))
    {
      // capture an image
      if (video_interface_get_image( handle_video, &image ))
      {
        // scale the image to a more agreeable size
        scale_image_data(
            (struct pixel_format_RGB *)&image,
            handle_video->configured_height,
            handle_video->configured_width,
            scaled_RGB_data,
            SCALE_REDUCTION_PER_AXIS,
            SCALE_REDUCTION_PER_AXIS );
        memcpy( scaled_grey_raw, scaled_RGB_raw, IMAGE_SIZE );

        switch (pressed_key)
        {
          case 'c': // toggle color display
            if (handle_GUI_RGB == NULL)
            {
              // create the window to show the bitmap
              handle_GUI_RGB  = draw_bitmap_create_window( scaled_width, scaled_height );
            }
            else
            {
              // close the window
              draw_bitmap_close_window( handle_GUI_RGB );
              handle_GUI_RGB = NULL;
            }
            pressed_key = '\0';
            break;

          case 'g': // toggle grey-scale display
            if (handle_GUI_grey == NULL)
            {
              // create the window to show the bitmap
              handle_GUI_grey  = draw_bitmap_create_window( scaled_width, scaled_height );
            }
            else
            {
              // close the window
              draw_bitmap_close_window( handle_GUI_grey );
              handle_GUI_grey = NULL;
            }
            pressed_key = '\0';
            break;

          default:
            break;  // some other key
        }

        /*
         * process and display image data
         */
        // process and display color data, if enabled
        if (handle_GUI_RGB != NULL)
        {
          // set the top-row color
          for (size_t i = 0; i < scaled_width; i++)
          {
            // row-oriented data
            scaled_RGB_data[i].R = 0;
            scaled_RGB_data[i].G = 255;
            scaled_RGB_data[i].B = 0;
          }

          // display what the camera sees
          draw_bitmap_display( handle_GUI_RGB,  scaled_RGB_data );
        }
        else
        {
          ; // the window is closed
        }

        // process and display grey-scale data, if enabled
        if (handle_GUI_grey != NULL)
        {
          // change to greyscale
          for (size_t i = 0; i < scaled_width; i++)
          {
            for (size_t j = 0; j < scaled_height; j++)
            {
              unsigned short p =
                  (scaled_grey_data[i + j * scaled_width].R +
                   scaled_grey_data[i + j * scaled_width].G +
                   scaled_grey_data[i + j * scaled_width].B)/3;
  ;

              // row-oriented data
              scaled_grey_data[i + j * scaled_width].R = p;
              scaled_grey_data[i + j * scaled_width].G = p;
              scaled_grey_data[i + j * scaled_width].B = p;
            }
          }

          // display what the camera sees
          draw_bitmap_display( handle_GUI_grey, scaled_grey_data );
        }
        else
        {
          ; // the window is closed

        }

#if REPORT_TIMES
        clock_gettime( CLOCK_REALTIME, &(frame_times[frame_index]) );
        frame_index = (frame_index + 1) % GET_FRAMES;
#endif
      }
      else
      {
        printf( "did not get an image\n" );
      }
    }
  }
  else
  {
    printf( "failed to configure\n" );
  }

  // clean up
  if (handle_GUI_RGB != NULL)
  {
    draw_bitmap_close_window( handle_GUI_RGB );
  }
  else
  {
    ; // window is not open
  }
  if (handle_GUI_grey != NULL)
  {
    draw_bitmap_close_window( handle_GUI_grey );
  }
  else
  {
    ; // window is not open
  }
  draw_bitmap_stop();
  video_interface_close( handle_video );
  free( scaled_RGB_raw );
  free( scaled_grey_raw );
#if REPORT_TIMES
  clock_gettime( CLOCK_REALTIME, &close_time );

  printf( "\n" );
  printf(   "run duration:\n" );
  printf(   "open:      %lu\n", calc_time_diff_in_us( &start_time, &open_time ) );
  printf(   "print:     %lu\n", calc_time_diff_in_us( &start_time, &print_time ) );
  printf(   "set mode:  %lu\n", calc_time_diff_in_us( &start_time, &set_mode_time ) );
  printf(   "close:     %lu\n", calc_time_diff_in_us( &start_time, &close_time ) );
  printf( "approximate frames/sec = %.2f\n", 1.0 / ((calc_time_diff_in_us( &start_time, &(frame_times[GET_FRAMES-1]) ) - calc_time_diff_in_us( &start_time, &(frame_times[GET_FRAMES-2]) )) / 1000000.0) );
#endif

  return 0;
}
