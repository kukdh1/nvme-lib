#include <stdio.h>

#include "nvme_low_lib.h"

void binary_print(unsigned char *buf, int len, int width)
{
	int i, offset = 0, line_done = 0;
	char ascii[width + 1];

	printf("     ");
	for (i = 0; i < width; i++) {
		printf("%3x", i);
	}
	for (i = 0; i < len; i++) {
		line_done = 0;
		if (i % width == 0)
			printf( "\n%04x:", offset);
		printf( " %02x", buf[i]);
		ascii[i % width] = (buf[i] >= '!' && buf[i] <= '~') ? buf[i] : '.';
		if (((i + 1) % width) == 0) {
			ascii[i % width + 1] = '\0';
			printf( "  %.*s", width, ascii);
			offset += width;
			line_done = 1;
		}
	}
	if (!line_done) {
		unsigned b = width - (i % width);
		ascii[i % width + 1] = '\0';
		printf(" %*s  %.*s",
				3 * b + 1, "",
				width, ascii);
	}
	printf( "\n");
}

int main(int argc, char *argv[]) {
  const char dev[] = "/dev/nvme0";
  nvme_cmd command;
  nvme_data data;
  nvme_error err;
  nvme_handle device;
  uint8_t *dptr;

  // Open device
  err = nvme_open(&device, dev);
  if (err) {
    printf("Error0: %d - %s\n", err, nvme_get_error_message(err));
    goto clean;
  }

  // Setting command
  memset(&command, 0, sizeof(nvme_cmd));
  command.cdw0 = 0x00000006;  // Identify
  command.cdw10 = 1;          // Identify controller

  // Allocate data pointer
  err = nvme_alloc_data(&data, 4096);
  if (err) {
    printf("Error1: %d - %s\n", err, nvme_get_error_message(err));
    goto clean;
  }

  // Send command
  err = nvme_submit_admin(device, &command, NULL, data);
  if (err) {
    printf("Error2: %d - %s\n", err, nvme_get_error_message(err));
    goto free_and_clean;
  }

  // Get data pointer
  err = nvme_get_data_pointer(data, (void **)&dptr);
  if (err) {
    printf("Error3: %d - %s\n", err, nvme_get_error_message(err));
    goto free_and_clean;
  }

  // Print result
  binary_print((char *)dptr, 1024, 16);

  // Close device
  nvme_close(device);

free_and_clean:
  nvme_free_data(data);

clean:

  return 0;
}
