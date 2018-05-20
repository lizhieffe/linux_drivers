/*
 * Test for file poll.
 * Need to write file in another thread for poll to be ready.
 */

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <iostream>

#define NUMBER_OF_BYTE 100
#define CHAR_DEVICE "/dev/sram-dev"

char data[NUMBER_OF_BYTE];

int main(int argc, char **argv) {
  int fd, retval;
  ssize_t read_count;
  fd_set readfds;

  std::cout << "Opening..." << std::endl;
  fd = open(CHAR_DEVICE, O_RDONLY);
  if (fd < 0) {
    std::cerr << "Cannot open device: " << CHAR_DEVICE << std::endl;
    return 1;
  }

  while (1) {
    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);

    std::cout << "Selecting..." << std::endl;
    // One needs to be notified of "read" events only, without timeout. This call
    // will put the process to sleep untile it is notified the event for which it
    // registered itself.
    retval = select(
        fd + 1,
        &readfds,
        NULL, /* write fds */
        NULL, /* error fds */
        NULL  /* timeout*/ );

    // From this line, the process has been notified already.
    if (retval == -1) {
      std::cerr << "Select call on " << CHAR_DEVICE << ": an error ocurred";
      break;
    }

    std::cout << "Select is ready.";
    // File descriptor is now ready.
    if (FD_ISSET(fd, &readfds)) {
      read_count = read(fd, data, NUMBER_OF_BYTE);

      if (read_count < 0) {
        std::cerr << "Cannot read file: " << CHAR_DEVICE;
      } else {
        if (read_count != NUMBER_OF_BYTE) {
          std::cout << "We have read less bytes than: " << NUMBER_OF_BYTE;
        }
        std::cout << "Read data: " << data;

        break;
      }

    }
  }

  close(fd);
  return EXIT_SUCCESS;
}
