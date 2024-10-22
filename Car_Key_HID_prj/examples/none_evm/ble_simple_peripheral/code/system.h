#ifndef   __SYSTEM_H
#define   __SYSTEM_H
#include <stdint.h>
extern uint32_t tick;
enum{
	ASK_TIM = 0,
	MEM_CHECK_TIM,
	TIM_CNT,
};

extern uint32_t system_t[TIM_CNT];
#define CHECK_TM(A)  system_t[A]
#define SET_TEM(A, B)	system_t[A]	= B








#endif