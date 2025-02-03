#include "UART.h"

int timeout_ms = 60000; // Тайм-аут 60 секунд
/**
 * @brief Initializes the UART interface.
 *
 * @param UART_DEVICE The UART device file.
 * @param UART_BaudRate The baud rate for the UART communication.
 * @return int The file descriptor for the UART device, or 0 on error.
 */
int UART_Init(const char *UART_DEVICE, speed_t UART_BaudRate)
{
    int uart_fd = open(UART_DEVICE, O_RDWR | O_NOCTTY);
    if (uart_fd == ERROR)
    {
        // Log the error if the UART device could not be opened
        printf("Error opening UART\n");
        return FAILED;
    }
    struct termios options;
    // Get the current configuration of the UART
    tcgetattr(uart_fd, &options);
    // Set the baud rate for input and output
    cfsetispeed(&options, UART_BaudRate);
    cfsetospeed(&options, UART_BaudRate);
    // Configure the UART for 8 data bits, no parity, 1 stop bit
    options.c_cflag = (options.c_cflag & ~CSIZE) | CS8;
    options.c_iflag = IGNBRK;
    options.c_lflag = 0;
    options.c_oflag = 0;
    options.c_cflag |= CLOCAL | CREAD;
    // Set VMIN and VTIME
    options.c_cc[VMIN] = 1;
    options.c_cc[VTIME] = 5;
    // Disable software flow control
    options.c_iflag &= ~(IXON | IXOFF | IXANY);
    // Disable parity
    options.c_cflag &= ~(PARENB | PARODD);
    // Apply the settings to the UART immediately
    tcsetattr(uart_fd, TCSANOW, &options);
    // Flush the input (read) buffer
    tcflush(uart_fd, TCIOFLUSH);
    return uart_fd;
}

/**
 * @brief Writes data to the UART interface.
 *
 * @param uart_fd The file descriptor for the UART device.
 * @param data The data to write.
 * @param size The size of the data to write.
 */
void UART_write(int uart_fd, const char *data, int size)
{
    int retries_UART_write = 0;
    while (retries_UART_write < 10)
    {
        int ret = write(uart_fd, data, size);
        if (ret == size)
            break;
        else if (ret == ERROR)
        {
            // Log the error if the write operation fails
            printf("Failed to write to UART\n");
            exit(EXIT_FAILURE);
        }
        else if (ret != size)
        {
            // Log partial writes
            printf("Error: Only %d out of %d bytes were written!\n", ret, size);
        }
        retries_UART_write++;
        usleep(DELAY);
    }

    if (retries_UART_write == 10)
    {
        // Log the error if maximum retries are reached
        printf("Error: Maximum retries reached\n");
    }
}

/**
 * @brief Reads data from the UART interface.
 *
 * @param uart_fd The file descriptor for the UART device.
 * @param buffer The buffer to store the read data.
 * @param size The size of the buffer.
 * @return int 1 if the read operation is successful, 0 otherwise.
 */
Status_t UART_read(int uart_fd, char *buffer, int size)
{
    static bool read_error_logged = false;
    int retries_UART_read = 0;
    while (retries_UART_read < 10)
    {
        int bytes_read = 0;
        while (bytes_read < size) // Пытаемся прочитать весь пакет
        {
            int ret = read(uart_fd, buffer + bytes_read, size - bytes_read);
            if (ret < 0)
            {
                // Логируем ошибку
                printf("Error reading from UART\n");
                return FAILED;
            }
            bytes_read += ret;
        }

        if (bytes_read == size)
        {
            return SUCCESS; // Если прочитано нужное количество данных
        }
        else
        {
            // Логируем ошибку, если получено меньше байтов
            if (!read_error_logged)
            {
                printf("Error: Expected %d bytes, got %d bytes\n", size, bytes_read);
                read_error_logged = true;
            }
            retries_UART_read++;
            usleep(DELAY * 2);
        }
    }
    return FAILED;
}

/**
 * @brief Closes the UART interface.
 *
 * @param uart_fd The file descriptor for the UART device.
 */
void UART_close(int uart_fd)
{
    close(uart_fd);
}