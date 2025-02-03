#ifndef KEYPAD_H
#define KEYPAD_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <stdint.h>
#include "defines.h"
#include <time.h>
#include "GPIO.h"
#include <errno.h>

#define NUM_ROWS 4
#define NUM_COLS 3
#define TOTAL_PINS (NUM_ROWS + NUM_COLS)

#define HIGH 1
#define LOW 0


Status_t keypad_init();
int enter_ID_keypad();
char read_keypad_value();
int check_timeout(struct timespec start_time, int timeout_seconds);
void beginDisplay();

#endif