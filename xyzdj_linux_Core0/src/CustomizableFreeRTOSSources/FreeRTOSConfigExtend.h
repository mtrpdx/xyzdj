/*
 * FreeRTOSCOnfigExtend.h : Once generated extension to FreeRTOSConfig.h
 *
 * File generated on May 13, 2025 at 13:08:48 
 * by the FreeRTOS Add-In for CrossCore Embedded Studio.
 * 
 * This file will only be generated when adding the FreeRTOS Add-In to your project, 
 * or if the project is opened and the file is not found.
 * It is otherwise safe to modify this file without risk of the changes being lost. 
 */

/** Extended FreeRTOS configuration **/

#ifndef _FREERTOSCONFIGEXTEND_H_
#define _FREERTOSCONFIGEXTEND_H_

#ifdef __ADSPARM__
#include <sys/platform.h>
#endif

/**No UI Option**/
#ifndef configUNIQUE_INTERRUPT_PRIORITIES
#define configUNIQUE_INTERRUPT_PRIORITIES                         32 
#endif
#ifndef configINTERRUPT_CONTROLLER_BASE_ADDRESS
#define configINTERRUPT_CONTROLLER_BASE_ADDRESS                   (REG_GICDST0_EN) 
#endif
#ifndef configINTERRUPT_CONTROLLER_CPU_INTERFACE_OFFSET
#define configINTERRUPT_CONTROLLER_CPU_INTERFACE_OFFSET           (REG_GICCPU1_CTL - REG_GICDST0_EN) 
#endif
#ifndef configUSE_16_BIT_TICKS
#define configUSE_16_BIT_TICKS                                    0 
#endif
#ifndef configUSE_NEWLIB_REENTRANT
#define configUSE_NEWLIB_REENTRANT                                0 
#endif


#endif /* _FREERTOSCONFIGEXTEND_H_ */ 

