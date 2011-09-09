/*
 * timebase.c
 *
 *  Created on: 02.10.2009
 *      Author: mthomas
 */

#include <stdint.h>
#include "stm32f10x.h"
#include "stm32f10x_conf.h"   /* RAMFUNC */
#include "platform_config.h"  /* LEDs */
#include "diskio.h"           /* disk_timerproc */

static volatile uint32_t ms_tick;

/**
  * @brief  This function handles SysTick Handler, operations done every 1ms
  * @param  None
  * @retval : None
  *  overrides weak SysTick_Handler in startup_stm32*.c
  *  When a RAMFUNC every function called from inside the ISR must be
  *  reachable. This can be achieved by using compiler-option -mlong-calls
  *  which globally enables long-calls. Here this option has not been used
  *  instead the unreachable functions GPIO_Set/ResetBits have been replaced
  *  by direct register-writes and disk_timerproc has also been attributed
  *  as RAMFUNC to be reachable.
  */

RAMFUNC void SysTick_Handler(void)
{
	static uint16_t cnt=0;
	static uint8_t flip=0, cntdiskio=0;

	if( cnt++ >= 500 ) {
		cnt = 0;
		/* alive sign */
		if ( flip ) {
			// GPIO_SetBits(GPIO_LED, GPIO_Pin_LED2 );
			GPIO_LED->BSRR = GPIO_Pin_LED2;
		} else {
			// GPIO_ResetBits(GPIO_LED, GPIO_Pin_LED2 );
			GPIO_LED->BRR = GPIO_Pin_LED2;
		}
		flip = !flip;
	}
	if ( cntdiskio++ >= 10 ) {
		cntdiskio = 0;
		disk_timerproc(); /* to be called every 10ms */
	}

	ms_tick++;
}

/**
 * @brief  Enables the CM3 systick-timer, systick-peripheral
 * and systick-interrupt to call interrupt-handler every millisecond
 * @param  none
 * @retval passed from CMSIS SysTick_Config
 */
uint32_t timebase_config(void)
{
	return SysTick_Config(SystemFrequency / 1000);
}

/**
 * @brief  Delay-loop for milliseconds
 * @param  delay in milliseconds
 * @retval none
 */
void timebase_delay_ms(uint32_t ms)
{
	uint32_t s = ms_tick;
	while ( ((uint32_t)(ms_tick-s)) < ms ) { ; }
}

/**
 * @brief  Get value of the running ms-counter
 * @param  none
 * @retval time value incremented every ms
 */
uint32_t timebase_get_count_ms(void)
{
	return ms_tick;
}

uint32_t timebase_abs_diff_ms(uint32_t t1, uint32_t t2)
{
	// return (uint32_t)(t1 - t2);
	if ( t1 > t2 ) { return t1 - t2; }
	else { return t2 - t1; }
}
