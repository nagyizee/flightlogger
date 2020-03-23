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
#include "NxpAccel.h"
#include "CypFlash.h"
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
    {&RTTask_1ms,	OS_TIME2TICK(   0)},
    {&RTTask_5ms_1,	OS_TIME2TICK( 500)},
    {&RTTask_1ms,	OS_TIME2TICK(1000)},
    {&RTTask_5ms_2,	OS_TIME2TICK(1500)},
    {&RTTask_1ms,	OS_TIME2TICK(2000)},
    {&RTTask_5ms_3,	OS_TIME2TICK(2500)},
    {&RTTask_1ms,	OS_TIME2TICK(3000)},
    {&RTTask_5ms_4,	OS_TIME2TICK(3500)},
    {&RTTask_1ms,	OS_TIME2TICK(4000)},
    {&RTTask_5ms_5,	OS_TIME2TICK(4500)},
};

tOsTaskBgndItem gOsRt_BgndTask = &RTTask_BackGnd;

/* definition of the startup code */
void OsRt_Init(void)
{
    /* init HAL modules */
    HALCore_Init();
    HALPort_Init();
    HALI2C_Init();
    HALSPI_Init();
    /* now it is safer to activate interrupts */

// problems at HALOsSys with HALOsSys_GetCurrentCounter
//  - can not use  HALOsSys_EnableAllInterrupts();
//TODO: solve the issue, and remove the mess below:
    #include "nrf.h"
    __enable_irq();


    /* init drivers and system modules */
    CypFlash_Init();
    NXPBaro_Init();
//placeholder for    NxpAccel_Init();
    /* init application modules */


//it blocks inside - need investigation
//    RtAppData_Init();
}

/* definition of timed Tasks - they are run in the low priority interrupt context */
static void RTTask_1ms(void)
{
#ifdef RTAPPEXAMPLEACTIVE
<<<<<<< HEAD
    RtAppExample_Main(0);
=======
//    RtAppExample_Main(0);
    Nvm_Main();
    CypFlash_Main();
>>>>>>> NVM
#endif
    HALI2C_MainFunction();
}

static void RTTask_5ms_1(void)
{
#ifdef RTAPPEXAMPLEACTIVE
//    RtAppExample_Main(1);
#endif
    RtAppSensor_Main();
}

static void RTTask_5ms_2(void)
{
#ifdef RTAPPEXAMPLEACTIVE
//    RtAppExample_Main(2);
#endif
    RtAppComm_Main();
<<<<<<< HEAD
    NXPBaro_MainFunction();
=======
>>>>>>> NVM
}

static void RTTask_5ms_3(void)
{
#ifdef RTAPPEXAMPLEACTIVE
//    RtAppExample_Main(3);
#endif
    RtAppPack_Main();
}

static void RTTask_5ms_4(void)
{
#ifdef RTAPPEXAMPLEACTIVE
//    RtAppExample_Main(4);
#endif
    RtAppData_Main();
    //place holder for NXPAccel_MainFunction();
}

static void RTTask_5ms_5(void)
{
#ifdef RTAPPEXAMPLEACTIVE
    RtAppExample_Main(5);
#endif
<<<<<<< HEAD
    CypFlash_Main();
=======
    
>>>>>>> NVM
}

/* definition of the constantly running background task - it is run in the application context */
static void RTTask_BackGnd(uint32 reason)
{

}
