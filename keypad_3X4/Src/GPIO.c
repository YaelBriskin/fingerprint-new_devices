#include "../Inc/GPIO.h"

typedef struct {
    struct gpiod_chip *chip;
    struct gpiod_line *lines[GPIO_COUNT];
} GPIO_Manager;

GPIO_Manager gpio_manager = {0}; 
/**
 * @brief Initializes a GPIO pin with the specified direction.
 * @param pinNumber The number of the GPIO pin to initialize.
 * @param direction The direction of the GPIO pin ("in" or "out").
 * @return 1 on success, 0 on failure.
 */
Status_t GPIO_init(int pinNumber, const char* direction)
{
    if (!gpio_manager.chip) 
    {
        gpio_manager.chip = gpiod_chip_open_by_name("gpiochip0");
        if (!gpio_manager.chip) 
        {
            fprintf(stderr, "Failed to open GPIO chip: %s\n", strerror(errno));
            return FAILED;
        }
    }

    struct gpiod_line *line = gpiod_chip_get_line(gpio_manager.chip, pinNumber);
    if (!line) 
    {
        fprintf(stderr, "Failed to get GPIO line: %s\n", strerror(errno));
        return FAILED;
    }

    if (strcmp(direction, "out") == 0) 
    {
        if (gpiod_line_request_output(line, "gpio_controller", 0) != 0) 
        {
            fprintf(stderr, "Failed to request GPIO line as output: %s\n", strerror(errno));
            return FAILED;
        }
    } 
    else if (strcmp(direction, "in") == 0) 
    {
        if (gpiod_line_request_input(line, "gpio_controller") != 0) 
        {
            fprintf(stderr, "Failed to request GPIO line as input: %s\n", strerror(errno));
            return FAILED;
        }
    } 
    else 
    {
        fprintf(stderr, "Invalid GPIO direction: Expected 'in' or 'out'\n");
        return FAILED;
    }

    gpio_manager.lines[pinNumber] = line; // Сохраняем линию
    return SUCCESS;
}


/**
 * @brief Reads the value of a GPIO pin.
 * 
 * @param pinNumber The number of the GPIO pin.
 * @return The value of the GPIO pin (1 or 0). Returns -1 on failure.
 */
int GPIO_read(int pinNumber) 
{
    struct gpiod_line *line = gpio_manager.lines[pinNumber];
    if (!line) 
    {
        fprintf(stderr, "GPIO line not initialized. Call GPIO_init first.\n");
        return -1;
    }
    return gpiod_line_get_value(line);
}
/**
 * @brief Writes a value to a GPIO pin.
 * @param pinNumber The GPIO pin number.
 * @param value The value to write to the GPIO pin (1 or 0).
 * @return SUCCESS on success, FAILED on failure.
 */
Status_t GPIO_write(int pinNumber, int value) 
{
    struct gpiod_line *line = gpio_manager.lines[pinNumber];
    if (!line) 
    {
        fprintf(stderr, "GPIO line not initialized. Call GPIO_init first.\n");
        return FAILED;
    }
    return gpiod_line_set_value(line, value) == 0 ? SUCCESS : FAILED;
}
/**
 * @brief Releases all resources associated with the GPIO manager.
 * 
 */
void GPIO_close() 
{
    if (gpio_manager.chip) 
    {
        for (int i = 0; i < GPIO_COUNT; i++) 
        {
            if (gpio_manager.lines[i]) 
            {
                gpiod_line_release(gpio_manager.lines[i]);
                gpio_manager.lines[i] = NULL;

            }
        }
        gpiod_chip_close(gpio_manager.chip);
        gpio_manager.chip = NULL;
    }
}
