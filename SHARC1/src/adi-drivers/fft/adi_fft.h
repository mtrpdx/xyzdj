/*********************************************************************************

Copyright(c) 2013-2022 Analog Devices, Inc. All Rights Reserved.

This software is proprietary.  By using this software you agree
to the terms of the associated Analog Devices License Agreement.

*********************************************************************************/
/** @addtogroup FFT_Driver FFT Accelerator Driver
 *  @{
 */
/*!
* @file      adi_fft.h
*
* @brief     FFT driver global API header file
*
* @details
*            This is the primary global FFT API header file for Application writers
*/
#ifndef _ADI_FFT_H_
#define _ADI_FFT_H_

#if !defined(__BYTE_ADDRESSING__) && defined(__ADSPSHARC__)
	#error "Only Byte addressing mode is supported"
#endif 

#if defined(__ADSPSC589_FAMILY__)
#include "adi_hpfp_fft.h"
#else
#error "This processor is not supported"
#endif

#endif /* _ADI_FFT_H_ */
/*@}*/
