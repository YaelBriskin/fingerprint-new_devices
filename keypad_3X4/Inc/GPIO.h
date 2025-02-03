#ifndef GPIO_H
#define GPIO_H

#include <gpiod.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "defines.h"
#include <errno.h>

#define GPIO_PIN_ROW1 5   //pin 29
#define GPIO_PIN_ROW2 6   //pin 31
#define GPIO_PIN_ROW3 26  //pin 37
#define GPIO_PIN_ROW4 16  //pin 36

#define GPIO_PIN_COL1 17  //pin 11
#define GPIO_PIN_COL2 27  //pin 13
#define GPIO_PIN_COL3 22  //pin 15

#define GPIO_COUNT 9

Status_t GPIO_init(int pinNumber, const char* direction);
int GPIO_read(int pinNumber);
Status_t GPIO_write(int pinNumber, int value);
void GPIO_close();

#endif 