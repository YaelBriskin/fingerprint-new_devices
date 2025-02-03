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
    // Открываем GPIO-чип
    if (!gpio_manager.chip) 
    {
        gpio_manager.chip = gpiod_chip_open_by_name("gpiochip0");
        if (!gpio_manager.chip) 
        {
            LOG_MESSAGE(LOG_ERR, __func__, "stderr", "Failed to open GPIO chip", strerror(errno));
            return FAILED;
        }
    }
    // Получаем линию GPIO
    struct gpiod_line *line = gpiod_chip_get_line(gpio_manager.chip, pinNumber);
    if (!line) 
    {
        char logMsg[256];
        snprintf(logMsg, sizeof(logMsg), "Failed to get GPIO line for pin %d. Error: %s", pinNumber, strerror(errno));
        LOG_MESSAGE(LOG_ERR, __func__, "stderr", logMsg, NULL); 
        return FAILED;
    }
    // Устанавливаем направление
    if (strcmp(direction, "out") == 0) 
    {
        if (gpiod_line_request_output(line, "gpio_controller", 0) != 0) 
        {
            char logMsg[256];
            snprintf(logMsg, sizeof(logMsg), "Failed to request GPIO line  %d as output. Error: %s", pinNumber, strerror(errno));
            LOG_MESSAGE(LOG_ERR, __func__, "stderr", logMsg, NULL); 
            return FAILED;
        }
    } 
    else if (strcmp(direction, "in") == 0) 
    {
        if (gpiod_line_request_input(line, "gpio_controller") != 0) 
        {
            char logMsg[256];
            snprintf(logMsg, sizeof(logMsg), "Failed to request GPIO line  %d as input. Error: %s", pinNumber, strerror(errno));
            LOG_MESSAGE(LOG_ERR, __func__, "stderr", logMsg, NULL); 
            return FAILED;
        }
    } 
    else 
    {
        LOG_MESSAGE(LOG_ERR, __func__, "stderr", "Invalid GPIO direction: Expected 'in' or 'out'", NULL);
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
        char logMsg[256];
        snprintf(logMsg, sizeof(logMsg), "GPIO line not initialized for pin %d. Call GPIO_init first.", pinNumber);
        LOG_MESSAGE(LOG_ERR, __func__, "stderr", logMsg, NULL);        
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
    // Проверка, что линия была инициализирована для данного пина
    struct gpiod_line *line = gpio_manager.lines[pinNumber];
    if (!line) 
    {   
        char logMsg[256];   
        snprintf(logMsg, sizeof(logMsg), "GPIO line not initialized for pin %d. Call GPIO_init first.", pinNumber);
        LOG_MESSAGE(LOG_ERR, __func__, "stderr", logMsg, NULL);
        return FAILED;
    }

    // Попытка установить значение на линии
    int result = gpiod_line_set_value(line, value);
    if (result != 0) 
    {
        char logMsg[256];        
        snprintf(logMsg, sizeof(logMsg), "Failed to set GPIO value for pin %d. Error: %s", pinNumber, strerror(errno));
        LOG_MESSAGE(LOG_ERR, __func__, "stderr", logMsg, NULL);
        return FAILED;
    }

    // Если все прошло успешно
    return SUCCESS;
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
