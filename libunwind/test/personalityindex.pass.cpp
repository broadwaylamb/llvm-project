// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// Ensure that functions marked as signal frames are reported as such.

// REQUIRES: libunwind-arm-ehabi

#include <assert.h>
#include <stdlib.h>
#include <unwind.h>
#include <stdio.h>

#define EXPECTED_NUM_FRAMES 50
#define NUM_FRAMES_UPPER_BOUND 104

// A definition of a simple call_through function that uses
// __aeabi_unwind_cpp_pr2 as personality routine.
asm("    .globl call_through                                                 \n"
    "    .p2align        2                                                   \n"
    "    .type   call_through,%function                                      \n"
    "    .code   32                                                          \n"
    "call_through:                                                           \n"
    "    .fnstart                                                            \n"
    "    .personalityindex 2                                                 \n"
    "    .save   {r11, lr}                                                   \n"
    "    push    {r11, lr}                                                   \n"
    "    .setfp  r11, sp                                                     \n"
    "    mov     r11, sp                                                     \n"
    "    mov     r2, r0                                                      \n"
    "    mov     r0, r1                                                      \n"
    "    blx     r2                                                          \n"
    "    pop     {r11, pc}                                                   \n"
    ".Lfunc_end0:                                                            \n"
    "    .size call_through, .Lfunc_end0-call_through                        \n"
    "    .fnend                                                                "
);

extern "C" void call_through(void(*callback)(void*), void* context);

_Unwind_Reason_Code callback(_Unwind_Context *context, void *cnt) {
  (void)context;
  int *i = (int *)cnt;
  ++*i;
  if (*i > NUM_FRAMES_UPPER_BOUND) {
    fprintf(stderr,
            "callback(_Unwind_Context*, void*):"
            "    *i should not be greater than NUM_FRAMES_UPPER_BOUND. "
            "    Actual value: %d\n", *i);
    abort();
  }
  return _URC_NO_REASON;
}

void test_backtrace() {
  int n = 0;
  _Unwind_Backtrace(&callback, &n);
  if (n < EXPECTED_NUM_FRAMES) {
    fprintf(stderr,
            "test_backtrace():"
            "    n should not be less than EXPECTED_NUM_FRAMES. "
            "    Actual value: %d\n", n);
    abort();
  }
}

void test(void* context) {
  int* i = (int*)context;
  if (*i == 0) {
    test_backtrace();
  } else {
    int result = *i - 1;
    call_through(&test, &result); // the same as test(&result);
    *i += result;
  }
}

int main() {
  int n = 50;
  test(&n);
  assert(n == 1275);
}
