#include "sum.h"

int Sum(const struct SumArgs *args) {
  int sum = 0;
  // Считаем сумму элементов массива от begin до end (не включая end)
  for (int i = args->begin; i < args->end; i++) {
    sum += args->array[i];
  }
  return sum;
}