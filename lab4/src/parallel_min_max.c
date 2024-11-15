#include <signal.h>
#include <stdbool.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

int active_child_processes = 0;

void alarm_handler(int sig) {
  printf("Timeout exceeded. Killing child processes...\n");
  fflush(NULL);
  kill(0, SIGKILL);
}

int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  int pnum = -1;
  bool with_files = false;
  int timeout = -1;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"by_files", no_argument, 0, 'f'},
                                      {"timeout", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            if (seed <= 0) {
              printf("Seed must be a positive integer\n");
              return 1;
            }
            break;
          case 1:
            array_size = atoi(optarg);
            if (array_size <= 0) {
              printf("Array size must be a positive integer\n");
              return 1;
            }
            break;
          case 2:
            pnum = atoi(optarg);
            if (pnum <= 0) {
              printf("Number of processes must be a positive integer\n");
              return 1;
            }
            break;
          case 3:
            with_files = true;
            break;
          case 4:
            timeout = atoi(optarg);
            if (timeout <= 0) {
              printf("Timeout must be a positive integer\n");
              return 1;
            }
            break;
          default:
            printf("Index %d is out of options\n", option_index);
        }
        break;
      case 'f':
        with_files = true;
        break;

      case '?':
        break;

      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (optind < argc) {
    printf("Has at least one no option argument\n");
    return 1;
  }

  if (seed == -1 || array_size == -1 || pnum == -1) {
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" [--timeout \"num\"]\n",
           argv[0]);
    return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  int pipefd[2];
  if (!with_files) {
    if (pipe(pipefd) == -1) {
      perror("pipe");
      exit(EXIT_FAILURE);
    }
  }

  signal(SIGALRM, alarm_handler);
  if (timeout > 0) {
    alarm(timeout);
  }

  for (int i = 0; i < pnum; i++) {
    pid_t child_pid = fork();
    if (child_pid >= 0) {
      // successful fork
      active_child_processes += 1;
      if (child_pid == 0) {
        // child process

        // parallel somehow
        struct MinMax min_max = GetMinMax(array, i * array_size / pnum, (i + 1) * array_size / pnum);

        if (with_files) {
          // use files here
          char filename[20];
          sprintf(filename, "file%d", i);
          FILE* file = fopen(filename, "w");
          fwrite(&min_max, sizeof(struct MinMax), 1, file);
          fclose(file);
        } else {
          // use pipe here
          close(pipefd[0]);
          write(pipefd[1], &min_max, sizeof(struct MinMax));
          close(pipefd[1]);
        }
        return 0;
      }

    } else {
      printf("Fork failed!\n");
      return 1;
    }
  }

  while (active_child_processes > 0) {
    // your code here
    int status;
    pid_t child_pid = wait(&status);
    if (child_pid >= 0) {
      active_child_processes -= 1;
    }
  }

  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = 0; i < pnum; i++) {
    struct MinMax part_min_max;

    if (with_files) {
      // read from files
      char filename[20];
      sprintf(filename, "file%d", i);
      FILE* file = fopen(filename, "r");
      fread(&part_min_max, sizeof(struct MinMax), 1, file);
      fclose(file);
    } else {
      // read from pipes
      close(pipefd[1]);
      read(pipefd[0], &part_min_max, sizeof(struct MinMax));
      close(pipefd[0]);
    }

    if (part_min_max.min < min_max.min) min_max.min = part_min_max.min;
    if (part_min_max.max > min_max.max) min_max.max = part_min_max.max;
  }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);

  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);
  fflush(NULL);
  return 0;
}