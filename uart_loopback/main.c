#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>

#define UART_DEVICE "/dev/ttyS0"
#define UART_BAUD_RATE B57600

// Инициализация UART
int UART_Init(const char *device, speed_t baudRate)
{
    int uart_fd = open(device, O_RDWR | O_NOCTTY);  // Убрали O_NONBLOCK
    if (uart_fd == -1)
    {
        perror("Error opening UART");
        return -1;
    }

    struct termios options;
    tcgetattr(uart_fd, &options);

    cfsetispeed(&options, baudRate);
    cfsetospeed(&options, baudRate);

    options.c_cflag = CS8 | CLOCAL | CREAD;
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;

    tcflush(uart_fd, TCIFLUSH);
    tcsetattr(uart_fd, TCSANOW, &options);

    return uart_fd;
}

// Отправка данных
void UART_Write(int uart_fd, const char *data, int size)
{
    int bytes_written = write(uart_fd, data, size);
    if (bytes_written == -1)
    {
        perror("Error writing to UART");
    }
    else
    {
        printf("Sent %d bytes: %s\n", bytes_written, data);
    }
    tcdrain(uart_fd);  // Ждём, пока данные полностью отправятся
}

// Чтение данных
void UART_Read(int uart_fd, char *buffer, int size)
{
    int bytes_read = read(uart_fd, buffer, size - 1);
    if (bytes_read > 0)
    {
        buffer[bytes_read] = '\0';
        printf("Received: %s\n", buffer);
    }
    else
    {
        perror("Error reading from UART");
    }
}

int main()
{
    int uart_fd = UART_Init(UART_DEVICE, UART_BAUD_RATE);
    if (uart_fd == -1)
        return EXIT_FAILURE;

    const char *test_data = "Hello, UART!";
    char buffer[256];

    // Отправляем данные
    UART_Write(uart_fd, test_data, strlen(test_data));

    // Ожидание, чтобы данные успели передаться
    usleep(200000);

    // Читаем ответ
    UART_Read(uart_fd, buffer, sizeof(buffer));

    close(uart_fd);
    return EXIT_SUCCESS;
}
