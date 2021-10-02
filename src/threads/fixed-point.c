#include "threads/fixed-point.h"
#include <stdint.h>

#define p 17
#define q 14

/* Fixed-point real arithmetic. We need to calculate recent_cpu
   and load_avg for advanced scheduling in project 1. Pintos does
   not support floating-point arithmetic in kernel, so we use
   integers and use below functions to do real arithmetic.
   
   We can use p.q fixed-point format where lowest q bits represents
   a fraction and p represents decimal part with p + q = 31.

   In the comments for each function, x and y are fixed-point
   numbers in signed p.q format where p + q = 31, n is an integer
   and f is 1 << q. */

static const int f = 1 << q;

float_t int_to_float (int n) {
    return n * f;
}

int float_to_int_rounding_toward_zero (float_t x) {
    return x / f;
}

int float_to_int_rounding_to_nearest (float_t x) {
    return x >= 0 ? (x + f/2) / f : (x - f/2) / f;
}

float_t add_float (float_t x, float_t y) {
    return x + y;
}

float_t sub_float (float_t x, float_t y) {
    return x - y;
}

float_t add_float_and_int (float_t x, int n) {
    return x + n * f;
}

float_t sub_int_from_float (float_t x, int n) {
    return x - n * f;
}

float_t mul_float (float_t x, float_t y) {
    return ((int64_t) x) * y / f;
}

float_t mul_float_by_int (float_t x, int n) {
    return x * n;
}

float_t div_float (float_t x, int y) {
    return ((int64_t) x) * f / y;
}

float_t div_float_by_int (float_t x, int n) {
    return x / n;
}
