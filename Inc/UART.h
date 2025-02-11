#ifndef UART_H
#define UART_H

#include <unistd.h> 
#include <sys/types.h> 
#include <errno.h>     
#include <stdio.h>     
#include <sys/wait.h>  
#include <stdlib.h>  
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include "syslog_util.h"
#include "defines.h"
#include "config.h"
#include <time.h>
#include <stdbool.h> 

#define FPM_DEVICE   "/dev/ttyS0"
#define FPM_BaudRate      B57600
// UART_TX GPIO 14 pin 8
// UART_RX GPIO 15 pin 10

int UART_Init(const char* device, speed_t UART_BaudRate);
void UART_write(int uart_fd,const char* data, int size);
Status_t UART_read(int uart_fd,char* buffer, int size);
void UART_close(int uart_fd);

#endif 