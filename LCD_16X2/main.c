#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <time.h>

#include "./Inc/defines.h"
#include "./Inc/lcd16x2_i2c.h"

int main() 
{
    if (!lcd16x2_i2c_init()) 
    {
        printf("Display initialization error!\n");
        return EXIT_FAILURE;
    }

    lcd16x2_i2c_clear();

    lcd16x2_i2c_print(0, 0, "Test Line 1");
    lcd16x2_i2c_print(1, 0, "Test Line 2");
    printf("Test message sent to display.\n");

    I2C_close();

    return EXIT_SUCCESS;
}