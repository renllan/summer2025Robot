#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>
#include <stdint.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include "wait_period.h"

#define PACKET_HEADER1 0xAA
#define PACKET_HEADER2 0x55

// Function codes
#define FUNC_SET_ANGLE 0x01
#define FUNC_SET_XYZ 0x03
#define FUNC_SET_PWMSERVO 0x05
#define FUNC_SET_SUCTIONNOZZLE 0x07
#define FUNC_READ_ANGLE 0x11
#define FUNC_READ_XYZ 0x13
#define FUNC_SET_CLAW 0x15
#define FUNC_READ_CLAW 0x17

// UART parameters
#define UART_DEVICE "/dev/ttyUSB0"
#define BAUD_RATE B115200

// CRC8 checksum calculation function
uint8_t checksum_crc8(uint8_t *data, size_t length);

// Function to open UART
int open_uart(const char *device, int baudrate);

// Write data to UART
void serial_send(int uart_fd, uint8_t *data, size_t length);

// Map function to scale values
int map_func(int value, int from_min, int from_max, int to_min, int to_max);

// Function to send angle data (set_angles in Python)
void set_angles(int uart_fd, int *angles, int time);

// Function to set XYZ position (set_xyz in Python)
void set_xyz(int uart_fd, int *pos, int time);

// Function to set PWM servo (set_pwmservo in Python)
void set_pwmservo(int uart_fd, int angle, int time);

// Function to read angles (read_angles in Python)
void read_angles(int uart_fd);

// Function to read XYZ position
void read_xyz(int uart_fd);

// Function to set claw position
void set_claw(int uart_fd, int pos, int time);

// Function to read claw position
void read_claw(int uart_fd);

uint8_t* rec_handle(uint8_t *datas_in, uint8_t func, uint8_t length);

void dly10us(int mult);