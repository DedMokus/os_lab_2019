#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main() {
  pid_t child_pid = fork();

  if (child_pid == 0) {
    // child process
    exit(0);
  } else {
    // parent process
    while (1) {
      sleep(1);
    }
  }

  return 0;
}