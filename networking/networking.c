#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <syslog.h>
#include <sys/wait.h>

#define CONFIG_FILE "/home/pi/fingerprint_raspberry_pi/fingerprint/networking/config_networking.conf"

// Структура для хранения конфигурации сети
typedef struct {
    char ssid[64];
    char password[64];
} NetworkConfig_t;

// Чтение конфигурации из файла
int read_network_config(NetworkConfig_t *config) {
    FILE *file = fopen(CONFIG_FILE, "r");
    if (!file) {
        syslog(LOG_ERR, "Could not open network config file: %s", CONFIG_FILE);
        return -1;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) 
    {
        if (strncmp(line, "ssid=", 5) == 0) 
        {
            strcpy(config->ssid, line + 5);
            config->ssid[strcspn(config->ssid, "\n")] = '\0'; 
        } 
        else if (strncmp(line, "password=", 9) == 0) 
        {
            strcpy(config->password, line + 9);
            config->password[strcspn(config->password, "\n")] = '\0';  
        }
    }

    fclose(file);
    if (strlen(config->ssid) == 0 || strlen(config->password) == 0) {
        syslog(LOG_ERR, "Error: SSID or password is missing in config file.");
        return -1;
    }

    syslog(LOG_INFO, "Read network config: SSID=%s", config->ssid);
    return 0;
}

int connect_to_network(const NetworkConfig_t *config) 
{
    syslog(LOG_INFO, "Connecting to SSID: %s", config->ssid);
    pid_t pid = fork();
    if (pid == -1) 
    {
        syslog(LOG_ERR, "Failed to fork process: %s", strerror(errno));
        return 1;
    }

    if (pid == 0) 
    {
        execlp("nmcli", "nmcli", "dev", "wifi", "connect", config->ssid, "password", config->password, (char *)NULL);
        syslog(LOG_ERR, "Failed to execute nmcli");
        exit(1);
    } 
    else 
    {
        // Родительский процесс ожидает завершения
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            syslog(LOG_INFO, "Successfully connected to the network.");
            return 0;
        }
    }
    return 1;
}

int main() 
{
    openlog("networking_daemon", LOG_PID | LOG_CONS, LOG_DAEMON);
    syslog(LOG_INFO, "Daemon started");
    NetworkConfig_t config;

    if (read_network_config(&config) == -1) 
    {
        syslog(LOG_ERR, "Failed to read network configuration");
        closelog();
        return 1;
    }
    if (connect_to_network(&config) != 0) 
    {
        syslog(LOG_ERR, "Network connection failed");
        closelog();
        return 1;
    }
    syslog(LOG_INFO, "Network connected successfully. Starting fingerprint daemon...");
    return 0;
}
