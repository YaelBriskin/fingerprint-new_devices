#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include  "packet.h"
#include  "UART.h"
#include  "defines.h"


#define UART_DEVICE "/dev/ttyS0"  
#define BAUD_RATE B57600   

int fpm_fd;
extern uint8_t fingerID[2];

void read_sensor_parameters() 
{  
    if (verifyPassword())
        printf("Found fingerprint sensor!\n");
    else
        printf("Did not find fingerprint sensor :( \n");


    int template_count = getTemplateCount();
    printf("The template count: %d\n", template_count);
    
    uint8_t parameters = getParameters();  

    printf("Parameters:\n");
    printParameters(parameters);  
}

int findFinger()
{
	struct timespec start_time;
	const int max_execution_time = 20;
	struct timespec current_time;
	char num[2] = {0};
    int finger_id = 0;
	int ack = -1;
	int previous_ack = -1;
	printf("Waiting finger to enroll\n");
	// Main loop for scanning the fingerprint
	while (ack != FINGERPRINT_OK)
	{
		// detecting finger and store the detected finger image in ImageBuffer while returning successfull confirmation code;
		// If there is no finger, returned confirmation code would be cant detect finger.
		ack = (int)getImage();
		if (ack != previous_ack)
		{
			printf("ack = %d\n",ack);
			// Handle different response codes
			switch (ack)
			{
			case FINGERPRINT_OK:
				printf("Image taken\n");
				break;
			case FINGERPRINT_NOFINGER:
				printf("Can't detect finger\n");
				break;
			case FINGERPRINT_IMAGEFAIL:
				printf("Imaging error\n");
				break;
			case FINGERPRINT_PACKETRECIEVER:
				printf("Communication error\n");
				break;
			default:
				printf("Error receiving package\n");
				break;
			}
			previous_ack = ack; // Update previous_ack for next iteration
		}
		usleep(8000);
	}
	// to generate character file from the original finger image in ImageBuffer and store the file in CharBuffer1 or CharBuffer2.
	// to generate character file from the original finger image in ImageBuffer and store the file in CharBuffer1 or CharBuffer2.
	// Convert image to template
	ack = image2Tz(1);
	// Handle different response codes
	switch (ack)
	{
	case FINGERPRINT_OK:
		printf("Generate character file complete\n");
		break;
	case FINGERPRINT_PACKETRECIEVER:
		printf("Error when receiving package\n");
		break;
	case FINGERPRINT_IMAGEMESS:
		printf("Fail to generate character file due to the over-disorderly fingerprint image\n");
		break;
	case FINGERPRINT_FEATUREFAIL:
		printf("Fail to generate character file due to lackness of character point or over-smallness of fingerprint image\n");
		break;
	case FINGERPRINT_INVALIDIMAGE:
		printf("Fail to generate the image for the lackness of valid primary image\n");
		break;
	default:;
	}

	if (ack != FINGERPRINT_OK)
		return FAILED;
	// Search for fingerprint in database
	ack = fingerFastSearch();
	printf("ack = %d\n",ack);
	// Handle different response codes
	switch (ack)
	{
	// checks how the procedure went. FINGERPRINT_OK means good
	case FINGERPRINT_OK:
        finger_id = (fingerID[1] | (fingerID[0] << 8));
		printf("Hello ID #%d", finger_id);
        return finger_id;
	case FINGERPRINT_PACKETRECIEVER:
		printf("Error receiving package\n");
		break;
	case FINGERPRINT_NOTFOUND:
		printf("Finger not found\n");;
		return ERROR;
	default:;
	}
	if (ack != FINGERPRINT_OK)
		return FAILED;
}
int main() 
{
    fpm_fd = UART_Init(UART_DEVICE, BAUD_RATE);
    if (fpm_fd == FAILED) {
        return -1; // Ошибка инициализации
    }

    ReadSysPara parameters;
    read_sensor_parameters();

    int result= findFinger();    
    return 0;
}
