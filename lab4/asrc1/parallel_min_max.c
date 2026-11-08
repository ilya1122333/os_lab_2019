#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <signal.h> 
#include <errno.h> 

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

bool timeout_reached = false;

void handle_alarm(int sig) {
  timeout_reached = true;
}

int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  int pnum = -1;
  int timeout = -1;
  bool with_files = false;

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
              printf("seed must be a positive number\n");
              return 1;
            }
            break;
          case 1:
            array_size = atoi(optarg);
            if (array_size <= 0) {
              printf("array_size must be a positive number\n");
              return 1;
            }
            break;
          case 2:
            pnum = atoi(optarg);
            if (pnum <= 0) {
              printf("pnum must be a positive number\n");
              return 1;
            }
            // your code here
            // error handling
            break;
          case 3:
            with_files = true;
            break;

          case 4:
            timeout = atoi(optarg);
            if (timeout <= 0) {
              printf("timeout must be a positive number\n");
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
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" \n",
           argv[0]);
    return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);
  int active_child_processes = 0;

  int pipefd[2];
  if (!with_files) {
    if (pipe(pipefd) == -1) {
      perror("Pipe creation failed");
      return 1;
    }
  }

  pid_t *child_pids = malloc(sizeof(pid_t) * pnum);

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  for (int i = 0; i < pnum; i++) {
    pid_t child_pid = fork();
    if (child_pid >= 0) {
      // successful fork
      if (child_pid == 0) {
        // child process

        // parallel somehow

        int chunk_size = array_size / pnum;
        int begin = i * chunk_size;
        int end = (i == pnum - 1) ? array_size : (i + 1) * chunk_size;

        struct MinMax local_min_max = GetMinMax(array, begin, end);

        if (with_files) {
          // use files here

          char filename[256];
          sprintf(filename, "tmp_data_%d.txt", i); // Уникальное имя файла для каждого процесса
          FILE *fp = fopen(filename, "w");
          if (fp) {
            fprintf(fp, "%d %d\n", local_min_max.min, local_min_max.max);
            fclose(fp);
          }

        } else {
          // use pipe here
          close(pipefd[0]); // Закрываем конец для чтения в дочернем процессе
          write(pipefd[1], &local_min_max, sizeof(struct MinMax));
          close(pipefd[1]); // Закрываем конец для записи после отправки данных

        }
        return 0;
      } else {
        // Запоминаем PID созданного процесса в родителе
        child_pids[i] = child_pid;
        active_child_processes += 1;
      }

    } else {
      printf("Fork failed!\n");
      return 1;
    }
  }

  // Закрываем конец для записи в родительском процессе, если используются пайпы
  if (!with_files) {
      close(pipefd[1]); 
  }

  while (active_child_processes > 0) {
    int status;
    pid_t pid = wait(&status);

    if (pid == -1) {
      // wait был прерван сигналом (SIGALRM)
      if (errno == EINTR && timeout_reached) {
        printf("\nTimeout %d seconds reached! Sending SIGKILL to children...\n", timeout);
        for (int i = 0; i < pnum; i++) {
          if (child_pids[i] > 0) {
            kill(child_pids[i], SIGKILL); // Убиваем оставшиеся процессы
          }
        }
        // Не выходим из цикла, так как убитые процессы нужно собрать с помощью wait
      }
    } else {
      // Какой-то процесс завершился (сам или был убит)
      active_child_processes -= 1;
      // Находим его в массиве и отмечаем как 0 (завершен)
      for (int i = 0; i < pnum; i++) {
        if (child_pids[i] == pid) {
          child_pids[i] = 0;
          break;
        }
      }
    }
  }


  if (timeout > 0) {
    alarm(0);
  }

  if (timeout_reached) {
    printf("Program aborted due to timeout.\n");
    free(array);
    free(child_pids);
    if (!with_files) close(pipefd[0]);
    return 1;
  }

  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = 0; i < pnum; i++) {
    int min = INT_MAX;
    int max = INT_MIN;

    if (with_files) {
      // read from files
      char filename[256];
      sprintf(filename, "tmp_data_%d.txt", i);
      FILE *fp = fopen(filename, "r");
      if (fp) {
        fscanf(fp, "%d %d", &min, &max);
        fclose(fp);
        remove(filename);
      }
    } else {
      // read from pipes
      struct MinMax child_min_max;
      read(pipefd[0], &child_min_max, sizeof(struct MinMax));
      min = child_min_max.min;
      max = child_min_max.max;
    }

    if (min < min_max.min) min_max.min = min;
    if (max > min_max.max) min_max.max = max;
  }

  if (!with_files) {
      close(pipefd[0]);
  }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);
  free(child_pids);

  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);
  fflush(NULL);
  return 0;
}
