/*********************************************************************************

Copyright(c) 2013-2019 Analog Devices, Inc. All Rights Reserved.

This software is proprietary.  By using this software you agree
to the terms of the associated Analog Devices License Agreement.

*********************************************************************************/
/*!
* @file      adi_hpfp_fft.h
*
* @brief     Driver High Performance Floating Point (HPFP) FFT.
*
* @details
*            This is the primary HPFP FFT API header file for Application writers.
*/
#ifndef _ADI_HPFP_FFT_H_
#define _ADI_HPFP_FFT_H_

/** @addtogroup FFT_Driver FFT Accelerator Driver
 *  @{
 */

#if defined(__ADSPSC589_FAMILY__)

#include <stdint.h>
#include <stdbool.h>
#include <sys/platform.h>
#include <services/int/adi_int.h>
#include <services/dma/adi_dma.h>
#include <adi_osal.h>

#ifdef _MISRA_RULES
#pragma diag(push)
#pragma diag(suppress:misra_rule_5_1:"Identifiers shall not rely on the significance of more than 31 characters")
#endif /* _MISRA_RULES */

/*! FFT device handle. */
typedef void*   ADI_FFT_HANDLE;

/*! Amount of memory in bytes required for FFT driver operation. */
#if defined(__ADSPSC589_FAMILY__)
#if defined(__ADSPCORTEXA5__)
#define ADI_FFT_MEMORY				((472u) + (2u*(ADI_OSAL_MAX_SEM_SIZE_CHAR)))
#else
#define ADI_FFT_MEMORY				((488u) + (2u*(ADI_OSAL_MAX_SEM_SIZE_CHAR)))
#endif
#endif

/*! Enumeration of different error codes returned by FFT driver. */
typedef enum
{
    /*! The operation completed successfully. */
    ADI_FFT_SUCCESS,
    
     /*! The operation failed. */
    ADI_FFT_FAILURE,

    /*! The given FFT handle is invalid. */
    ADI_FFT_INVALID_HANDLE,

    /*! The given pointer is pointing to NULL. */
    ADI_FFT_NULL_POINTER,

    /*! The given device instance number is outside
        the number of instances supported by the processor. */
    ADI_FFT_INVALID_DEVICE_NUM,

    /*! The given instance is already opened and in use. */
    ADI_FFT_DEVICE_IN_USE,

    /*! Failed to register the interrupt handler with interrupt service. */
    ADI_FFT_INT_FAILED,

    /*! DMA related failure occurred. */
    ADI_FFT_DMA_FAILED,

    /*! Semaphore operation failed */
    ADI_FFT_SEM_FAILED,

    /*! Failed to submit the given buffer. */
    ADI_FFT_SUBMIT_FAILED,

    /*! The given operation is not permitted in the current state of the device. */
    ADI_FFT_INVALID_OPERATION,

    /*! If any of the given parameters are invalid. */
    ADI_FFT_INVALID_PARAMETER,

    /*! If the given buffer size invalid. */
    ADI_FFT_INVALID_BUF_SIZE,

    /*! If no buffers are submitted when enabling data flow. */
    ADI_FFT_BUFFERS_NOT_SUBMITTED,

    /*! Hardware error(s) is detected. The hardware error can be known by calling
     *  the API #adi_fft_GetHWErrorStatus
     */
    ADI_FFT_HW_ERROR_DETECTED

} ADI_FFT_RESULT;


/*! Enumeration of different hardware error codes. */
typedef enum
{
    /*! No hardware error occurred. */
    ADI_FFT_HW_ERR_NONE              = 0x0,

    /*! The given input has NAN data or invalid math operation. */
    ADI_FFT_HW_ERR_INVALID_INPUT      = 0x1,
    
    /*! Math result after rounding has a magnitude less than the
        minimum normalized number, and it is not an exact zero. */
    ADI_FFT_HW_ERR_ROUNDING_UNDERFLOW = 0x2,
    
    /*! Math result after rounding has a magnitude greater than the
       maximum normalized number. */
    ADI_FFT_HW_ERR_ROUNDING_OVERFLOW  = 0x4,
    
    /*! An invalid instruction was executed. */ 
    ADI_FFT_HW_ERR_INVALID_INSTRUCTION = 0x8,

    /*! DMA error occurred. */
    ADI_FFT_HW_ERR_DMA                 = 0x7FFFFFFF
/*    ADI_FFT_HW_ERR_DMA                 = 0x80000000 */
    
} ADI_FFT_HW_ERR;

/*! Enumeration of DMA transfer size supported by the FFT device */
typedef enum
{
    /* FIX-ME : Check how what is the maximum width supported by FFT DMA */
    ADI_FFT_DMA_TRANSFER_4BYTES   = ADI_DMA_MSIZE_4BYTES,         /*! 8 bytes transfer. */
    ADI_FFT_DMA_TRANSFER_8BYTES   = ADI_DMA_MSIZE_8BYTES,         /*! 8 bytes transfer. */
    ADI_FFT_DMA_TRANSFER_16BYTES  = ADI_DMA_MSIZE_16BYTES,        /*! 16 bytes transfer. */
    ADI_FFT_DMA_TRANSFER_32BYTES  = ADI_DMA_MSIZE_32BYTES         /*! 32 bytes transfer. */

}ADI_FFT_DMA_TRANSFER_SIZE;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/***************************************
    Open/Close APIs
****************************************/    

/* 
** Opens FFT device.
*/
ADI_FFT_RESULT adi_fft_Open (    
    uint32_t                    nDeviceNum,
    void  * const               pMemory,
    ADI_FFT_HANDLE              *phDevice
);

/*
** Closes FFT device.
*/
ADI_FFT_RESULT adi_fft_Close (    
    ADI_FFT_HANDLE const        hDevice
);


/*************************************
    Configuration APIs 
*************************************/

/*
** Enables or disables FFT device.
*/
ADI_FFT_RESULT adi_fft_Enable (
    ADI_FFT_HANDLE const        hDevice,
    bool                        bEnable
);

/*
** Resets the FFT device.
*/
ADI_FFT_RESULT adi_fft_Reset ( 
    ADI_FFT_HANDLE const        hDevice
);


/*
** Sets the DMA transfer size for Tx.
*/
ADI_FFT_RESULT adi_fft_SetTxDmaTransferSize(
    ADI_FFT_HANDLE const        hDevice,
    ADI_FFT_DMA_TRANSFER_SIZE   eDMASize
);

/*
** Sets the DMA transfer size for Tx.
*/
ADI_FFT_RESULT adi_fft_SetRxDmaTransferSize(
    ADI_FFT_HANDLE const        hDevice,
    ADI_FFT_DMA_TRANSFER_SIZE   eDMASize
);


/************************************
    Data transaction APIs 
*************************************/

/******** Blocking data transaction APIs ************/

/* Reads the data into the given buffer. */
ADI_FFT_RESULT adi_fft_Read (
    ADI_FFT_HANDLE const        hDevice,
    void *                      pBuffer,
    uint32_t                    nSamples
);

/* Writes the given data. */
ADI_FFT_RESULT adi_fft_Write (
    ADI_FFT_HANDLE const        hDevice,
    void *                      pBuffer,
    uint32_t                    nSamples                  
);

/* Reads the data into the given buffer. */
ADI_FFT_RESULT adi_fft_ReadFrame (
    ADI_FFT_HANDLE const        hDevice,
    void *                      pFrame,
    uint32_t                    nSamplesPerLine,
    int32_t                     nXStride,
    uint32_t                    nNumLines,
    int32_t                     nYStride
);

/* Writes the given data. */
ADI_FFT_RESULT adi_fft_WriteFrame (
    ADI_FFT_HANDLE const        hDevice,
    void *                      pFrame,
    uint32_t                    nSamplesPerLine,
    int32_t                     nXStride,
    uint32_t                    nNumLines,
    int32_t                     nYStride
);

/******** Non-Blocking Buffer submission APIs ************/

/* Enable/Disable Tx data flow. */
ADI_FFT_RESULT adi_fft_EnableTx (
    ADI_FFT_HANDLE const        hDevice,
    bool                        bEnable
);

/* Enable/Disable Rx data flow. */
ADI_FFT_RESULT adi_fft_EnableRx (
    ADI_FFT_HANDLE const        hDevice,
    bool                        bEnable
);

/* Submits the buffer to be written to the FFT. */
ADI_FFT_RESULT adi_fft_SubmitTxBuffer (
    ADI_FFT_HANDLE const        hDevice,
    void *                      pBuffer,
    uint32_t                    nSamples
);

/* Submits the frame to be written to the FFT. */
ADI_FFT_RESULT adi_fft_SubmitTxFrame (
    ADI_FFT_HANDLE const        hDevice,
    void *                      pFrame,
    uint32_t                    nSamplesPerLine,
    int32_t                     nXStride,
    uint32_t                    nNumLines,
    int32_t                     nYStride
);

/* Submits the buffer to be read from FFT. */
ADI_FFT_RESULT adi_fft_SubmitRxBuffer (
    ADI_FFT_HANDLE const        hDevice,
    void *                      pBuffer,
    uint32_t                    nSamples
);

/* Submits the frame to be read from FFT. */
ADI_FFT_RESULT adi_fft_SubmitRxFrame (
    ADI_FFT_HANDLE const        hDevice,
    void *                      pFrame,
    uint32_t                    nSamplesPerLine,
    int32_t                     nXStride,
    uint32_t                    nNumLines,
    int32_t                     nYStride
);

/* Checks if a processed Rx Buffer/frame is available. */
ADI_FFT_RESULT adi_fft_IsRxBufferAvailable (
    ADI_FFT_HANDLE const        hDevice,
    bool                       *pbAvailable
);

/* Checks if a processed Tx Buffer/frame is available. */
ADI_FFT_RESULT adi_fft_IsTxBufferAvailable (
    ADI_FFT_HANDLE const        hDevice,
    bool                       *pbAvailable
);


/*** Blocking buffer/frame retrieval APIs ****/

/* Gets back the processed Rx frame/buffer. */
ADI_FFT_RESULT adi_fft_GetRxBuffer (
    ADI_FFT_HANDLE const        hDevice,
    void **                     ppBuffer
);

/* Gets back the processed Tx frame/buffer. */
ADI_FFT_RESULT adi_fft_GetTxBuffer (
    ADI_FFT_HANDLE const        hDevice,
    void **                     ppBuffer
);

/**** Support for loading DMA descriptor chains ****/

/* Sets the TX DMA channel ready to load a 3 word DMA descriptor */
ADI_FFT_RESULT adi_fft_SubmitTxDescriptorChain3 (
    ADI_FFT_HANDLE              hDevice,
    ADI_DMA_TCB3               *pTCB,
    uint32_t                    nSamplesPerLine,
    int32_t                     nXStride,
    uint32_t                    nNumLines,
    int32_t                     nYStride,
    bool                        bTrigToStart
);

/* Sets the RX DMA channel ready to load a 3 word DMA descriptor */
ADI_FFT_RESULT adi_fft_SubmitRxDescriptorChain3 (
    ADI_FFT_HANDLE              hDevice,
    ADI_DMA_TCB3               *pTCB,
    uint32_t                    nSamplesPerLine,
    int32_t                     nXStride,
    uint32_t                    nNumLines,
    int32_t                     nYStride,
    bool                        bTrigToStart
);

/* Sets the TX DMA channel ready to load a 4 word DMA descriptor */
ADI_FFT_RESULT adi_fft_SubmitTxDescriptorChain4 (
    ADI_FFT_HANDLE              hDevice,
    ADI_DMA_TCB4               *pTCB,
    int32_t                     nXStride,
    uint32_t                    nNumLines,
    int32_t                     nYStride,
    bool                        bTrigToStart
);

/* Sets the RX DMA channel ready to load a 4 word DMA descriptor */
ADI_FFT_RESULT adi_fft_SubmitRxDescriptorChain4 (
    ADI_FFT_HANDLE              hDevice,
    ADI_DMA_TCB4               *pTCB,
    int32_t                     nXStride,
    uint32_t                    nNumLines,
    int32_t                     nYStride,
    bool                        bTrigToStart
);

/* Sets the TX DMA channel ready to load a 5 word DMA descriptor */
ADI_FFT_RESULT adi_fft_SubmitTxDescriptorChain5 (
    ADI_FFT_HANDLE              hDevice,
    ADI_DMA_TCB5               *pTCB,
    uint32_t                    nNumLines,
    int32_t                     nYStride,
    bool                        bTrigToStart
);

/* Sets the RX DMA channel ready to load a 5 word DMA descriptor */
ADI_FFT_RESULT adi_fft_SubmitRxDescriptorChain5 (
    ADI_FFT_HANDLE              hDevice,
    ADI_DMA_TCB5               *pTCB,
    uint32_t                    nNumLines,
    int32_t                     nYStride,
    bool                        bTrigToStart
);

/* Sets the TX DMA channel ready to load a 6 word DMA descriptor */
ADI_FFT_RESULT adi_fft_SubmitTxDescriptorChain6 (
    ADI_FFT_HANDLE              hDevice,
    ADI_DMA_TCB6               *pTCB,
    int32_t                     nYStride,
    bool                        bTrigToStart
);

/* Sets the RX DMA channel ready to load a 6 word DMA descriptor */
ADI_FFT_RESULT adi_fft_SubmitRxDescriptorChain6 (
    ADI_FFT_HANDLE              hDevice,
    ADI_DMA_TCB6               *pTCB,
    int32_t                     nYStride,
    bool                        bTrigToStart
);

/* Sets the TX DMA channel ready to load a 7 word DMA descriptor */
ADI_FFT_RESULT adi_fft_SubmitTxDescriptorChain7 (
    ADI_FFT_HANDLE              hDevice,
    ADI_DMA_TCB7               *pTCB,
    bool                        bTrigToStart
);

/* Sets the RX DMA channel ready to load a 7 word DMA descriptor */
ADI_FFT_RESULT adi_fft_SubmitRxDescriptorChain7 (
    ADI_FFT_HANDLE              hDevice,
    ADI_DMA_TCB7               *pTCB,
    bool                        bTrigToStart
);

/* Sets a callback on the TX DMA channel for use with a DMA descriptor chain */
ADI_FFT_RESULT adi_fft_RegisterTxCallback(
    ADI_FFT_HANDLE              hDevice,
    ADI_CALLBACK                pfCallback,
    void                        *pCBParam
);

/* Sets a callback on the RX DMA channel for use with a DMA descriptor chain */
ADI_FFT_RESULT adi_fft_RegisterRxCallback(
    ADI_FFT_HANDLE              hDevice,
    ADI_CALLBACK                pfCallback,
    void                        *pCBParam
);

/* Sets the Trigger Master to use for the TX DMA Slave */
ADI_FFT_RESULT adi_fft_RegisterTxTriggerMaster(
    ADI_FFT_HANDLE              hDevice,
    int32_t                     nTriggerMaster
);

/* Sets the Trigger Master to use for the RX DMA Slave */
ADI_FFT_RESULT adi_fft_RegisterRxTriggerMaster(
    ADI_FFT_HANDLE              hDevice,
    int32_t                     nTriggerMaster
);

/* Manually trigger the TX DMA channel's registered trigger master. */
ADI_FFT_RESULT adi_fft_TriggerTx(
    ADI_FFT_HANDLE              hDevice
);

/* Manually trigger the RX DMA channel's registered trigger master. */
ADI_FFT_RESULT adi_fft_TriggerRx(
    ADI_FFT_HANDLE              hDevice
);

/* Wait until an interrupt has been raised by the TX DMA channel.
 * Should not be used if a callback has been registered on the TX channel.
 */
ADI_FFT_RESULT adi_fft_SynchronizeTx(
    ADI_FFT_HANDLE              hDevice
);

/* Wait until an interrupt has been raised by the RX DMA channel.
 * Should not be used if a callback has been registered on the RX channel.
 */
ADI_FFT_RESULT adi_fft_SynchronizeRx(
    ADI_FFT_HANDLE              hDevice
);

/* Check if an interrupt has been raised by the TX DMA channel.
 * Should not be used if a callback has been registered on the RX channel.
 */
ADI_FFT_RESULT adi_fft_IsSynchronizeTxReady(
    ADI_FFT_HANDLE              hDevice,
    bool                       *pReady
);

/* Check if an interrupt has been raised by the RX DMA channel.
 * Should not be used if a callback has been registered on the RX channel.
 */
ADI_FFT_RESULT adi_fft_IsSynchronizeRxReady(
    ADI_FFT_HANDLE              hDevice,
    bool                       *pReady
);


/********** HW error Query API ****************/

/* Gets back the detected hardware errors */
ADI_FFT_RESULT adi_fft_GetHWErrorStatus (
    ADI_FFT_HANDLE const  hDevice,
    uint32_t *const       pnHWError
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#ifdef _MISRA_RULES
#pragma diag(pop)
#endif /* _MISRA_RULES */


/**
 * @}
 */


#else
#error "This processor is not supported"
#endif 
#endif /* _ADI_HPFP_FFT_H_ */
