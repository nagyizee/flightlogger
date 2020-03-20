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
#include "CypFlash.h"
#include "Nvm.h"
/* AppRT */
#include "RtAppExample.h"
#include "RtAppData.h"
#include "RtAppPack.h"
#include "RtAppComm.h"
#include "RtAppSensor.h"
/* AppBgnd */


/* Task list definition */
static void RTTask_1ms(void);
static void RTTask_5ms_1(void);
static void RTTask_5ms_2(void);
static void RTTask_5ms_3(void);
static void RTTask_5ms_4(void);
static void RTTask_5ms_5(void);

static void RTTask_BackGnd(uint32 reason);

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
    HALI2C_Init();
    HALSPI_Init();
    
    CypFlash_Init();
    Nvm_Init();
    
    RtAppData_Init();
}

/* definition of timed Tasks - they are run in the low priority interrupt context */
static void RTTask_1ms(void)
{
#ifdef RTAPPEXAMPLEACTIVE
    RtAppExample_Main(0);
    CypFlash_Main();
#endif
}

static void RTTask_5ms_1(void)
{
#ifdef RTAPPEXAMPLEACTIVE
    RtAppExample_Main(1);
#endif
    RtAppSensor_Main();
}

static void RTTask_5ms_2(void)
{
#ifdef RTAPPEXAMPLEACTIVE
    RtAppExample_Main(2);
#endif
    RtAppComm_Main();
    Nvm_Main();
}

static void RTTask_5ms_3(void)
{
#ifdef RTAPPEXAMPLEACTIVE
    RtAppExample_Main(3);
#endif
    RtAppPack_Main();
}

static void RTTask_5ms_4(void)
{
#ifdef RTAPPEXAMPLEACTIVE
    RtAppExample_Main(4);
#endif
    RtAppData_Main();
}

static void RTTask_5ms_5(void)
{
#ifdef RTAPPEXAMPLEACTIVE
    RtAppExample_Main(5);
#endif
    Nvm_Main();
}

/* definition of the constantly running background task - it is run in the application context */
static void RTTask_BackGnd(uint32 reason)
{

}
