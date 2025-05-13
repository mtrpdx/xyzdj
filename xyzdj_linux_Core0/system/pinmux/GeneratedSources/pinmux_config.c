/*
 **
 ** Source file generated on May 13, 2025 at 12:52:58.	
 **
 ** Copyright (C) 2011-2025 Analog Devices Inc., All Rights Reserved.
 **
 ** This file is generated automatically based upon the options selected in 
 ** the Pin Multiplexing configuration editor. Changes to the Pin Multiplexing
 ** configuration should be made by changing the appropriate options rather
 ** than editing this file.
 **
 ** Selected Peripherals
 ** --------------------
 ** MSI0 (CD, CLK, CMD, D0, D1, D2, D3, D4, D5, D6, D7, INT)
 **
 ** GPIO (unavailable)
 ** ------------------
 ** PF02, PF03, PF04, PF05, PF06, PF07, PF08, PF09, PF10, PF11, PF12, PF13
 */

#include <sys/platform.h>
#include <stdint.h>

#define MSI0_CD_PORTF_MUX  ((uint32_t) ((uint32_t) 0<<24))
#define MSI0_CLK_PORTF_MUX  ((uint32_t) ((uint32_t) 0<<22))
#define MSI0_CMD_PORTF_MUX  ((uint32_t) ((uint32_t) 0<<20))
#define MSI0_D0_PORTF_MUX  ((uint16_t) ((uint16_t) 0<<4))
#define MSI0_D1_PORTF_MUX  ((uint16_t) ((uint16_t) 0<<6))
#define MSI0_D2_PORTF_MUX  ((uint16_t) ((uint16_t) 0<<8))
#define MSI0_D3_PORTF_MUX  ((uint16_t) ((uint16_t) 0<<10))
#define MSI0_D4_PORTF_MUX  ((uint16_t) ((uint16_t) 0<<12))
#define MSI0_D5_PORTF_MUX  ((uint16_t) ((uint16_t) 0<<14))
#define MSI0_D6_PORTF_MUX  ((uint32_t) ((uint32_t) 0<<16))
#define MSI0_D7_PORTF_MUX  ((uint32_t) ((uint32_t) 0<<18))
#define MSI0_INT_PORTF_MUX  ((uint32_t) ((uint32_t) 3<<26))

#define MSI0_CD_PORTF_FER  ((uint32_t) ((uint32_t) 1<<12))
#define MSI0_CLK_PORTF_FER  ((uint32_t) ((uint32_t) 1<<11))
#define MSI0_CMD_PORTF_FER  ((uint32_t) ((uint32_t) 1<<10))
#define MSI0_D0_PORTF_FER  ((uint16_t) ((uint16_t) 1<<2))
#define MSI0_D1_PORTF_FER  ((uint16_t) ((uint16_t) 1<<3))
#define MSI0_D2_PORTF_FER  ((uint16_t) ((uint16_t) 1<<4))
#define MSI0_D3_PORTF_FER  ((uint16_t) ((uint16_t) 1<<5))
#define MSI0_D4_PORTF_FER  ((uint16_t) ((uint16_t) 1<<6))
#define MSI0_D5_PORTF_FER  ((uint16_t) ((uint16_t) 1<<7))
#define MSI0_D6_PORTF_FER  ((uint32_t) ((uint32_t) 1<<8))
#define MSI0_D7_PORTF_FER  ((uint32_t) ((uint32_t) 1<<9))
#define MSI0_INT_PORTF_FER  ((uint32_t) ((uint32_t) 1<<13))

int32_t adi_initpinmux(void);

/*
 * Initialize the Port Control MUX and FER Registers
 */
int32_t adi_initpinmux(void) {
    /* PORTx_MUX registers */
    *pREG_PORTF_MUX = MSI0_CD_PORTF_MUX | MSI0_CLK_PORTF_MUX
     | MSI0_CMD_PORTF_MUX | MSI0_D0_PORTF_MUX | MSI0_D1_PORTF_MUX
     | MSI0_D2_PORTF_MUX | MSI0_D3_PORTF_MUX | MSI0_D4_PORTF_MUX
     | MSI0_D5_PORTF_MUX | MSI0_D6_PORTF_MUX | MSI0_D7_PORTF_MUX
     | MSI0_INT_PORTF_MUX;

    /* PORTx_FER registers */
    *pREG_PORTF_FER = MSI0_CD_PORTF_FER | MSI0_CLK_PORTF_FER
     | MSI0_CMD_PORTF_FER | MSI0_D0_PORTF_FER | MSI0_D1_PORTF_FER
     | MSI0_D2_PORTF_FER | MSI0_D3_PORTF_FER | MSI0_D4_PORTF_FER
     | MSI0_D5_PORTF_FER | MSI0_D6_PORTF_FER | MSI0_D7_PORTF_FER
     | MSI0_INT_PORTF_FER;
    return 0;
}

