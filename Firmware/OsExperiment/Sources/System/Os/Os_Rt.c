/* Os related includes */
#include "Os.h"
#include "Os_Rt.h"
#include "Os_Internals.h"

/* project specific includes */
#include "HALCore.h"


/* Task list definition */
static void RTTask_1ms(void);
static void RTTask_5ms_1(void);
static void RTTask_5ms_2(void);
static void RTTask_5ms_3(void);
static void RTTask_5ms_4(void);
static void RTTask_5ms_5(void);

static void RTTask_BackGnd(void);

tOsSchedulerElement gOsRt_TaskList[] =
{
	{&RTTask_1ms,	OS_TIME2TICK(0)},
	{&RTTask_5ms_1,	OS_TIME2TICK(100)},
	{&RTTask_1ms,	OS_TIME2TICK(1000)},
	{&RTTask_5ms_2,	OS_TIME2TICK(1100)},
	{&RTTask_1ms,	OS_TIME2TICK(2000)},
	{&RTTask_5ms_3,	OS_TIME2TICK(2100)},
	{&RTTask_1ms,	OS_TIME2TICK(3000)},
	{&RTTask_5ms_4,	OS_TIME2TICK(3100)},
	{&RTTask_1ms,	OS_TIME2TICK(4000)},
	{&RTTask_5ms_5,	OS_TIME2TICK(4100)},
};

tOsSchedulerElement gOsRt_BackgndTask =
{
	&RTTask_BackGnd, 0
};


/* definition of the startup code */
void OsRt_Init(void)
{
	HALCore_Init();


}



/* definition of timed Tasks - they are run in the low priority interrupt context */
static void RTTask_1ms(void)
{
	asm("nop");
}

static void RTTask_5ms_1(void)
{
	asm("nop");
}

static void RTTask_5ms_2(void)
{
	asm("nop");
}

static void RTTask_5ms_3(void)
{
	asm("nop");
}

static void RTTask_5ms_4(void)
{
	asm("nop");
}

static void RTTask_5ms_5(void)
{
	asm("nop");
}

/* definition of the constantly running background task - it is run in the application context */
static void RTTask_BackGnd(void)
{
	asm("nop");
}

