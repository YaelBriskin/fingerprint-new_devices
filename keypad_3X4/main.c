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
#include "./Inc/keypad.h"

// Функция для обработки сигнала SIGINT (Ctrl+C)
void handle_signal(int signal) 
{
    // Закрытие всех ресурсов
    GPIO_close();  // Закрываем GPIO перед завершением программы
    printf("GPIO resources closed.\n");
    exit(0);  // Завершаем программу
}

int main() 
{
    if (keypad_init() == FAILED)
    {
        printf("Failed to initialize keypad\n");
        return -1;
    }

    while (1)
    {
        char key = read_keypad_value();
        if (key != '\0') // Если клавиша нажата
        {
            printf("Key pressed: %c\n", key);
        }
    }
    GPIO_close();
    return EXIT_SUCCESS;
}