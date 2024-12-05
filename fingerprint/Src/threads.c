#include "../Inc/threads.h"

// Flag to stop threads
extern volatile sig_atomic_t stop;

//-------------display
extern pthread_mutex_t displayMutex;
extern pthread_cond_t displayCond;

//-------------database
pthread_cond_t databaseCond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t databaseMutex = PTHREAD_MUTEX_INITIALIZER;

//-------------POST request
pthread_cond_t requestCond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t requestMutex = PTHREAD_MUTEX_INITIALIZER;

/**
* @brief Checks the file size and clears it if it exceeds the maximum allowed size.
*
* The function checks if the file exists, and if its size is greater than the maximum allowed size,
* then clears it. Otherwise, an error is returned.
*
* @param file_name The name of the file to check and clear.
*/
void check_and_clear_file(const char *file_name)
{
    struct stat st;
    if (stat(file_name, &st) == 0)
    {
        if (st.st_size > MAX_FILE_SIZE)
        {
            FILE *file = fopen(file_name, "w");
            if (file != NULL)

                fclose(file);
            else

                perror("Failed to clear file");
        }
    }
    else
    {
        perror("Failed to get file status");
    }
}
/**
 * @brief This function returns the current time in UTC format as an integer timestamp.
 *
 * @return The current UTC time as an integer timestamp.
 */
int getCurrent_UTC_Timestamp()
{
    // Get the current time
    time_t current_time = time(NULL);
    return (int)current_time;
}

/**
 * @brief This function activates a buzzer for a short duration.
 *
 * This function turns on a buzzer, waits for a predefined short duration, and then turns off the buzzer.
 */
void buzzer()
{
    //turn on
    if (GPIO_write(GPIO_BUZZER, BUZZER_ON) == FAILED)
    {
        LOG_MESSAGE(LOG_ERR, __func__, "stderr", "Failed to turn on buzzer", NULL);
        return;
    }
    
    usleep(SLEEP_BUZZER);

    //turn off
    if (GPIO_write(GPIO_BUZZER, BUZZER_OFF) == FAILED)
    {
        LOG_MESSAGE(LOG_ERR, __func__, "stderr", "Failed to turn off buzzer", NULL);
    }
}

/**
 * @brief This function runs in a separate thread to periodically check for unsent data in the database and send it to the server.
 *
 * The function operates in an infinite loop, checking every two minutes if there is data in the database that has not been sent.
 * If it fails to send data 10 times in a row, it turns on a LED indicator.
 *
 * @param arg Unused parameter.
 * @return Always returns NULL.
 */
void *databaseThread(void *arg)
{
    int count = 0;
    struct timespec timeout;

    while (!stop)
    {
        if (GPIO_write(GPIO_LED_RED, LED_OFF) == FAILED)
        {
            LOG_MESSAGE(LOG_ERR, __func__, "stderr", "Failed to set GPIO_LED_RED to OFF", NULL);
            pthread_exit(NULL);
        }
        // checks whether there is data in the database that has not yet been sent and
        // if there is any, it sends it to the server
        if (DB_find() != 1)
        {
            // if it fails to send data 10 times in a row, the LED will light up
            if (++count == g_max_retries)
            {
                // led on
                if (GPIO_write(GPIO_LED_RED, LED_ON) == FAILED)
                {
                    LOG_MESSAGE(LOG_ERR, __func__, "stderr", "Failed to set GPIO_LED_RED to ON", NULL);
                    pthread_exit(NULL);
                }
            }
        }
        else
        {
            count = 0;
            // Turn off the LED if data is sent successfully
            if (GPIO_write(GPIO_LED_RED, LED_OFF) == FAILED)
            {
                LOG_MESSAGE(LOG_ERR, __func__, "stderr", "Failed to set GPIO_LED_RED to OFF", NULL);
                pthread_exit(NULL);
            }
        }
        clock_gettime(CLOCK_REALTIME, &timeout);
        timeout.tv_sec += g_db_sleep;

        pthread_mutex_lock(&databaseMutex);
        pthread_cond_timedwait(&databaseCond, &databaseMutex, &timeout);
        pthread_mutex_unlock(&databaseMutex);
    }
    pthread_exit(NULL);
}

/**
 * @brief This function runs in a separate thread to update the display with the current time and manage database records.
 *
 * This function updates the LCD display with the current time and manages old records in the database.
 * It waits for a signal or a timeout to update the display.
 *
 * @param arg Unused parameter.
 * @return Always returns NULL.
 */
void *clockThread(void *arg)
{
    struct timespec timeout;
    int lastDay = -1;

    while (!stop)
    {
        lcd16x2_i2c_clear();
        usleep(1000);
        time_t rawtime;
        struct tm *timeinfo;
        time(&rawtime);
        timeinfo = localtime(&rawtime);

        // If the day has changed since the last check, it deletes old records from the database.
        if (lastDay != -1 && lastDay != timeinfo->tm_mday)
        {
            DB_delete_old_records(rawtime);
        }

        // Updates the last checked day
        lastDay = timeinfo->tm_mday;

        char timeString[TIME_STR_LEN] = {'\0'};
        strftime(timeString, TIME_STR_LEN, "%H:%M %d/%m/%y", timeinfo);
        pthread_mutex_lock(&displayMutex);
        while (!stop)
        {
            // pthread_cond_wait(&displayCond, &displayMutex);
            clock_gettime(CLOCK_REALTIME, &timeout);
            timeout.tv_sec += 1;
            // Wait for a signal or timer to expire
            int result = pthread_cond_timedwait(&displayCond, &displayMutex, &timeout);
            // If time has expired, exit the wait loop
            if (result == ETIMEDOUT)
            {
                break;
            }
        }
        // Updates the display with the current time.
        lcd16x2_i2c_puts(0, 0, timeString);
        lcd16x2_i2c_puts(1, 0, g_lcd_message);
        // Wait for one minute before updating the display again
        clock_gettime(CLOCK_REALTIME, &timeout);
        timeout.tv_sec += ONE_MINUTE;
        pthread_cond_timedwait(&displayCond, &displayMutex, &timeout);
        pthread_mutex_unlock(&displayMutex);
    }
    pthread_exit(NULL);
}

/**
 * @brief This function runs in a separate thread to periodically send POST requests to the server.
 *
 * The function sends POST requests to the server at regular intervals and processes the server's response.
 *
 * @param arg Unused parameter.
 * @return Always returns NULL.
 */
void *post_requestThread(void *arg)
{
    struct timespec timeout;
    while (!stop)
    {
        check_and_clear_file(FILE_NAME);
        int result = send_get_request(g_url_delete_employee);
        if (result != SUCCESS)
        {
            writeToFile(file_URL, __func__, "Failed to send request for deletions.");
            continue;
        }
        // Set the timeout for the next request
        clock_gettime(CLOCK_REALTIME, &timeout);
        timeout.tv_sec += CHECK_INTERVAL;

        pthread_mutex_lock(&requestMutex);
        pthread_cond_timedwait(&requestCond, &requestMutex, &timeout);
        pthread_mutex_unlock(&requestMutex);
    }
    pthread_exit(NULL);
}
