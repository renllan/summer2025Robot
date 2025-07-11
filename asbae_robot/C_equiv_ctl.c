#include "C_equiv_ctl.h"
#define READ_CT 10 // Number of read attempts before giving up

/*
Packet structure: 0xAA 0x55 | func | data_len | data | check

- Frame header: if 0xAA and 0x55 are received sequentially, it indicates that there is data to be received,
  consisting of a fixed 2 bytes.
- Function Code: Used to indicate the purpose of an information frame, consists of 1 byte. Includes:
    - 0x01: Set Angle
    - 0x03: Set XYZ
    - 0x05: Set PWM Servo
    - 0x07: Set Suction Nozzle (not implemented in this code)
    - 0x11: Read Angle
    - 0x13: Read XYZ
    - 0x15: Set Arm Position
    - 0x17: Read Arm Position
- Data Length: Indicates the number of data bits carried by the data frame.
  Each data length for each function code is the following:
    - Set Angle: 8 bytes (6 bytes for angles + 2 bytes for time, each angle is 2 bytes)
    - Set XYZ: 8 bytes (6 bytes for position + 2 bytes for time, each coordinate is 2 bytes)
    - Set PWM Servo: 4 bytes (2 bytes for pulse + 2 bytes for time, pulse is 2 bytes)
    - Read Angle: 6 bytes (for angles, each angle is 2 bytes)
    - Read XYZ: 6 bytes (for position, each coordinate is 2 bytes)
    - Set Arm Position: 4 bytes (2 bytes for position + 2 bytes for time)
    - Read Arm Position: 2 bytes (2 bytes for position)
- Data: The actual data being transmitted, with a length specified by the Data Length field.
- Check Bit: Verifies the correctness of the data frame.
  If correct, the corresponding function is called; otherwise, the data frame is skipped.
  The calculation method for the check bit is:
    - calculate the sum of the function code, data length, and data
    - take the complement
    - take the low byte, which serves as the checksum.
*/

// CRC8 checksum calculation function
uint8_t checksum_crc8(uint8_t *data, size_t length) {
    uint8_t check = 0;
    for (size_t i = 0; i < length; i++) {
        check = (check + data[i]);
    }
    return ~check & 0xFF;
}

// Function to open UART
int open_uart(const char *device, int baudrate) {
    int uart_fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    if (uart_fd == -1) {
        perror("Unable to open UART\n");
        exit(1);
    }
    printf("file status = 0x%x\n\n", fcntl(uart_fd, F_GETFL));
    struct termios options;
    tcgetattr(uart_fd, &options);
    cfsetispeed(&options, baudrate);
    cfsetospeed(&options, baudrate);

    options.c_cflag |= (CLOCAL | CREAD); // Enable receiver, disable modem control
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8; // 8 data bits
    options.c_cflag &= ~PARENB; // No parity
    options.c_cflag &= ~CSTOPB; // 1 stop bit
    tcsetattr(uart_fd, TCSANOW, &options);
    return uart_fd;
}

// Map function to scale values
// Parameters:
// - value: the value to be mapped
// - from_min: the minimum of the original range
// - from_max: the maximum of the original range
// - to_min: the minimum of the target range
// - to_max: the maximum of the target range
int map_func(int value, int from_min, int from_max, int to_min, int to_max) {
    return (value - from_min) * (to_max - to_min) / (from_max - from_min) + to_min;
}

// Function to send angle data (set_angles in Python)
void set_angles(int uart_fd, int *angles, int time) {
    uint8_t data[100];
    int mapped_angles[3];
    for (int i = 0; i < 3; i++) {
        mapped_angles[i] = map_func(angles[i], 0, 180, 0, 1000);
    }

    // Construct the data packet
    size_t idx = 0;
    data[idx++] = PACKET_HEADER1;
    data[idx++] = PACKET_HEADER2;
    data[idx++] = FUNC_SET_ANGLE;
    data[idx++] = 0x08;  // Data length

    for (int i = 0; i < 3; i++) {
        data[idx++] = mapped_angles[i] & 0xFF;        // Low byte
        data[idx++] = (mapped_angles[i] >> 8) & 0xFF; // High byte
    }

    data[idx++] = time & 0xFF;           // Time (low byte)
    data[idx++] = (time >> 8) & 0xFF;    // Time (high byte)

    uint8_t crc = checksum_crc8(data + 2, idx - 2);
    data[idx++] = crc;

    //serial_send(uart_fd, data, idx);
    printf("Sending command: ");
    for (size_t i = 0; i < idx; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
    write(uart_fd, data, idx);

    // For debugging purposes, print the command sent
    // usleep(100000); // Wait for 100ms to ensure the command is sent
    // uint8_t response[8];
    // int read_ct = READ_CT;
    // read_again_sang: ssize_t bytes_read = read(uart_fd, response, sizeof(response));
    // printf("bytes_read = %zd\n", bytes_read);
    // if (bytes_read <= 0 && read_ct > 0) {
    //     printf("No data received, retrying...\n");
    //     usleep(100000); // Wait for 100ms before retrying
    //     read_ct--;
    //     printf("Retrying read, remaining attempts: %d\n", read_ct);
    //     goto read_again_sang; // Retry reading
    // }
    // else if(bytes_read == 0) {
    //     printf("No data received, exiting...\n\n");
    // }
    // else if(bytes_read < 0) printf("Error reading from UART: %s\n\n", strerror(errno));
    // if (bytes_read > 0) {
    //     // Process the received data
    //     printf("Received response: ");
    //     for (size_t i = 0; i < bytes_read; i++) {
    //         printf("%02X ", response[i]);
    //     }
    //     printf("\n\n");
    // }
}

// Function to send XYZ data (set_xyz in Python)
void set_xyz(int uart_fd, int *pos, int time) {
    uint8_t data[100];
    size_t idx = 0;
    data[idx++] = PACKET_HEADER1;
    data[idx++] = PACKET_HEADER2;
    data[idx++] = FUNC_SET_XYZ;
    data[idx++] = 0x08;  // Data length

    // Pack position data
    for (int i = 0; i < 3; i++) {
        data[idx++] = pos[i] & 0xFF;        // Low byte
        data[idx++] = (pos[i] >> 8) & 0xFF; // High byte
    }

    data[idx++] = time & 0xFF;           // Time (low byte)
    data[idx++] = (time >> 8) & 0xFF;    // Time (high byte)

    uint8_t crc = checksum_crc8(data + 2, idx - 2);
    data[idx++] = crc;

    //serial_send(uart_fd, data, idx);
    printf("Sending command: ");
    for (size_t i = 0; i < idx; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
    write(uart_fd, data, idx);

    // For debugging purposes, print the command sent
    // usleep(100000); // Wait for 100ms to ensure the command is sent
    // uint8_t response[8];
    // int read_ct = READ_CT;
    // read_again_sxyz: ssize_t bytes_read = read(uart_fd, response, sizeof(response));
    // printf("bytes_read = %zd\n", bytes_read);
    // if (bytes_read <= 0 && read_ct > 0) {
    //     printf("No data received, retrying...\n");
    //     usleep(100000); // Wait for 100ms before retrying
    //     read_ct--;
    //     printf("Retrying read, remaining attempts: %d\n", read_ct);
    //     goto read_again_sxyz; // Retry reading
    // }
    // else if(bytes_read == 0) {
    //     printf("No data received, exiting...\n\n");
    // }
    // else if(bytes_read < 0) printf("Error reading from UART: %s\n\n", strerror(errno));
    // if (bytes_read > 0) {
    //     // Process the received data
    //     printf("Received response: ");
    //     for (size_t i = 0; i < bytes_read; i++) {
    //         printf("%02X ", response[i]);
    //     }
    //     printf("\n\n");
    // }
}

// Function to set PWM servo (set_pwmservo in Python)
void set_pwmservo(int uart_fd, int angle, int time) {
    // Ensure angle is between 0 and 180
    if (angle > 180) {
        angle = 180;
    }

    // Map the angle to a pulse value (500 to 2500 microseconds)
    int pul = map_func(angle, 0, 180, 500, 2500);

    // Construct the data packet
    uint8_t data[9];
    size_t idx = 0;

    data[idx++] = PACKET_HEADER1;
    data[idx++] = PACKET_HEADER2;
    data[idx++] = FUNC_SET_PWMSERVO;
    data[idx++] = 0x04;  // Data length: 4 bytes (pulse + time)

    data[idx++] = pul & 0xFF;           // Low byte of pulse
    data[idx++] = (pul >> 8) & 0xFF;    // High byte of pulse

    data[idx++] = time & 0xFF;          // Low byte of time
    data[idx++] = (time >> 8) & 0xFF;   // High byte of time

    // Calculate the checksum
    uint8_t crc = checksum_crc8(data + 2, idx - 2);  // Exclude headers from checksum calculation
    data[idx++] = crc;  // Append checksum

    // Send the data packet
    printf("Sending command: ");
    for (size_t i = 0; i < idx; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
    //serial_send(uart_fd, data, idx);
    write(uart_fd, data, idx);
}

// Function to send claw data
void set_claw(int uart_fd, int pos, int time) {
    struct timespec timer_state;
    wait_period_initialize(&timer_state);
    uint8_t data[100];
    size_t idx = 0;
    data[idx++] = PACKET_HEADER1;
    data[idx++] = PACKET_HEADER2;
    data[idx++] = FUNC_SET_CLAW;
    data[idx++] = 0x04;  // Data length: 4 bytes (position + time)

    // Pack position data
    data[idx++] = pos & 0xFF;        // Low byte of position
    data[idx++] = (pos >> 8) & 0xFF; // High byte of position

    data[idx++] = time & 0xFF;           // Time (low byte)
    data[idx++] = (time >> 8) & 0xFF;    // Time (high byte)

    uint8_t crc = checksum_crc8(data + 2, idx - 2);
    data[idx++] = crc;

    //serial_send(uart_fd, data, idx);
    printf("Sending command: ");
    for (size_t i = 0; i < idx; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
    write(uart_fd, data, idx);

    // For debugging purposes, print the command sent
    // wait_period(&timer_state, 100u); // Wait for 100ms to ensure the command is sent
    // uint8_t response[8];
    // int read_ct = READ_CT;
    // read_again_claw: ssize_t bytes_read = read(uart_fd, response, sizeof(response));
    // printf("bytes_read = %zd\n", bytes_read);
    // if (bytes_read <= 0 && read_ct > 0) {
    //     printf("No data received, retrying...\n");
    //     wait_period(&timer_state, 100u); // Wait for 100ms before retrying
    //     read_ct--;
    //     printf("Retrying read, remaining attempts: %d\n", read_ct);
    //     goto read_again_claw; // Retry reading
    // }
    // else if(bytes_read == 0) {
    //     printf("No data received, exiting...\n\n");
    // }
    // else if(bytes_read < 0) printf("Error reading from UART: %s\n\n", strerror(errno));
    // if (bytes_read > 0) {
    //     // Process the received data
    //     printf("Received response: ");
    //     for (size_t i = 0; i < bytes_read; i++) {
    //         printf("%02X ", response[i]);
    //     }
    //     printf("\n\n");
    // }
}

// // Function to read arm position (read_arm_pos in Python)
// void read_arm_pos(int uart_fd) {
//     // Send command to read arm position (0xAA, 0x55, FUNC_READ_ARM_POS, 0x00, 0xEE)
//     uint8_t command[5] = {PACKET_HEADER1, PACKET_HEADER2, FUNC_READ_CLAW, 0x00, 0xEE};
//     printf("Sending command: ");
//     for (int i = 0; i < 5; i++) {
//         printf("%02X ", command[i]);
//     }
//     printf("\n");

//     // Wait for the response (assuming the MaxArm sends 7 bytes)
//     uint8_t response[7];
//     write(uart_fd, command, 5);
//     usleep(100000);
//     read_again_claw: ssize_t bytes_read = read(uart_fd, response, sizeof(response));
//     printf("bytes_read = %zd\n", bytes_read);
//     int read_ct = 3;
//     if (bytes_read == 0 && read_ct > 0) {
//         printf("No data received, retrying...\n");
//         usleep(100000); // Wait for 100ms before retrying
//         read_ct--;
//         goto read_again_claw; // Retry reading
//     }
//     else if(bytes_read == 0) {
//         printf("No data received, exiting...\n");
//         goto no_response; // Exit if no data is received
//     }
//     else if(bytes_read < 0) printf("Error reading from UART: %s\n", strerror(errno));

//     if (bytes_read == 11) {
//         // Print out the raw response (for debugging purposes)
//         printf("Received response: ");
//         for (size_t i = 0; i < 11; i++) {
//             printf("%02X ", response[i]);
//         }
//         printf("\n");

//         uint8_t *data = rec_handle(response, FUNC_READ_ARM_POS, 11);
//         if (data) {
//             short pos = (short)(data[0] | (data[1] << 8));
//             printf("Received arm position: %d\n\n", pos);
//         }
//         else {
//             no_response: printf("Invalid response or checksum error.\n\n");
//         }
//     }
// }

// Function to read angles (read_angles in Python)
void read_angles(int uart_fd) {
    // Send command to read angles (0xAA, 0x55, FUNC_READ_ANGLE, 0x00, 0xEE)
    struct timespec timer_state;
    wait_period_initialize(&timer_state);
    uint8_t command[5] = {PACKET_HEADER1, PACKET_HEADER2, FUNC_READ_ANGLE, 0x00, 0xEE};
    ssize_t bytes_read;
    printf("Sending command: ");
    for (int i = 0; i < 5; i++) {
        printf("%02X ", command[i]);
    }
    printf("\n");

    // Wait for the response (assuming the MaxArm sends 11 bytes)
    uint8_t response[11];
    write(uart_fd, command, 5);
    wait_period(&timer_state, 100u);

    read_again_ang: size_t bytes_read = read(uart_fd, response, sizeof(response));
    printf("bytes_read = %zd\n", bytes_read);
    int read_ct = READ_CT;
    if (bytes_read <= 0 && read_ct > 0) {
        printf("No data received, retrying...\n");
        wait_period(&timer_state, 100u); // Wait for 100ms before retrying
        read_ct--;
        goto read_again_ang; // Retry reading
    }
    else if(bytes_read == 0) {
        printf("No data received, exiting...\n");
        goto no_response_ang; // Exit if no data is received
    }
    else if(bytes_read < 0) printf("Error reading from UART: %s\n", strerror(errno));

    if (bytes_read == 11) {
        // Print out the raw response (for debugging purposes)
        printf("Received response: ");
        for (size_t i = 0; i < 11; i++) {
            printf("%02X ", response[i]);
        }
        printf("\n");

        uint8_t *data = rec_handle(response, FUNC_READ_ANGLE, 11);
        if (data) {
            short angles[3];
            angles[0] = (short)(data[0] | (data[1] << 8)) * 9 / 50; // Convert to degrees
            angles[1] = (short)(data[2] | (data[3] << 8)) * 9 / 50;
            angles[2] = (short)(data[4] | (data[5] << 8)) * 9 / 50;
            printf("Received angles: %d, %d, %d\n\n", angles[0], angles[1], angles[2]);
        }
        else {
            no_response_ang: printf("Invalid response or checksum error.\n\n");
        }
    }
}

// Function to read XYZ position
void read_xyz(int uart_fd) {
    // Send command to read XYZ position (0xAA, 0x55, FUNC_READ_XYZ, 0x00, 0xEC)
    struct timespec timer_state;
    wait_period_initialize(&timer_state);
    uint8_t command[5] = {PACKET_HEADER1, PACKET_HEADER2, FUNC_READ_XYZ, 0x00, 0xEC};
    ssize_t bytes_read;
    printf("Sending command: ");
    for (int i = 0; i < 5; i++) {
        printf("%02X ", command[i]);
    }
    printf("\n");

    // Wait for the response (assuming the MaxArm sends 11 bytes)
    uint8_t response[11];
    write(uart_fd, command, 5);
    wait_period(&timer_state, 100u);

    read_again_xyz: bytes_read = read(uart_fd, response, sizeof(response));
    printf("bytes_read = %zd\n", bytes_read);
    int read_ct = READ_CT;
    if (bytes_read <= 0 && read_ct > 0) {
        printf("No data received, retrying...\n");
        wait_period(&timer_state, 100u); // Wait for 100ms before retrying
        read_ct--;
        goto read_again_xyz; // Retry reading
    }
    else if(bytes_read == 0) {
        printf("No data received, exiting...\n");
        goto no_response_xyz; // Exit if no data is received
    }
    else if(bytes_read < 0) printf("Error reading from UART: %s\n", strerror(errno));

    if (bytes_read == 11) {
        // Print out the raw response (for debugging purposes)
        printf("Received response: ");
        for (size_t i = 0; i < 11; i++) {
            printf("%02X ", response[i]);
        }
        printf("\n");

        uint8_t *data = rec_handle(response, FUNC_READ_XYZ, 11);
        if (data) {
            short x = (short)(data[0] | (data[1] << 8));
            short y = (short)(data[2] | (data[3] << 8));
            short z = (short)(data[4] | (data[5] << 8));
            printf("Received XYZ: x=%d, y=%d, z=%d\n\n", x, y, z);
        }
        else {
            printf("Invalid response or checksum error.\n\n");
        }
    }
    else {
        no_response_xyz: printf("Failed to read the expected response.\n\n");
    }
}

// Data parsing function
uint8_t* rec_handle(uint8_t *datas_in, uint8_t func, uint8_t length) {
    static uint8_t rt_data[6];  // To store the result
    uint8_t step = 1;
    uint8_t len = 0;
    //uint8_t count = 0;
    
    uint8_t byte_data[256];  // Buffer to store data
    uint8_t byte_data_index = 0;
    
    for (uint8_t i = 0; i < length; i++) {
        uint8_t data = datas_in[i];
        
        switch (step) {
            case 1: // Check for the first header byte
                if (data == 0xAA) {
                    byte_data_index = 0; // Reset the buffer index
                    step = 2; // Proceed to check for the second header byte
                }
                break;

            case 2: // Check for the second header byte
                if (data == 0x55) {
                    step = 3; // Proceed to check function code
                } else {
                    step = 1; // Reset if the first byte is not 0x55
                }
                break;

            case 3: // Check for the function code
                if (data == func) {
                    byte_data[byte_data_index++] = data;
                    step = 4;
                } else {
                    step = 1;
                }
                break;

            case 4: // Check for the data length
                if (data == 6) {
                    byte_data[byte_data_index++] = data;
                    len = data;
                    step = 5;
                } else {
                    step = 1;
                }
                break;

            case 5: // Collect the data bytes
                byte_data[byte_data_index++] = data;
                len--;
                if (len == 0) {
                    step = 6;
                }
                break;

            case 6: // Check for the checksum byte
                if (checksum_crc8(byte_data, byte_data_index) == data) {
                    memcpy(rt_data, &byte_data[2], 6);  // Copy data to result
                    return rt_data;
                } else {
                    return NULL; // Invalid checksum
                }
                break;

            default:
                step = 1;  // Reset in case of an unexpected state
                break;
        }
    }
    return NULL;  // Return NULL if no valid data is found
}

void dly10us(int mult)
{
  int total = 1866*mult;
  int i = 1;
  while(i <= total)     // 10us*mult busy processing
  {
    ++i;
  }
}