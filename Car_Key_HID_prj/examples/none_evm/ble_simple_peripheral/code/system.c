#include <stdio.h>
#include <string.h>
#include "system.h"
#include "co_printf.h"
#include "sys_utils.h"
#include "FIFO.h"
#include "driver_iomux.h"
#include "driver_system.h"
#include "driver_uart.h"
#include "driver_plf.h"
#include "driver_timer.h"
__attribute__((section("ram_code"))) void uart0_isr_ram(void)
{
    uint8_t int_id;
    uint8_t c;
    volatile struct uart_reg_t *uart_reg = (volatile struct uart_reg_t *)UART0_BASE;
		
    int_id = uart_reg->u3.iir.int_id;
    if(int_id == 0x04 || int_id == 0x0c )   /* Receiver data available or Character time-out indication */
    {
        while(uart_reg->lsr & 0x01)
        {
          c = uart_reg->u1.data;
					FIFO_Write_OneByte(FIFO_INDEX0, c);
        }
    }
    else if(int_id == 0x06)
    {
        //uart_reg->u3.iir.int_id = int_id;
        //uart_reg->u2.ier.erlsi = 0;
        volatile uint32_t line_status = uart_reg->lsr;
    }
}
uint32_t system_t[TIM_CNT];
uint32_t tick;
uint8_t i;
__attribute__((section("ram_code"))) void timer0_isr_ram(void)
{
    timer_clear_interrupt(TIMER0);
    tick++;
		for(i = 0; i < TIM_CNT; i++)
			if(system_t[i])system_t[i]--;
}