#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

#define DEVICE_PATH "/dev/S3554025Device"
#define OSP_HELLOWORLD_MESSAGE "This is assignment 1 message to my device"

int main()
{
	// Open device
	printf("[INFO] Opening device...\n");
	int file_ref = open(DEVICE_PATH, O_RDWR);

	if(file_ref < 0)
	{
		perror("[ERROR] Write failed, app will quit.\n");
		return errno;
	}
	else
	{
		printf("[SUCCESS] ...device opened.\n");
	}

	// Write to device
	printf("[INFO] Writing to device...\n");
	int write_result = write(
			file_ref,
			OSP_HELLOWORLD_MESSAGE,
			strlen(OSP_HELLOWORLD_MESSAGE));

	if(write_result < 0)
	{
		perror("[ERROR] Failed to write to the device!\n");
		return errno;
	}
	else
	{
		printf("[SUCCESS] ...message written.\n");
	}

	// Read from device
	char message_buffer[100];

	int read_result = read(file_ref, message_buffer, 100);
	if(read_result < 0)
	{
		perror("[ERROR] Failed to read from the device!\n");
		return errno;
	}
	else
	{
		printf("[SUCCESS] Message read: \"%s\"", message_buffer);
		printf("[SUCCESS] All test passed!\n");
		return 0;
	}
}
