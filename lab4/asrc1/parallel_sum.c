#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>
#include <getopt.h>

#include <pthread.h>

#include "utils.h"
#include "sum.h"


void *ThreadSum(void *args) {
  struct SumArgs *sum_args = (struct SumArgs *)args;
  return (void *)(size_t)Sum(sum_args);
}

int main(int argc, char **argv) {

  uint32_t threads_num = 0;
  uint32_t array_size = 0;
  uint32_t seed = 0;
  // pthread_t threads[threads_num];

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {
        {"threads_num", required_argument, 0, 0},
        {"seed", required_argument, 0, 0},
        {"array_size", required_argument, 0, 0},
        {0, 0, 0, 0}
    };

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            threads_num = atoi(optarg);
            break;
          case 1:
            seed = atoi(optarg);
            break;
          case 2:
            array_size = atoi(optarg);
            break;
        }
        break;
      case '?':
        break;
      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (threads_num == 0 || array_size == 0 || seed == 0) {
    printf("Usage: %s --threads_num \"num\" --seed \"num\" --array_size \"num\"\n", argv[0]);
    return 1;
  }

  if (threads_num > array_size) {
      threads_num = array_size;
  }

  /*
   * TODO:
   * your code here
   * Generate array here
   */

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);

  pthread_t *threads = malloc(sizeof(pthread_t) * threads_num);
  struct SumArgs *args = malloc(sizeof(struct SumArgs) * threads_num);

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  int chunk_size = array_size / threads_num;

  for (uint32_t i = 0; i < threads_num; i++) {
    args[i].array = array;
    args[i].begin = i * chunk_size;
    args[i].end = (i == threads_num - 1) ? array_size : (i + 1) * chunk_size;

    if (pthread_create(&threads[i], NULL, ThreadSum, (void *)&args[i])) {
      printf("Error: pthread_create failed!\n");
      return 1;
    }
  }
   

  // struct SumArgs args[threads_num];
  // for (uint32_t i = 0; i < threads_num; i++) {
  //   if (pthread_create(&threads[i], NULL, ThreadSum, (void *)&args)) {
  //     printf("Error: pthread_create failed!\n");
  //     return 1;
  //   }
  // }

  int total_sum = 0;
  for (uint32_t i = 0; i < threads_num; i++) {
    void *thread_result;
    pthread_join(threads[i], &thread_result);
    total_sum += (int)(size_t)thread_result; 
  }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;


  free(array);
  free(threads);
  free(args);

  printf("Total: %d\n", total_sum);
  printf("Elapsed time: %f ms\n", elapsed_time);
  return 0;
}
