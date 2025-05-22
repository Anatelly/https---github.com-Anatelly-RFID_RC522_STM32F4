/**
  ******************************************************************************
  * @file endianes.c
  * @author Vasiliy Turchenko
  * @version V0.0.1
  * @date     11-09-2020
  * @brief
  * @note This file is part of the interface_control_rtos project.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT STC </center></h2>
  ******************************************************************************
  */
#include "endianes.h"
#include "CAssert.h"

/*
 * todo: use on cm3 intrinsic: REV, REV16 and so on
 *
 */

CASSERT( (sizeof(float) == sizeof(uint32_t)),  endianes_c )

typedef union
{
    uint32_t dword;
    float    flt;
} dword_float_t;

/**
 * @brief read_float_lendian
 * @param src_lendian
 * @return
 */
float read_float_lendian(const void* src_lendian)
{
    dword_float_t x;
    x.dword = read_dword_lendian(src_lendian);
    return (x.flt);
}

/**
 * @brief read_float_bendian
 * @param src_bendian
 * @return
 */
float read_float_bendian(const void* src_bendian)
{
    dword_float_t x;
    x.dword = read_dword_bendian(src_bendian);
    return (x.flt);
}

/**
 * @brief write_float_lendian
 * @param dst_lendian
 * @param x
 */
void write_float_lendian(void* dst_lendian, float x)
{
    dword_float_t f;
    f.flt = x;
    write_dword_lendian(dst_lendian, f.dword);
}

/**
 * @brief write_float_bendian
 * @param dst_bendian
 * @param x
 */
void write_float_bendian(void* dst_bendian, float x)
{
    dword_float_t f;
    f.flt = x;
    write_dword_bendian(dst_bendian, f.dword);
}

/****************** TESTS ************************************/
/* TESTS were moved to the separate endianes_test.c/.h */

/* The code below is for standalone testing only */
#if (ENDIANES_TEST == 1)

#include <stdio.h>
#include <math.h>

#define FAILED do {printf("test failed at %s : %d\n", __FILE__, __LINE__);} while (0)
#define PASSED do {printf("test passed at %s : %d\n", __FILE__, __LINE__);} while (0)

void endianes_test(void)
{

    uint16_t    src = 0x1234U;


    uint16_t res1 = read_word_bendian(&src);
    if (res1 == 0x3412U) {
        PASSED;
    } else {
        FAILED;
    }
    uint16_t res2 = read_word_bendian(&res1);
    if (res2 == src) {
        PASSED;
    } else {
        FAILED;
    }

    uint16_t res3 = read_word_lendian(&src);
    if (res3 == src) {
        PASSED;
    } else {
        FAILED;
    }

    src = 0xFFFFU;
    res1 = read_word_bendian(&src);
    if (res1 == src) {
        PASSED;
    } else {
        FAILED;
    }

    src = 0x0000U;
    res1 = read_word_bendian(&src);
    if (res1 == src) {
        PASSED;
    } else {
        FAILED;
    }

    uint16_t    dst = 0U;

    write_word_bendian(&dst, 0x1234U);
    if (dst == 0x3412U) {
        PASSED;
    } else {
        FAILED;
    }

    write_word_lendian(&dst, 0x1234U);
    if (dst == 0x1234U) {
        PASSED;
    } else {
        FAILED;
    }

    uint32_t    src32 = 0xDEADBEEFU;
    uint32_t res10 = read_dword_bendian(&src32);
    if (res10 == 0xEFBEADDEU) {
        PASSED;
    } else {
        FAILED;
    }
    uint32_t res20 = read_dword_bendian(&res10);
    if (res20 == src32) {
        PASSED;
    } else {
        FAILED;
    }

    uint32_t    dst32 = 0U;

    write_dword_bendian(&dst32, src32);
    if (dst32 == 0xEFBEADDEU) {
        PASSED;
    } else {
        FAILED;
    }

    write_dword_lendian(&dst32, src32);
    if (dst32 == src32) {
        PASSED;
    } else {
        FAILED;
    }

    uint32_t b = 0U;

    write_float_bendian(&b, 100.54f);

    float dstf = read_float_bendian(&b);

    if (dstf == 100.54f) {
        PASSED;
    } else {
        FAILED;
    }

    b = 0U;
    write_float_lendian(&b, 100.54f);

    float dstf2 = read_float_lendian(&b);

    if (dstf == 100.54f) {
        PASSED;
    } else {
        FAILED;
    }

}

#endif
