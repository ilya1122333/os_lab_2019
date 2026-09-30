#!/bin/bash

count=$#
sum=0

for num in "$@"; do
    sum=$((sum + num))  
done

int=$((sum / count))

echo "Количество аргументов: $count"
echo "Сумма: $sum"
echo "Среднее арифметическое: $int"
