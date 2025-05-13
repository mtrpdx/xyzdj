/*****************************************************************************
 * xyzdj_linux_Core0.c
 *****************************************************************************/

/* Standard includes. */
#include <stdlib.h>

/* CCES includes */
#include <sys/platform.h>
#include <sys/adi_core.h>

/* Kernel includes. */
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

/* Simple service includes */
#include "syslog.h"

/* Project includes */
#include "adi_initialize.h"
#include "xyzdj_linux_Core0.h"

/* Application includes. */
#include "FreeRTOSUserApplication.h"


int main()
{
	/**
	 * Initialize managed drivers and/or services that have been added to 
	 * the project.
	 * @return zero on success 
	 */
	adi_initComponents();
	
	/**
	 * The default startup code does not include any functionality to allow
	 * core 0 to enable core 1 and core 2. A convenient way to enable
	 * core 1 and core 2 is to use the adi_core_enable function. 
	 */
	adi_core_enable(ADI_CORE_SHARC0);
	adi_core_enable(ADI_CORE_SHARC1);

	/* Begin adding your custom code here */

	/* When using FreeRTOS calls to SSL/DD cannot be made until the scheduler is active.
	This includes adi_initComponents().
	To ensure that devices are initialized before tasks are started place your code
	in vApplicationDaemonTaskStartupHook() located in FreeRTOSHooks.c */

	/* Create a FreeRTOS task to run when the scheduler starts if userStartupTask is enabled in system.svc */
#if INCLUDE_userStartupTask == 1
	BaseType_t xReturned;
	TaskHandle_t startupTaskHandle;
	xReturned = xTaskCreate( userStartupTask,
				"Startup Task",
				configMINIMAL_STACK_SIZE,
				NULL,
				tskIDLE_PRIORITY + 2,
				&startupTaskHandle );

	if( xReturned != pdPASS )
	{
		abort();
	}
#endif /* INCLUDE_userStartupTask == 1 */

	/* Start the scheduler. */
	vTaskStartScheduler();

	return 0;
}

