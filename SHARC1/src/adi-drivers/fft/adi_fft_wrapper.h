/* Copyright (C) 2013-2020 Analog Devices, Inc. All Rights Reserved.
**
** File version:  2.9.3.0
**
** Declarations of wrapper library functions for the FFT Accelerator.
*/

#ifndef __ADI_FFT_WRAPPER_H__
#define __ADI_FFT_WRAPPER_H__

#pragma system_header

#if !defined(__ADSPSC589_FAMILY__)
#error "This processor is not supported"
#endif

#include <stddef.h>
#if defined(__ADSPARM__) || defined(__BYTE_ADDRESSING__)
  #include <stdint.h>
#endif

#if defined(__ADSP21000__)

#if defined(__BYTE_ADDRESSING__)
  #include <drivers/fft/adi_fft.h>
#endif /* __BYTE_ADDRESSING__ */
  #include <complex.h>

#if defined(_MISRA_2004_RULES) || defined(_MISRA_2012_RULES)
#pragma diag(push)
#ifdef _MISRA_2004_RULES
#pragma diag(suppress:misra_rule_5_1:"Some of the APIs use more than 31 characters.")
#pragma diag(suppress:misra_rule_6_3:"The interfaces are defined in terms of the standard C types.")
#pragma diag(suppress:misra_rule_19_7:"Use macros to define size of TCB chain which must be compile-time constants.")
#pragma diag(suppress:misra_rule_19_4:"Use macros to remap drop-in FFT functions to accelerator APIs.")
#endif
#endif /* _MISRA_2004_RULES || _MISRA_2012_RULES */

  #pragma byte_addressed(push)

#elif defined(__ADSPARM__)

  #include <drivers/fft/adi_fft.h>
  #include <services/int/adi_gic.h>

  /* gcc doesn't have the appropriate definitions of complex_float.
  */

  typedef struct {
     float re;
     float im;
  } complex_float ;


  typedef struct {
     long double re;
     long double im;
  } complex_long_double ;

#ifdef __DOUBLES_ARE_FLOATS__          /* 32-bit doubles */
  typedef complex_float          complex_double;
#else                                  /* 64-bit doubles */
  typedef complex_long_double    complex_double;
#endif

  /* the ARM also doesn't understand PM and DM
  */
  #define __pm
  #define __dm

#endif

/* If the data input is smaller than this size, then we
** also need to pad out the data input with zeroes, in
** order to use the accelerator.
*/
#define MIN_POINTS_FOR_SMALL_FFT   64

/* If the data input is larger than this size, then we
** need to do the FFT in stages.
*/
#define MAX_POINTS_FOR_SMALL_FFT 2048

/* Largest data set the FFT accelerator can handle
*/
#define MAX_POINTS_FFT_ACCEL  4194304

#if defined(__ADSPARM__) || defined(__BYTE_ADDRESSING__)

typedef struct {
  uint32_t __arr[(uint32_t)ADI_FFT_MEMORY/sizeof(uint32_t)];
} ADI_FFT_DEVICE_MEMORY;

/*
** Error handling
*/

typedef enum {
  ADI_FFT_ERROR_UNKNOWN,
  ADI_FFT_ERROR_GIC,
  ADI_FFT_ERROR_FFT,
  ADI_FFT_ERROR_SIZE,
  ADI_FFT_ERROR_ALIGNMENT,
  ADI_FFT_ERROR_ALIGNMENT_CACHE,
  ADI_FFT_ERROR_TWIDDLES,
  ADI_FFT_ERROR_FIR_BLOCK_SIZE,
  ADI_FFT_ERROR_NUM_CHANNELS
} ADI_FFT_ERROR_KIND;

typedef void (*AccelFFTErrorHandler)(ADI_FFT_HANDLE     __h,
                                     ADI_FFT_ERROR_KIND __aek,
                                     void              *__error_handle,
                                     unsigned int       __error_code);

extern AccelFFTErrorHandler accel_fft_error_handler;

/* supported error handlers.
*/
#if defined(__cplusplus)
extern "C" {
#endif

void accel_fft_error_handler_null(ADI_FFT_HANDLE      __h,
                                  ADI_FFT_ERROR_KIND  __aek,
                                  void               *__error_handle,
                                  unsigned int        __error_code);
void accel_fft_error_handler_print(ADI_FFT_HANDLE     __h,
                                   ADI_FFT_ERROR_KIND __aek,
                                   void              *__error_handle,
                                   unsigned int       __error_code);

void accel_fft_set_error_handler(AccelFFTErrorHandler __f);

#if defined(__cplusplus)
}
#endif

#endif /* __BYTE_ADDRESSING__ */

#if !defined(_MISRA_2004_RULES) && !defined(_MISRA_2012_RULES)

/* a varargs macro can't be used in MISRA mode.
*/
#define adi_fft_Check(h, op, ...)                                         \
   do {                                                                   \
     ADI_FFT_RESULT result = (op);                                        \
     if (result != ADI_FFT_SUCCESS) {                                     \
       accel_fft_error_handler((h), ADI_FFT_ERROR_FFT, #op, (unsigned int)result);\
       __VA_ARGS__;                                                       \
     }                                                                    \
   } while (0)

#endif /* !defined(_MISRA_2004_RULES) && !defined(_MISRA_2012_RULES) */

/*
** Twiddle factors for various large FFT sizes
*/

extern const __pm complex_float accel_twiddles_4096[4096];
extern const __pm complex_float accel_twiddles_8192[8192];
extern const __pm complex_float accel_twiddles_16384[16384];
extern const __pm complex_float accel_twiddles_32768[32768];
extern const __pm complex_float accel_twiddles_65536[65536];

/* 
** Support for setting up DMA descriptor chains.
*/

#define ACCEL_FFT_TX_CONFIG(twod, twait, trig1d, trig2d, int1d, int2d, logmsize, nextdescsize) \
  (  ((twait) ? BITM_DMA_CFG_TWAIT : 0U) \
   | ((trig1d) ? ((uint32_t)1U << BITP_DMA_CFG_TRIG) : 0U) \
   | ((trig2d) ? ((uint32_t)2U << BITP_DMA_CFG_TRIG) : 0U) \
   | ((int1d) ? ((uint32_t)1U << BITP_DMA_CFG_INT) : 0U) \
   | ((int2d) ? ((uint32_t)2U << BITP_DMA_CFG_INT) : 0U) \
   | ((uint32_t)(logmsize) << BITP_DMA_CFG_MSIZE) \
   | (/*logpsize*/(uint32_t)3U << BITP_DMA_CFG_PSIZE) \
   | ((uint32_t)((nextdescsize) - 1U) << BITP_DMA_CFG_NDSIZE) \
   | (((nextdescsize) != 0U) ? ((uint32_t)4U << BITP_DMA_CFG_FLOW) : 0U) \
   | ((twod) ? BITM_DMA_CFG_TWOD : 0U) \
   | BITM_DMA_CFG_EN)

#define ACCEL_FFT_RX_CONFIG(twod, twait, trig1d, trig2d, int1d, int2d, logmsize, nextdescsize) \
  (  (BITM_DMA_CFG_WNR|BITM_DMA_CFG_SYNC) \
   | ((twait) ? BITM_DMA_CFG_TWAIT : 0U) \
   | ((trig1d) ? ((uint32_t)1U << BITP_DMA_CFG_TRIG) : 0U) \
   | ((trig2d) ? ((uint32_t)2U << BITP_DMA_CFG_TRIG) : 0U) \
   | ((int1d) ? ((uint32_t)1U << BITP_DMA_CFG_INT) : 0U) \
   | ((int2d) ? ((uint32_t)2U << BITP_DMA_CFG_INT) : 0U) \
   | ((uint32_t)(logmsize) << BITP_DMA_CFG_MSIZE) \
   | (/*logpsize*/(uint32_t)3U << BITP_DMA_CFG_PSIZE) \
   | ((uint32_t)((nextdescsize) - 1U) << BITP_DMA_CFG_NDSIZE) \
   | (((nextdescsize) != 0U) ? ((uint32_t)4U << BITP_DMA_CFG_FLOW) : 0U) \
   | ((twod) ? BITM_DMA_CFG_TWOD : 0U) \
   | BITM_DMA_CFG_EN)

#define __TCB_CHAIN_SIZE(iter2, tstride) \
  ((5U * 4U * 2U)                                           /* rx, both loops     */ + \
   (7U * 4U)                                                /* 1st loop tx        */ + \
   (7U * 4U)                                                /* 2nd loop, first tx */ + \
   (((tstride) == 1U ? 5U : 7U) * 2U * 4U * ((iter2) - 1U)) /* 2nd loop, later tx */ + \
   (((tstride) == 1U ? 5U : 7U) * 4U))                      /* 2nd loop, final tx */

/* This macro matches the size of the TCB chain created by the 
** accel_fft_large_set_tcbs function, and declares the size in
** bytes of the memory region that must be allocated by the caller of
** that function and passed in via the __tcb parameter.
*/
#define ACCEL_FFT_LARGE_TCB_CHAIN_SIZE(npts, twid_stride) \
  ((npts) == 4096 ? __TCB_CHAIN_SIZE(4U, (twid_stride)) \
   : ((npts) == 8192 ? __TCB_CHAIN_SIZE(8U, (twid_stride)) \
      : ((npts) == 16384 ? __TCB_CHAIN_SIZE(16U, (twid_stride)) \
         : ((npts) == 32768 ? __TCB_CHAIN_SIZE(32U, (twid_stride)) \
            : ((npts) == 65536 ? __TCB_CHAIN_SIZE(64U, (twid_stride)) \
               : ((npts) == 131072 ? __TCB_CHAIN_SIZE(128U, (twid_stride)) \
                  : ((npts) == 262144 ? __TCB_CHAIN_SIZE(256U, (twid_stride)) \
                     : ((npts) == 524288 ? __TCB_CHAIN_SIZE(512U, (twid_stride)) \
                        : ((npts) == 1048576 ? __TCB_CHAIN_SIZE(1024U, (twid_stride)) \
                           : ((npts) == 2097152 ? __TCB_CHAIN_SIZE(1024U, (twid_stride)) \
                              : ((npts) == 4194304 ? __TCB_CHAIN_SIZE(2048U, (twid_stride)) \
                                 : 0U)))))))))))

#define __WIN_TCB_CHAIN_SIZE(iter1, iter2, tstride) \
  ((5U * 4U * 2U)                                           /* rx, both loops     */ + \
   ((7U * 4U) * 2U * (iter1))                               /* 1st loop tx        */ + \
   (7U * 4U)                                                /* 2nd loop, first tx */ + \
   (((tstride) == 1U ? 5U : 7U) * 2U * 4U * ((iter2) - 1U)) /* 2nd loop, later tx */ + \
   (((tstride) == 1U ? 5U : 7U) * 4U))                      /* 2nd loop, final tx */

/* This macro matches the size of the TCB chain created by the 
** accel_fft_large_windowed_set_tcbs function, and declares the size in
** bytes of the memory region that must be allocated by the caller of
** that function and passed in via the __tcb parameter.
*/
#define ACCEL_FFT_LARGE_WINDOWED_TCB_CHAIN_SIZE(npts, twid_stride) \
  ((npts) == 4096 ? __WIN_TCB_CHAIN_SIZE(4U, 16U, (twid_stride)) \
   : ((npts) == 8192 ? __WIN_TCB_CHAIN_SIZE(8U, 32U, (twid_stride)) \
      : ((npts) == 16384 ? __WIN_TCB_CHAIN_SIZE(16U, 64U, (twid_stride)) \
         : ((npts) == 32768 ? __WIN_TCB_CHAIN_SIZE(32U, 128U, (twid_stride)) \
            : ((npts) == 65536 ? __WIN_TCB_CHAIN_SIZE(64U, 256U, (twid_stride)) \
               : ((npts) == 131072 ? __WIN_TCB_CHAIN_SIZE(128U, 256U, (twid_stride)) \
                  : ((npts) == 262144 ? __WIN_TCB_CHAIN_SIZE(256U, 512U, (twid_stride)) \
                     : ((npts) == 524288 ? __WIN_TCB_CHAIN_SIZE(512U, 512U, (twid_stride)) \
                        : ((npts) == 1048576 ? __WIN_TCB_CHAIN_SIZE(1024U, 1024U, (twid_stride)) \
                           : ((npts) == 2097152 ? __WIN_TCB_CHAIN_SIZE(2048U, 1024U, (twid_stride)) \
                              : ((npts) == 4194304 ? __WIN_TCB_CHAIN_SIZE(2048U, 2048U, (twid_stride)) \
                                 : 0U)))))))))))

/* This macro matches the size of the TCB chain created by the 
** accel_fft_large_mag_sq_set_tcbs function, and declares the size
** in bytes of the memory region that must be allocated by the caller of
** that function and passed in via the __tcb parameter.
*/
#define ACCEL_FFT_LARGE_MAG_SQ_TCB_CHAIN_SIZE(npts, twid_stride) \
  ACCEL_FFT_LARGE_TCB_CHAIN_SIZE((npts), (twid_stride))

/* This macro matches the size of the TCB chain created by the 
** accel_fft_large_windowed_mag_sq_set_tcbs function, and declares the size
** in bytes of the memory region that must be allocated by the caller of
** that function and passed in via the __tcb parameter.
*/
#define ACCEL_FFT_LARGE_WINDOWED_MAG_SQ_TCB_CHAIN_SIZE(npts, twid_stride) \
  ACCEL_FFT_LARGE_WINDOWED_TCB_CHAIN_SIZE((npts), (twid_stride))
/*
** Variable size FFTs
*/

#if defined(__cplusplus)
extern "C" {
#endif

complex_float *accel_rfft_small(const float         __dm __input[],
                                complex_float       __dm __output[],
                                float                    __scale,
                                int                      __npts);

complex_float *accel_cfft_small(const complex_float __dm __input[],
                                complex_float       __dm __output[],
                                float                    __scale,
                                int                      __npts);

complex_float *accel_ifft_small(const complex_float __dm __input[],
                                complex_float       __dm __output[],
                                float                    __scale,
                                int                      __npts);

float *accel_irfft_small(const complex_float __dm __input[],
                         float               __dm __output[],
                         float                    __scale,
                         int                      __npts);

complex_float *accel_rfft_large(const float         __dm __input[],
                                complex_float       __dm __output[],
                                const complex_float __pm __twiddles[],
                                int                      __twiddle_stride,
                                float                    __scale,
                                int                      __npts);

complex_float *accel_cfft_large(const complex_float __dm __input[],
                                complex_float       __dm __output[],
                                const complex_float __pm __twiddles[],
                                int                      __twiddle_stride,
                                float                    __scale,
                                int                      __npts);

complex_float *accel_ifft_large(const complex_float __dm __input[],
                                complex_float       __dm __output[],
                                const complex_float __pm __twiddles[],
                                int                      __twiddle_stride,
                                float                    __scale,
                                int                      __npts);

float *accel_irfft_large(const complex_float __dm __input[],
                         float               __dm __output[],
                         complex_float       __dm __temp[],
                         const complex_float __pm __twiddles[],
                         int                      __twiddle_stride,
                         float                    __scale,
                         int                      __npts);
#if defined(__cplusplus)
}
#endif

#if defined(__ADSPARM__) || defined(__BYTE_ADDRESSING__)

#if defined(__cplusplus)
extern "C" {
#endif

ADI_FFT_HANDLE accel_cfft_large_pipe(ADI_FFT_DEVICE_MEMORY   *__h,
                                     float                    __scale_value,
                                     int                      __npts);

ADI_FFT_HANDLE accel_ifft_large_pipe(ADI_FFT_DEVICE_MEMORY   *__h,
                                     float                    __scale_value,
                                     int                      __npts);

ADI_FFT_HANDLE accel_irfft_large_pipe(ADI_FFT_DEVICE_MEMORY   *__h,
                                      float                    __scale_value,
                                      int                      __npts);

ADI_FFT_HANDLE accel_rfft_large_pipe(ADI_FFT_DEVICE_MEMORY   *__h,
                                     float                    __scale_value,
                                     int                      __npts);

void *
accel_fft_large_set_tcbs_strided(ADI_FFT_HANDLE           __h,
                                 void                    *__tcb,
                                 const void              *__input,
                                 int                      __input_stride,
                                 complex_float       __dm __output[],
                                 int                      __output_stride,
                                 complex_float       __dm __temp[],
                                 const complex_float __pm __twiddles[],
                                 int                      __twiddle_stride,
                                 int                      __npts,
                                 bool                     __is_real);


void *
accel_fft_large_mag_sq_set_tcbs_strided(ADI_FFT_HANDLE           __h,
                                        void                    *__tcb,
                                        const void              *__input,
                                        int                      __in_stride,
                                        float               __dm __output[],
                                        int                      __out_stride,
                                        complex_float       __dm __temp[],
                                        const complex_float __pm __twiddles[],
                                        int                      __twiddle_stride,
                                        int                      __npts,
                                        bool                     __is_real);
#if defined(__cplusplus)
}
#endif

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static void *
accel_cfft_large_set_tcbs(ADI_FFT_HANDLE           __h,
                          void                    *__tcb,
                          const complex_float __dm __input[],
                          complex_float       __dm __output[],
                          const complex_float __pm __twiddles[],
                          int                      __twiddle_stride,
                          int                      __npts) {
  return accel_fft_large_set_tcbs_strided(__h, __tcb, __input, 1, __output, 1, NULL,
                                          __twiddles, __twiddle_stride, __npts, false);
}

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static void *
accel_cfft_large_set_tcbs_strided(ADI_FFT_HANDLE           __h,
                                  void                    *__tcb,
                                  const complex_float __dm __input[],
                                  int                      __in_stride,
                                  complex_float       __dm __output[],
                                  int                      __out_stride,
                                  complex_float       __dm __temp[],
                                  const complex_float __pm __twiddles[],
                                  int                      __twiddle_stride,
                                  int                      __npts) {
  return accel_fft_large_set_tcbs_strided(__h, __tcb, __input, __in_stride,
                                          __output, __out_stride, __temp,
                                          __twiddles, __twiddle_stride, __npts, false);
}

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static void *
accel_ifft_large_set_tcbs(ADI_FFT_HANDLE           __h,
                          void                    *__tcb,
                          const complex_float __dm __input[],
                          complex_float       __dm __output[],
                          const complex_float __pm __twiddles[],
                          int                      __twiddle_stride,
                          int                      __npts) {
  return accel_fft_large_set_tcbs_strided(__h, __tcb, __input, 1, __output, 1, NULL,
                                          __twiddles, __twiddle_stride, __npts, false);
}

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static void *
accel_irfft_large_set_tcbs(ADI_FFT_HANDLE           __h,
                           void                    *__tcb,
                           const complex_float __dm __input[],
                           float               __dm __output[],
                           complex_float       __dm __temp[],
                           const complex_float __pm __twiddles[],
                           int                      __twiddle_stride,
                           int                      __npts) {
  /* This function needs same TCBs as the mag_sq function. */
  return accel_fft_large_mag_sq_set_tcbs_strided(__h, __tcb, __input, 1, __output, 1, __temp,
                                                 __twiddles, __twiddle_stride, __npts, false);
}

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static void *
accel_ifft_large_set_tcbs_strided(ADI_FFT_HANDLE           __h,
                                  void                    *__tcb,
                                  const complex_float __dm __input[],
                                  int                      __in_stride,
                                  complex_float       __dm __output[],
                                  int                      __out_stride,
                                  complex_float       __dm __temp[],
                                  const complex_float __pm __twiddles[],
                                  int                      __twiddle_stride,
                                  int                      __npts) {
  return accel_fft_large_set_tcbs_strided(__h, __tcb, __input, __in_stride,
                                          __output, __out_stride, __temp,
                                          __twiddles, __twiddle_stride, __npts, false);
}

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static void *
accel_irfft_large_set_tcbs_strided(ADI_FFT_HANDLE           __h,
                                   void                    *__tcb,
                                   const complex_float __dm __input[],
                                   int                      __in_stride,
                                   float               __dm __output[],
                                   int                      __out_stride,
                                   complex_float       __dm __temp[],
                                   const complex_float __pm __twiddles[],
                                   int                      __twiddle_stride,
                                   int                      __npts) {
  /* This function needs same TCBs as the mag_sq function. */
  return accel_fft_large_mag_sq_set_tcbs_strided(__h, __tcb, __input, __in_stride,
                                                 __output, __out_stride, __temp,
                                                 __twiddles, __twiddle_stride, __npts, false);
}

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static void *
accel_rfft_large_set_tcbs(ADI_FFT_HANDLE           __h,
                          void                    *__tcb,
                          const float         __dm __input[],
                          complex_float       __dm __output[],
                          const complex_float __pm __twiddles[],
                          int                      __twiddle_stride,
                          int                      __npts) {
  return accel_fft_large_set_tcbs_strided(__h, __tcb, __input, 1, __output, 1, NULL,
                                          __twiddles, __twiddle_stride, __npts, true);
}

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static void *
accel_rfft_large_set_tcbs_strided(ADI_FFT_HANDLE           __h,
                                  void                    *__tcb,
                                  const float         __dm __input[],
                                  int                      __in_stride,
                                  complex_float       __dm __output[],
                                  int                      __out_stride,
                                  complex_float       __dm __temp[],
                                  const complex_float __pm __twiddles[],
                                  int                      __twiddle_stride,
                                  int                      __npts) {
  return accel_fft_large_set_tcbs_strided(__h, __tcb, __input, __in_stride,
                                          __output, __out_stride, __temp,
                                          __twiddles, __twiddle_stride, __npts, true);
}

#endif /* __ADSPARM__ || __BYTE_ADDRESSING__ */

/*
** Variable size FFTs with magnitude squared output
*/

#if defined(__cplusplus)
extern "C" {
#endif

float *accel_rfft_small_mag_sq(const float         __dm __input[],
                               float               __dm __output[],
                               float                    __scale,
                               int                      __npts);

float *accel_cfft_small_mag_sq(const complex_float __dm __input[],
                               float               __dm __output[],
                               float                    __scale,
                               int                      __npts);

float *accel_rfft_large_mag_sq(const float         __dm __input[],
                               float               __dm __output[],
                               complex_float       __dm __temp[],
                               const complex_float __pm __twiddles[],
                               int                      __twiddle_stride,
                               float                    __scale,
                               int                      __npts);

float *accel_cfft_large_mag_sq(const complex_float __dm __input[],
                               float               __dm __output[],
                               complex_float       __dm __temp[],
                               const complex_float __pm __twiddles[],
                               int                      __twiddle_stride,
                               float                    __scale,
                               int                      __npts);
#if defined(__cplusplus)
}
#endif

#if defined(__ADSPARM__) || defined(__BYTE_ADDRESSING__)

#if defined(__cplusplus)
extern "C" {
#endif

ADI_FFT_HANDLE accel_cfft_large_mag_sq_pipe(ADI_FFT_DEVICE_MEMORY   *__h,
                                            float                    __scale_value,
                                            int                      __npts);

ADI_FFT_HANDLE accel_rfft_large_mag_sq_pipe(ADI_FFT_DEVICE_MEMORY   *__h,
                                            float                    __scale_value,
                                            int                      __npts);

#if defined(__cplusplus)
}
#endif

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static void *
accel_cfft_large_mag_sq_set_tcbs(ADI_FFT_HANDLE           __h,
                                 void                    *__tcb,
                                 const complex_float __dm __input[],
                                 float               __dm __output[],
                                 complex_float       __dm __temp[],
                                 const complex_float __pm __twiddles[],
                                 int                      __twiddle_stride,
                                 int                      __npts) {
  return accel_fft_large_mag_sq_set_tcbs_strided(__h, __tcb, __input, 1, __output, 1, __temp,
                                                 __twiddles, __twiddle_stride, __npts, false);
}

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static void *
accel_cfft_large_mag_sq_set_tcbs_strided(ADI_FFT_HANDLE           __h,
                                         void                    *__tcb,
                                         const complex_float __dm __input[],
                                         int                      __in_stride,
                                         float               __dm __output[],
                                         int                      __out_stride,
                                         complex_float       __dm __temp[],
                                         const complex_float __pm __twiddles[],
                                         int                      __twiddle_stride,
                                         int                      __npts) {
  return accel_fft_large_mag_sq_set_tcbs_strided(__h, __tcb, __input, __in_stride, 
                                                 __output, __out_stride, __temp,
                                                 __twiddles, __twiddle_stride, __npts, false);
}

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static void *
accel_rfft_large_mag_sq_set_tcbs(ADI_FFT_HANDLE           __h,
                                 void                    *__tcb,
                                 const float         __dm __input[],
                                 float               __dm __output[],
                                 complex_float       __dm __temp[],
                                 const complex_float __pm __twiddles[],
                                 int                      __twiddle_stride,
                                 int                      __npts) {
  return accel_fft_large_mag_sq_set_tcbs_strided(__h, __tcb, __input, 1, __output, 1, __temp,
                                                 __twiddles, __twiddle_stride, __npts, true);
}

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static void *
accel_rfft_large_mag_sq_set_tcbs_strided(ADI_FFT_HANDLE           __h,
                                         void                    *__tcb,
                                         const float         __dm __input[],
                                         int                      __in_stride,
                                         float               __dm __output[],
                                         int                      __out_stride,
                                         complex_float       __dm __temp[],
                                         const complex_float __pm __twiddles[],
                                         int                      __twiddle_stride,
                                         int                      __npts) {
  return accel_fft_large_mag_sq_set_tcbs_strided(__h, __tcb, __input, __in_stride,
                                                 __output, __out_stride, __temp,
                                                 __twiddles, __twiddle_stride, __npts, true);
}

#endif /* __ADSPARM__ || __BYTE_ADDRESSING__ */

/*
** Variable size FFTs with windowing
*/

#if defined(__cplusplus)
extern "C" {
#endif

complex_float *accel_rfft_small_windowed(const float         __dm __input[],
                                         complex_float       __dm __output[],
                                         const float         __dm __window[],
                                         float                    __scale,
                                         int                      __npts);

complex_float *accel_cfft_small_windowed(const complex_float __dm __input[],
                                         complex_float       __dm __output[],
                                         const float         __dm __window[],
                                         float                    __scale,
                                         int                      __npts);

complex_float *accel_ifft_small_windowed(const complex_float __dm __input[],
                                         complex_float       __dm __output[],
                                         const float         __dm __window[],
                                         float                    __scale,
                                         int                      __npts);

float *accel_irfft_small_windowed(const complex_float __dm __input[],
                                  float               __dm __output[],
                                  const float         __dm __window[],
                                  float                    __scale,
                                  int                      __npts);

complex_float *accel_rfft_large_windowed(const float         __dm __input[],
                                         complex_float       __dm __output[],
                                         const complex_float __pm __twiddles[],
                                         int                      __twiddle_stride,
                                         const float         __dm __window[],
                                         float                    __scale,
                                         int                      __npts);

complex_float *accel_cfft_large_windowed(const complex_float __dm __input[],
                                         complex_float       __dm __output[],
                                         const complex_float __pm __twiddles[],
                                         int                      __twiddle_stride,
                                         const float         __dm __window[],
                                         float                    __scale,
                                         int                      __npts);

complex_float *accel_ifft_large_windowed(const complex_float __dm __input[],
                                         complex_float       __dm __output[],
                                         const complex_float __pm __twiddles[],
                                         int                      __twiddle_stride,
                                         const float         __dm __window[],
                                         float                    __scale,
                                         int                      __npts);

float *accel_irfft_large_windowed(const complex_float __dm __input[],
                                  float               __dm __output[],
                                  complex_float       __dm __temp[],
                                  const complex_float __pm __twiddles[],
                                  int                      __twiddle_stride,
                                  const float         __dm __window[],
                                  float                    __scale,
                                  int                      __npts);
#if defined(__cplusplus)
}
#endif

#if defined(__ADSPARM__) || defined(__BYTE_ADDRESSING__)

#if defined(__cplusplus)
extern "C" {
#endif

ADI_FFT_HANDLE accel_cfft_large_windowed_pipe(ADI_FFT_DEVICE_MEMORY   *__h,
                                              float                    __scale_value,
                                              int                      __npts);

ADI_FFT_HANDLE accel_ifft_large_windowed_pipe(ADI_FFT_DEVICE_MEMORY   *__h,
                                              float                    __scale_value,
                                              int                      __npts);

ADI_FFT_HANDLE accel_irfft_large_windowed_pipe(ADI_FFT_DEVICE_MEMORY   *__h,
                                               float                    __scale_value,
                                               int                      __npts);

ADI_FFT_HANDLE accel_rfft_large_windowed_pipe(ADI_FFT_DEVICE_MEMORY   *__h,
                                              float                    __scale_value,
                                              int                      __npts);

void *
accel_fft_large_windowed_set_tcbs_strided(ADI_FFT_HANDLE           __h,
                                          void                    *__tcb,
                                          const void              *__input,
                                          int                      __in_stride,
                                          complex_float       __dm __output[],
                                          int                      __out_stride,
                                          complex_float       __dm __temp[],
                                          const complex_float __pm __twiddles[],
                                          int                      __twiddle_stride,
                                          const float         __dm __window[],
                                          int                      __npts,
                                          bool                     __is_real);

void *
accel_fft_large_windowed_mag_sq_set_tcbs_strided(ADI_FFT_HANDLE           __h,
                                                 void                    *__tcb,
                                                 const void              *__input,
                                                 int                      __in_stride,
                                                 float               __dm __output[],
                                                 int                      __out_stride,
                                                 complex_float       __dm __temp[],
                                                 const complex_float __pm __twiddles[],
                                                 int                      __twiddle_stride,
                                                 const float         __dm __window[],
                                                 int                      __npts,
                                                 bool                     __is_real);

#if defined(__cplusplus)
}
#endif

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static void *
accel_cfft_large_windowed_set_tcbs(ADI_FFT_HANDLE           __h,
                                   void                    *__tcb,
                                   const complex_float __dm __input[],
                                   complex_float       __dm __output[],
                                   const complex_float __pm __twiddles[],
                                   int                      __twiddle_stride,
                                   const float         __dm __window[],
                                   int                      __npts) {
  return accel_fft_large_windowed_set_tcbs_strided(__h, __tcb, __input, 1, __output, 1,
                                                   NULL, __twiddles, __twiddle_stride,
                                                   __window, __npts, false);
}

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static void *
accel_cfft_large_windowed_set_tcbs_strided(ADI_FFT_HANDLE           __h,
                                           void                    *__tcb,
                                           const complex_float __dm __input[],
                                           int                      __in_stride,
                                           complex_float       __dm __output[],
                                           int                      __out_stride,
                                           complex_float       __dm __temp[],
                                           const complex_float __pm __twiddles[],
                                           int                      __twiddle_stride,
                                           const float         __dm __window[],
                                           int                      __npts) {
  return accel_fft_large_windowed_set_tcbs_strided(__h, __tcb, __input, __in_stride,
                                                   __output, __out_stride, __temp,
                                                   __twiddles, __twiddle_stride,
                                                   __window, __npts, false);
}

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static void *
accel_ifft_large_windowed_set_tcbs(ADI_FFT_HANDLE           __h,
                                   void                    *__tcb,
                                   const complex_float __dm __input[],
                                   complex_float       __dm __output[],
                                   const complex_float __pm __twiddles[],
                                   int                      __twiddle_stride,
                                   const float         __dm __window[],
                                   int                      __npts) {
  return accel_fft_large_windowed_set_tcbs_strided(__h, __tcb, __input, 1, __output, 1,
                                                   NULL, __twiddles, __twiddle_stride,
                                                   __window, __npts, false);
}

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static void *
accel_irfft_large_windowed_set_tcbs(ADI_FFT_HANDLE           __h,
                                    void                    *__tcb,
                                    const complex_float __dm __input[],
                                    float               __dm __output[],
                                    complex_float       __dm __temp[],
                                    const complex_float __pm __twiddles[],
                                    int                      __twiddle_stride,
                                    const float         __dm __window[],
                                    int                      __npts) {
  /* This function needs same TCBs as the mag_sq function. */
  return accel_fft_large_windowed_mag_sq_set_tcbs_strided(__h, __tcb, __input, 1, __output, 1,
                                                          __temp, __twiddles, __twiddle_stride,
                                                          __window, __npts, false);
}

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static void *
accel_ifft_large_windowed_set_tcbs_strided(ADI_FFT_HANDLE           __h,
                                           void                    *__tcb,
                                           const complex_float __dm __input[],
                                           int                      __in_stride,
                                           complex_float       __dm __output[],
                                           int                      __out_stride,
                                           complex_float       __dm __temp[],
                                           const complex_float __pm __twiddles[],
                                           int                      __twiddle_stride,
                                           const float         __dm __window[],
                                           int                      __npts) {
  return accel_fft_large_windowed_set_tcbs_strided(__h, __tcb, __input, __in_stride, 
                                                   __output, __out_stride, __temp,
                                                   __twiddles, __twiddle_stride,
                                                   __window, __npts, false);
}

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static void *
accel_irfft_large_windowed_set_tcbs_strided(ADI_FFT_HANDLE           __h,
                                            void                    *__tcb,
                                            const complex_float __dm __input[],
                                            int                      __in_stride,
                                            float               __dm __output[],
                                            int                      __out_stride,
                                            complex_float       __dm __temp[],
                                            const complex_float __pm __twiddles[],
                                            int                      __twiddle_stride,
                                            const float         __dm __window[],
                                            int                      __npts) {
  /* This function needs same TCBs as the mag_sq function. */
  return accel_fft_large_windowed_mag_sq_set_tcbs_strided(__h, __tcb, __input, __in_stride,
                                                          __output, __out_stride, __temp,
                                                          __twiddles, __twiddle_stride,
                                                          __window, __npts, false);
}

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static void *
accel_rfft_large_windowed_set_tcbs(ADI_FFT_HANDLE           __h,
                                   void                    *__tcb,
                                   const float         __dm __input[],
                                   complex_float       __dm __output[],
                                   const complex_float __pm __twiddles[],
                                   int                      __twiddle_stride,
                                   const float         __dm __window[],
                                   int                      __npts) {
  return accel_fft_large_windowed_set_tcbs_strided(__h, __tcb, __input, 1, __output, 1,
                                                   NULL, __twiddles, __twiddle_stride,
                                                   __window, __npts, true);
}

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static void *
accel_rfft_large_windowed_set_tcbs_strided(ADI_FFT_HANDLE           __h,
                                           void                    *__tcb,
                                           const float         __dm __input[],
                                           int                      __in_stride,
                                           complex_float       __dm __output[],
                                           int                      __out_stride,
                                           complex_float       __dm __temp[],
                                           const complex_float __pm __twiddles[],
                                           int                      __twiddle_stride,
                                           const float         __dm __window[],
                                           int                      __npts) {
  return accel_fft_large_windowed_set_tcbs_strided(__h, __tcb, __input, __in_stride,
                                                   __output, __out_stride, __temp,
                                                   __twiddles, __twiddle_stride,
                                                   __window, __npts, true);
}

#endif /* __ADSPARM__ || __BYTE_ADDRESSING__ */

/*
** Variable size FFTs with windowing and magnitude squared output
*/

#if defined(__cplusplus)
extern "C" {
#endif

float *accel_rfft_small_windowed_mag_sq(const float         __dm __input[],
                                        float               __dm __output[],
                                        const float         __dm __window[],
                                        float                    __scale,
                                        int                      __npts);

float *accel_cfft_small_windowed_mag_sq(const complex_float __dm __input[],
                                        float               __dm __output[],
                                        const float         __dm __window[],
                                        float                    __scale,
                                        int                      __npts);

float *accel_rfft_large_windowed_mag_sq(const float         __dm __input[],
                                        float               __dm __output[],
                                        complex_float       __dm __temp[],
                                        const complex_float __pm __twiddles[],
                                        int                      __twiddle_stride,
                                        const float         __dm __window[],
                                        float                    __scale,
                                        int                      __npts);

float *accel_cfft_large_windowed_mag_sq(const complex_float __dm __input[],
                                        float               __dm __output[],
                                        complex_float       __dm __temp[],
                                        const complex_float __pm __twiddles[],
                                        int                      __twiddle_stride,
                                        const float         __dm __window[],
                                        float                    __scale,
                                        int                      __npts);
#if defined(__cplusplus)
}
#endif

#if defined(__ADSPARM__) || defined(__BYTE_ADDRESSING__)

#if defined(__cplusplus)
extern "C" {
#endif

ADI_FFT_HANDLE accel_cfft_large_windowed_mag_sq_pipe(ADI_FFT_DEVICE_MEMORY   *__h,
                                                     float                    __scale_value,
                                                     int                      __npts);

ADI_FFT_HANDLE accel_rfft_large_windowed_mag_sq_pipe(ADI_FFT_DEVICE_MEMORY   *__h,
                                                     float                    __scale_value,
                                                     int                      __npts);

#if defined(__cplusplus)
}
#endif

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static void *
accel_cfft_large_windowed_mag_sq_set_tcbs(ADI_FFT_HANDLE           __h,
                                          void                    *__tcb,
                                          const complex_float __dm __input[],
                                          float               __dm __output[],
                                          complex_float       __dm __temp[],
                                          const complex_float __pm __twiddles[],
                                          int                      __twiddle_stride,
                                          const float         __dm __window[],
                                          int                      __npts) {
  return accel_fft_large_windowed_mag_sq_set_tcbs_strided(__h, __tcb, __input, 1,
                                                          __output, 1, __temp,
                                                          __twiddles, __twiddle_stride,
                                                          __window, __npts, false);
}

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static void *
accel_cfft_large_windowed_mag_sq_set_tcbs_strided(ADI_FFT_HANDLE           __h,
                                                  void                    *__tcb,
                                                  const complex_float __dm __input[],
                                                  int                      __in_stride,
                                                  float               __dm __output[],
                                                  int                      __out_stride,
                                                  complex_float       __dm __temp[],
                                                  const complex_float __pm __twiddles[],
                                                  int                      __twiddle_stride,
                                                  const float         __dm __window[],
                                                  int                      __npts) {
  return accel_fft_large_windowed_mag_sq_set_tcbs_strided(__h, __tcb, __input, __in_stride,
                                                          __output, __out_stride, __temp,
                                                          __twiddles, __twiddle_stride,
                                                          __window, __npts, false);
}

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static void *
accel_rfft_large_windowed_mag_sq_set_tcbs(ADI_FFT_HANDLE           __h,
                                          void                    *__tcb,
                                          const float         __dm __input[],
                                          float               __dm __output[],
                                          complex_float       __dm __temp[],
                                          const complex_float __pm __twiddles[],
                                          int                      __twiddle_stride,
                                          const float         __dm __window[],
                                          int                      __npts) {
  return accel_fft_large_windowed_mag_sq_set_tcbs_strided(__h, __tcb, __input, 1,
                                                          __output, 1, __temp,
                                                          __twiddles, __twiddle_stride,
                                                          __window, __npts, true);
}

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static void *
accel_rfft_large_windowed_mag_sq_set_tcbs_strided(ADI_FFT_HANDLE           __h,
                                                  void                    *__tcb,
                                                  const float         __dm __input[],
                                                  int                      __in_stride,
                                                  float               __dm __output[],
                                                  int                      __out_stride,
                                                  complex_float       __dm __temp[],
                                                  const complex_float __pm __twiddles[],
                                                  int                      __twiddle_stride,
                                                  const float         __dm __window[],
                                                  int                      __npts) {
  return accel_fft_large_windowed_mag_sq_set_tcbs_strided(__h, __tcb, __input, __in_stride,
                                                          __output, __out_stride, __temp,
                                                          __twiddles, __twiddle_stride,
                                                          __window, __npts, true);
}

#endif /* __ADSPARM__ || __BYTE_ADDRESSING__ */

#if defined(__ADSPARM__) || defined(__BYTE_ADDRESSING__)

/*
** Continuous FFTs
*/

#if defined(__cplusplus)
extern "C" {
#endif

ADI_FFT_HANDLE accel_rfft_small_pipe(ADI_FFT_DEVICE_MEMORY *__h,
                                     float                  __scale,
                                     int                    __npts);

ADI_FFT_HANDLE accel_cfft_small_pipe(ADI_FFT_DEVICE_MEMORY *__h,
                                     float                  __scale,
                                     int                    __npts);

ADI_FFT_HANDLE accel_ifft_small_pipe(ADI_FFT_DEVICE_MEMORY *__h,
                                     float                  __scale,
                                     int                    __npts);

ADI_FFT_HANDLE accel_irfft_small_pipe(ADI_FFT_DEVICE_MEMORY *__h,
                                      float                  __scale,
                                      int                    __npts);

/*
** Continuous FFTs with magnitude squared output
*/

ADI_FFT_HANDLE accel_rfft_small_mag_sq_pipe(ADI_FFT_DEVICE_MEMORY *__h,
                                            float                  __scale,
                                            int                    __npts);

ADI_FFT_HANDLE accel_cfft_small_mag_sq_pipe(ADI_FFT_DEVICE_MEMORY *__h,
                                            float                  __scale,
                                            int                    __npts);

/*
** Continuous FFTs with windowing
*/

ADI_FFT_HANDLE accel_rfft_small_windowed_pipe(ADI_FFT_DEVICE_MEMORY   *__h,
                                              const float         __dm __window[],
                                              float                    __scale,
                                              int                      __npts);

ADI_FFT_HANDLE accel_cfft_small_windowed_pipe(ADI_FFT_DEVICE_MEMORY   *__h,
                                              const float         __dm __window[],
                                              float                    __scale,
                                              int                      __npts);

ADI_FFT_HANDLE accel_ifft_small_windowed_pipe(ADI_FFT_DEVICE_MEMORY   *__h,
                                              const float         __dm __window[],
                                              float                    __scale,
                                              int                      __npts);

ADI_FFT_HANDLE accel_irfft_small_windowed_pipe(ADI_FFT_DEVICE_MEMORY   *__h,
                                               const float         __dm __window[],
                                               float                    __scale,
                                               int                      __npts);

/*
** Continuous FFTs with windowing and magnitude squared output
*/

ADI_FFT_HANDLE accel_rfft_small_windowed_mag_sq_pipe(ADI_FFT_DEVICE_MEMORY   *__h,
                                                     const float         __dm __window[],
                                                     float                    __scale,
                                                     int                      __npts);

ADI_FFT_HANDLE accel_cfft_small_windowed_mag_sq_pipe(ADI_FFT_DEVICE_MEMORY   *__h,
                                                     const float         __dm __window[],
                                                     float                    __scale,
                                                     int                      __npts);

/*
** Continuous interleaved FFTs
*/

ADI_FFT_HANDLE accel_rfft_intleav_small(ADI_FFT_DEVICE_MEMORY *__h,
                                        float                  __rscale,
                                        float                  __iscale,
                                        int                    __npts);

ADI_FFT_HANDLE accel_cfft_intleav_small(ADI_FFT_DEVICE_MEMORY *__h,
                                        float                  __cscale,
                                        float                  __iscale,
                                        int                    __npts);

/*
** Continuous interleaved magnitude squared FFTs
*/

ADI_FFT_HANDLE accel_rfft_intleav_small_mag_sq(ADI_FFT_DEVICE_MEMORY *__h,
                                               float                  __rscale,
                                               float                  __iscale,
                                               int                    __npts);

ADI_FFT_HANDLE accel_cfft_intleav_small_mag_sq(ADI_FFT_DEVICE_MEMORY *__h,
                                               float                  __cscale,
                                               float                  __iscale,
                                               int                    __npts);

/*
** Continuous interleaved FFTs with windowing
*/

ADI_FFT_HANDLE accel_rfft_intleav_small_windowed(ADI_FFT_DEVICE_MEMORY   *__h,
                                                 const float         __dm __window[],
                                                 float                    __rscale,
                                                 float                    __iscale,
                                                 int                      __npts);

ADI_FFT_HANDLE accel_cfft_intleav_small_windowed(ADI_FFT_DEVICE_MEMORY   *__h,
                                                 const float         __dm __window[],
                                                 float                    __cscale,
                                                 float                    __iscale,
                                                 int                      __npts);

/*
** Continuous interleaved magnitude squared FFTs with windowing
*/

ADI_FFT_HANDLE accel_rfft_intleav_small_windowed_mag_sq(ADI_FFT_DEVICE_MEMORY   *__h,
                                                        const float         __dm __window[],
                                                        float                    __rscale,
                                                        float                    __iscale,
                                                        int                      __npts);

ADI_FFT_HANDLE accel_cfft_intleav_small_windowed_mag_sq(ADI_FFT_DEVICE_MEMORY   *__h,
                                                        const float         __dm __window[],
                                                        float                    __cscale,
                                                        float                    __iscale,
                                                        int                      __npts);

#if defined(__cplusplus)
}
#endif

#endif /* __BYTE_ADDRESSING__ */

#if defined(__ADSP21000__)
#pragma byte_addressed(pop)
#endif

#ifndef NO_FFT_WRAPPER_INLINES

/* Inline functions to facilitate replacing DSP library FFT functions
** with drop-in accelerator replacements.
*/

/* Real FFTs.
*/

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static complex_float *accel_rfft(const float         __dm __input[],
                                 complex_float       __dm __output[],
                                 const complex_float __pm __twiddles[],
                                 int                      __twiddle_stride,
                                 float                    __scale,
                                 int                      __npts) {
  /* the small and large functions will give an error if the size isn't
  ** supported.
  */
  complex_float *__result;
  if (__npts <= MAX_POINTS_FOR_SMALL_FFT) {
    /* the accelerator generates its own twiddles.
    */
    __result = accel_rfft_small(__input,__output,__scale,__npts);
  } else {
    __result = accel_rfft_large(__input,__output,__twiddles,__twiddle_stride,
                                __scale,__npts);
  }
  return __result;
}

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static complex_float *accel_rfft1024(float         __dm __input[],
                                     complex_float __dm __output[]) {
  return accel_rfft_small(__input, __output, 1.0f, 1024);
}

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static complex_float *accel_rfft2048(float         __dm __input[],
                                     complex_float __dm __output[]) {
  return accel_rfft_small(__input, __output, 1.0f, 2048);
}

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static complex_float *accel_rfft4096(float         __dm __input[],
                                     complex_float __dm __output[]) {
  return accel_rfft_large(__input, __output, accel_twiddles_4096,
                          1, 1.0f, 4096);
}

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static complex_float *accel_rfft8192(float         __dm __input[],
                                     complex_float __dm __output[]) {
  return accel_rfft_large(__input, __output, accel_twiddles_8192,
                          1, 1.0f, 8192);
}

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static complex_float *accel_rfft16384(float         __dm __input[],
                                      complex_float __dm __output[]) {
  return accel_rfft_large(__input, __output, accel_twiddles_16384,
                          1, 1.0f, 16384);
}

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static complex_float *accel_rfft32768(float         __dm __input[],
                                      complex_float __dm __output[]) {
  return accel_rfft_large(__input, __output, accel_twiddles_32768,
                          1, 1.0f, 32768);
}

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static complex_float *accel_rfft65536(float         __dm __input[],
                                      complex_float __dm __output[]) {
  return accel_rfft_large(__input, __output, accel_twiddles_65536,
                          1, 1.0f, 65536);
}

/* Complex FFTs
*/

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static complex_float *accel_cfft(const complex_float __dm __input[],
                                 complex_float       __dm __output[],
                                 const complex_float __pm __twiddles[],
                                 int                      __twiddle_stride,
                                 float                    __scale,
                                 int                      __npts) {
  /* the small and large functions will give an error if the size isn't
  ** supported.
  */
  complex_float *__result;
  if (__npts <= MAX_POINTS_FOR_SMALL_FFT) {
    /* the accelerator generates its own twiddles.
    */
    __result = accel_cfft_small(__input,__output,__scale,__npts);
  } else {
    __result = accel_cfft_large(__input,__output,__twiddles,__twiddle_stride,
                                __scale,__npts);
  }
  return __result;
}

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static complex_float *accel_cfft1024(complex_float __dm __input[],
                                     complex_float __dm __output[]) {
  return accel_cfft_small(__input, __output, 1.0f, 1024);
}

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static complex_float *accel_cfft2048(complex_float __dm __input[],
                                     complex_float __dm __output[]) {
  return accel_cfft_small(__input, __output, 1.0f, 2048);
}

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static complex_float *accel_cfft4096(complex_float __dm __input[],
                                     complex_float __dm __output[]) {
  return accel_cfft_large(__input, __output, accel_twiddles_4096,
                          1, 1.0f, 4096);
}

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static complex_float *accel_cfft8192(complex_float __dm __input[],
                                     complex_float __dm __output[]) {
  return accel_cfft_large(__input, __output, accel_twiddles_8192,
                          1, 1.0f, 8192);
}

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static complex_float *accel_cfft16384(complex_float __dm __input[],
                                      complex_float __dm __output[]) {
  return accel_cfft_large(__input, __output, accel_twiddles_16384,
                          1, 1.0f, 16384);
}

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static complex_float *accel_cfft32768(complex_float __dm __input[],
                                      complex_float __dm __output[]) {
  return accel_cfft_large(__input, __output, accel_twiddles_32768,
                          1, 1.0f, 32768);
}

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static complex_float *accel_cfft65536(complex_float __dm __input[],
                                      complex_float __dm __output[]) {
  return accel_cfft_large(__input, __output, accel_twiddles_65536,
                          1, 1.0f, 65536);
}

/* Inverse FFTs.
*/

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static complex_float *accel_ifft(const complex_float __dm __input[],
                                 complex_float       __dm __output[],
                                 const complex_float __pm __twiddles[],
                                 int                      __twiddle_stride,
                                 float                    __scale,
                                 int                      __npts) {
  /* the small and large functions will give an error if the size isn't
  ** supported.
  */
  complex_float *__result;
  if (__npts <= MAX_POINTS_FOR_SMALL_FFT) {
    /* the accelerator generates its own twiddles.
    */
    __result = accel_ifft_small(__input,__output,__scale,__npts);
  } else {
    __result = accel_ifft_large(__input,__output,__twiddles,__twiddle_stride,
                                __scale,__npts);
  }
  return __result;
}

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static complex_float *accel_ifft1024(complex_float __dm __input[],
                                     complex_float __dm __output[]) {
  return accel_ifft_small(__input, __output, 1.0f/1024.0f, 1024);
}

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static complex_float *accel_ifft2048(complex_float __dm __input[],
                                     complex_float __dm __output[]) {
  return accel_ifft_small(__input, __output, 1.0f/2048.0f, 2048);
}

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static complex_float *accel_ifft4096(complex_float __dm __input[],
                                     complex_float __dm __output[]) {
  return accel_ifft_large(__input, __output, accel_twiddles_4096,
                          1, 1.0f/4096.0f, 4096);
}

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static complex_float *accel_ifft8192(complex_float __dm __input[],
                                     complex_float __dm __output[]) {
  return accel_ifft_large(__input, __output, accel_twiddles_8192,
                          1, 1.0f/8192.0f, 8192);
}

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static complex_float *accel_ifft16384(complex_float __dm __input[],
                                      complex_float __dm __output[]) {
  return accel_ifft_large(__input, __output, accel_twiddles_16384,
                          1, 1.0f/16384.0f, 16384);
}

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static complex_float *accel_ifft32768(complex_float __dm __input[],
                                      complex_float __dm __output[]) {
  return accel_ifft_large(__input, __output, accel_twiddles_32768,
                          1, 1.0f/32768.0f, 32768);
}

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static complex_float *accel_ifft65536(complex_float __dm __input[],
                                      complex_float __dm __output[]) {
  return accel_ifft_large(__input, __output, accel_twiddles_65536,
                          1, 1.0f/65536.0f, 65536);
}

#endif /* NO_FFT_WRAPPER_INLINES */

/* Create twiddles for large FFT in accelerator native format.
*/
#if defined(__cplusplus)
extern "C" {
#endif

complex_float __pm *accel_twidfft(complex_float __pm      *__twids,
                                  int                      __npts);

#if defined(__cplusplus)
}
#endif

#if defined(__ADSPARM__) || defined(__BYTE_ADDRESSING__)

/* This macro matches the size of the TCB chain created by the 
** accel_fft_fir_multi_channel_set_tcbs function, and declares the size in
** bytes of the memory region that must be allocated by the caller of
** that function and passed in via the __tcb parameter.
** - 4 TX descriptors per channel and 2 RX descriptors, each of 5 words
*/
#define ACCEL_FFT_FIR_MULTI_CHANNEL_TCB_CHAIN_SIZE(block_size, ntaps, nchan) \
  (4u * ((4u * (nchan)) + 2u) * 5u)

/* This macro matches the size of the TCB chain created by the 
** accel_fft_fir_set_tcbs function, and declares the size in
** bytes of the memory region that must be allocated by the caller of
** that function and passed in via the __tcb parameter.
*/
#define ACCEL_FFT_FIR_TCB_CHAIN_SIZE(block_size, ntaps) \
  ACCEL_FFT_FIR_MULTI_CHANNEL_TCB_CHAIN_SIZE((block_size), (ntaps), 1u)

#if defined(__cplusplus)
extern "C" {
#endif

ADI_FFT_HANDLE accel_fft_fir_pipe(ADI_FFT_DEVICE_MEMORY   *__h,
                                  int                      __block_size,
                                  int                      __ntaps);

void *
accel_fft_fir_multi_channel_set_tcbs(ADI_FFT_HANDLE           __h,
                                     void                    *__tcb,
                                     const float         __dm __input[],
                                     float               __dm __output[],
                                     const complex_float __dm __coeff[],
                                     int                      __block_size,
                                     int                      __ntaps,
                                     int                      __nchannels);

#pragma inline
#pragma always_inline
#pragma source_position_from_call_site
static void *
accel_fft_fir_set_tcbs(ADI_FFT_HANDLE           __h,
                       void                    *__tcb,
                       const float         __dm __input[],
                       float               __dm __output[],
                       const complex_float __dm __coeff[],
                       int                      __block_size,
                       int                      __ntaps) {
  return accel_fft_fir_multi_channel_set_tcbs(__h, __tcb, __input, __output,
                                              __coeff, __block_size, __ntaps,
                                              /*nchannels*/1);
}

#if defined(__cplusplus)
}
#endif

#endif

#if defined(__ADSP21000__)

#if defined(_MISRA_2004_RULES) || defined(_MISRA_2012_RULES)
#pragma diag(pop)
#endif /* _MISRA_2004_RULES || _MISRA_2012_RULES */

#endif /* defined(__ADSP21000__) */

#endif /* __ADI_FFT_WRAPPER_H__ */
