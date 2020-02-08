#pragma once

#include <iostream>

void builtin_print(int value) { std::cout << value << std::endl; }

void out(int *ptr) { *ptr = 0; }

void builtin_malloc(int size, int **out) { *out = (int *)malloc(size); }

void builtin_free(int *ptr) { free(ptr); }
