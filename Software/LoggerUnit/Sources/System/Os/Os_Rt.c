/* Os related includes */
#include "Os.h"
#include "Os_Rt.h"
#include "Os_Internals.h"

/* project specific includes */
/* HAL */
#include "HALCore.h"
#include "HALport.h"
#include "HALSpi.h"
#include "HALI2c.h"
/* System */
/* Drivers */
#include "NxpBaro.h"
/* AppRT */
#include "ExampleRtApp.h"
/* AppBgnd */


/* Task list definition */
static void RTTask_1ms(void);
static void RTTask_5ms_1(void);
static void RTTask_5ms_2(void);
static void RTTask_5ms_3(void);
static void RTTask_5ms_4(void);
static void RTTask_5ms_5(void);

static void RTTask_BackGnd(uint32 reason);

static uint32 task_Counter = 0;

tOsSchedulerElement gOsRt_TaskList[OSRT_TASK_LIST_LENGTH] =
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

tOsTaskBgndItem gOsRt_BgndTask = &RTTask_BackGnd;

/* definition of the startup code */
void OsRt_Init(void)
{
  HALCore_Init();
  HALPort_Init();
}

/* definition of timed Tasks - they are run in the low priority interrupt context */
static void RTTask_1ms(void)
{
  ExampleRtApp_Main(0);
}

static void RTTask_5ms_1(void)
{
  ExampleRtApp_Main(1);
  
  if (task_Counter==100)
  {  
    HALSPI_ReleaseCS(0);
  }
}

static void RTTask_5ms_2(void)
{
  ExampleRtApp_Main(2);

  task_Counter++;
  if (task_Counter>=200) 
    task_Counter=0;
}

static void RTTask_5ms_3(void)
{
  TSpiStatus retval;
  ExampleRtApp_Main(3);

  if (task_Counter==85)
  {
    retval = HALSPI_SetCS(0);
  }
}

static void RTTask_5ms_4(void)
{
  ExampleRtApp_Main(4);
}

static void RTTask_5ms_5(void)
{
  ExampleRtApp_Main(5);
}

/* definition of the constantly running background task - it is run in the application context */
static void RTTask_BackGnd(uint32 reason)
{

}

