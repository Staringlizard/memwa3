#ifndef _DRV_SIDBUS_H
#define _DRV_SIDBUS_H

#include "stm32h7xx_hal.h"
#include "dev_term.h"

typedef enum
{
    SIDBUS_STATE_ACTIVATE_CHIP,
    SIDBUS_STATE_DATA_SENT
} sidbus_state_t;

void drv_sidbus_irq();
void drv_sidbus_init();
void drv_sidbus_write(uint8_t addr, uint8_t value);
uint8_t drv_sidbus_read(uint8_t addr);

#endif
