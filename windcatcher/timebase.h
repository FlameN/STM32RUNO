/*
 *  Created on: 02.10.2009
 *      Author: mthomas
 */

#ifndef TIMEBASE_H_
#define TIMEBASE_H_

#include <stdint.h>

uint32_t timebase_config(void);
void timebase_delay_ms(uint32_t ms);
uint32_t timebase_get_count_ms(void);
uint32_t timebase_abs_diff_ms(uint32_t t1, uint32_t t2);

static inline uint32_t timebase_diff(uint32_t t1, uint32_t t2) {
	return (t1 > t2) ? ( t1 - t2 ) : ( t2 - t1 );
}

#endif /* TIMEBASE_H_ */
