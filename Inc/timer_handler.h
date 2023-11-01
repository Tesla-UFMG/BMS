/*
 * timer_handler.h
 *
 *  Created on: Jun 27, 2022
 *      Author: iFeli
 */

#ifndef INC_TIMER_HANDLER_H_
#define INC_TIMER_HANDLER_H_

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Waits a specific timer to be elapsed by a desired amount. Allows
 * multiple timers by sending a different timer_start.
 *
 * @param timer_start Timer specifier.
 * @param delay Amount desired to wait, max is 2^32, which is equal to
 * approximately 50 days. Values higher than that will not work as expected!
 * @return true The desired time has elapsed.
 * @return false The desired time has not been elapsed.
 */
bool timer_wait_ms(uint32_t timer_start, uint32_t delay);
/**
 * @brief Restart timer to current tick value.
 *
 * @param timer_to_restart Address of timer specifier.
 */
void timer_restart(uint32_t* timer_to_restart);

#endif /* INC_TIMER_HANDLER_H_ */
