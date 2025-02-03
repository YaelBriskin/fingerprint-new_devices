#include "../Inc/keypad.h"

int rowPins[NUM_ROWS] = {GPIO_PIN_ROW1, GPIO_PIN_ROW2, GPIO_PIN_ROW3, GPIO_PIN_ROW4};
int colPins[NUM_COLS] = {GPIO_PIN_COL1, GPIO_PIN_COL2, GPIO_PIN_COL3};

char keys[NUM_ROWS][NUM_COLS] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}};

// Define the buffer for the entered ID
char code[MAX_LENGTH_ID + 1] = {'_', '_', '_', '\0'};

/**
 * @brief Initializes the keypad by configuring the row and column GPIO pins.
 *
 * This function configures the GPIO pins for rows as input and columns as output.
 * It returns FAILED if any pin initialization fails.
 *
 * @return Status_t SUCCESS if initialization is successful, FAILED otherwise.
 */
Status_t keypad_init()
{
    for (int i = 0; i < NUM_ROWS; i++)
    {
        if (GPIO_init(rowPins[i], "in") == FAILED)
        {
            printf("Failed to initialize row pin %d\n", rowPins[i]);
            return FAILED;
        }
    }
    for (int j = 0; j < NUM_COLS; j++)
    {
        if (GPIO_init(colPins[j], "out") == FAILED)
        {
            printf("Failed to initialize column pin %d\n", colPins[j]);
            return FAILED;
        }
    }
    return SUCCESS;
}

/**
 * @brief Checks if the specified timeout duration has passed since the given start time.
 *
 * This function compares the current time with the start time and returns 1 if the timeout 
 * has passed, and 0 otherwise.
 *
 * @param start_time The starting time of the operation.
 * @param timeout_seconds The timeout duration in seconds.
 * @return int Returns 1 if the timeout has passed, 0 otherwise.
 */
int check_timeout(struct timespec start_time, int timeout_seconds)
{
    struct timespec current_time;
    clock_gettime(CLOCK_MONOTONIC, &current_time);
    long elapsed_time = (current_time.tv_sec - start_time.tv_sec) + (current_time.tv_nsec - start_time.tv_nsec) / 1000000000;
    return (elapsed_time >= timeout_seconds);
}
/**
 * @brief Reads and validates an ID entered through the keypad.
 *
 * This function waits for the user to enter an ID using the keypad, validates it,
 * and returns the ID if valid. If a timeout occurs or an invalid ID is entered, it returns ERROR.
 *
 * @return int The entered ID or ERROR if a timeout occurs or invalid ID is entered.
 */
int enter_ID_keypad()
{
    struct timespec start_time;
    const int max_execution_time = 30; // Maximum time allowed for input in seconds
    struct timespec current_time;
    int digit_count = 0;
    int attempts = 0;

    // Record the start time
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    while (1)
    {
        if (check_timeout(start_time, max_execution_time))
        {
            printf("Timeout: time is up\n");
            return ERROR;
        }
        char character = read_keypad_value();
        // Handle character input
        if (character != '\0') // If the button is pressed
        {
            if (character != '#' && character != '*' && digit_count < MAX_LENGTH_ID)
            {

                code[digit_count] = character;
                digit_count++;
                // Update LCD display
                printf("%s\n", code);
            }
            else if (character == '*')
            {
                if (digit_count > 0)
                {
                    code[--digit_count] = '_';
                    // Update LCD display
                    printf("%s\n", code);
                }
                else
                {
                    printf("cancel the operation?\n * yes   # no");

                    struct timespec start_time;
                    clock_gettime(CLOCK_MONOTONIC, &start_time); // Записываем текущее время
                    while (1)
                    {
                        if (check_timeout(start_time, max_execution_time))
                        {
                            printf("Timeout: time is up\n");
                            return ERROR;
                        }

                        character = read_keypad_value();
                        if (character == '*') // The user selected "Yes"
                        {
                            return CANCEL;
                        }
                        else if (character == '#') // User selected "No"
                        {
                            printf("%s\n", code);
                            break;
                        }

                        usleep(200000);
                    }
                }
            }
            else if (character == '#')
            {
                if (digit_count == MAX_LENGTH_ID)
                {
                    code[digit_count] = '\0';
                    int id = atoi(code);
                    return id;
                }
                else
                {
                    printf("Invalid ID, enter again.\n");
                    attempts++;
                    if (attempts >= MAX_RETRIES)
                    {
                        printf("Max attempts reached.\n");
                        return ERROR;
                    }
                }
                digit_count = 0; // Reset the digit counter
            }
        }
        usleep(100000); 
        fflush(stdout);
    }
}
/**
 * @brief Reads a value from the keypad.
 *
 * This function scans the keypad by setting each column pin to LOW one at a time and 
 * checking if any of the row pins are LOW, indicating a button press.
 *
 * @return char The character corresponding to the pressed button or '\0' if no button is pressed.
 */
char read_keypad_value()
{
    for (int j = 0; j < NUM_COLS; j++)
    {
        if (GPIO_write(colPins[j], HIGH) == FAILED)
        {
            printf("Failed to set column high: %s", strerror(errno));
            return '\0';
        }
    }
    // Check each column
    for (int j = 0; j < NUM_COLS; j++)
    {
        // Set current column to LOW
        if (GPIO_write(colPins[j], LOW) == FAILED)
        {
            printf("Failed to set column low: %s", strerror(errno));
            return '\0';
        }

        
        for (int i = 0; i < NUM_ROWS; i++)
        {
            int value = GPIO_read(rowPins[i]);
            if (value == 0) // Button pressed
            {
                GPIO_write(colPins[j], HIGH);
                return keys[i][j];
            }
        }
    }
    return '\0';
}
