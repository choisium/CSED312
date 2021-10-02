#ifndef THREADS_FIXED_POINT_H
#define THREADS_FIXED_POINT_H

typedef int float_t;

/* Converting float to int and vice versa */
float_t int_to_float (int n);
int float_to_int_rounding_to_zero (float_t x);
int float_to_int_rounding_to_nearest (float_t x);

/* Arithmetic between two floats */
float_t add_float (float_t x, float_t y);
float_t sub_float (float_t x, float_t y);
float_t mul_float (float_t x, float_t y);
float_t div_float (float_t x, float_t y);

/* Arithmetic between float and int */
float_t add_float_and_int (float_t x, int n);
float_t sub_int_from_float (float_t x, int n);
float_t mul_float_by_int (float_t x, int n);
float_t div_float_by_int (float_t x, int n);

#endif /* threads/floating-point.h */
