/*
 *
 *      NXP MMA8452 3x axis accelerometer internal stuff
 *
 */

#ifndef DRIVERS_NXPACCEL_NXPACCEL_INTERNALS_H_
#define DRIVERS_NXPACCEL_NXPACCEL_INTERNALS_H_

/*--------------------------------------------------
 *                              Includes
 *--------------------------------------------------*/

#include "NxpBaro.h"

/*--------------------------------------------------
                                Type definitions
 *--------------------------------------------------*/

#define REGACC_STATUS               (0x00)              // 1byte accelero sensor status register
#define REGACC_OUTX                 (0x01)              // 2byte X axis G value
#define REGACC_OUTY                 (0x03)              // 2byte Y axis G value
#define REGACC_OUTZ                 (0x05)              // 2byte Z axis G value
#define REGACC_ID                   (0x0D)              // 1byte accelero sensor chip ID
#define REGACC_XYZDATACFG           (0x0E)              // 1byte XYZ data setup register
#define REGACC_CTRL1                (0x2A)              // 1byte control register 1 - generic functional stuff
#define REGACC_CTRL2                (0x2B)              // 1byte control register 2 - sampling modes, selftest, etc.
#define REGACC_CTRL3                (0x2C)              // 1byte control register 3 - interrupt setup
#define REGACC_CTRL4                (0x2D)              // 1byte control register 4 - interrupt enable
#define REGACC_CTRL5                (0x2E)              // 1byte control register 5 - interrupt pin.cfg

#define AREG_ID_VALUE               (0x2A)              // device ID

#define AREG_XYZDATACFG_HPF         (0x10)              // enable highpass filter
#define AREG_XYZDATACFG_FS2         (0x00)              // 2G full scale range
#define AREG_XYZDATACFG_FS4         (0x01)              // 4G full scale range
#define AREG_XYZDATACFG_FS8         (0x02)              // 8G full scale range

#define AREG_CFG1_DR_800            (0x00 << 3)         // output data rate config for 800Hz
#define AREG_CFG1_DR_400            (0x01 << 3)         // output data rate config for 400Hz
#define AREG_CFG1_DR_200            (0x02 << 3)         // output data rate config for 200Hz
#define AREG_CFG1_DR_100            (0x03 << 3)         // output data rate config for 100Hz
#define AREG_CFG1_DR_50             (0x04 << 3)         // output data rate config for 50Hz
#define AREG_CFG1_DR_12_5           (0x05 << 3)         // output data rate config for 12.5Hz
#define AREG_CFG1_DR_6_25           (0x06 << 3)         // output data rate config for 6.25Hz
#define AREG_CFG1_DR_1_56           (0x07 << 3)         // output data rate config for 1.56Hz
#define AREG_CFG1_ACTIVE            (0x01)              // set to active mode

#define AREG_CFG2_SELFTEST          (0x80)              // start selftest
#define AREG_CFG2_MOD_NORMAL        (0x00)
#define AREG_CFG2_MOD_LNLP          (0x01)
#define AREG_CFG2_MOD_HIRES         (0x02)
#define AREG_CFG2_MOD_LP            (0x03)

#define AREG_CFG3_IPOL_H            (0x02)              // interrupt polarity high
#define AREG_CFG3_OPENDRAIN         (0x01)              // interrupt output is open drain

#define AREG_CFG4_INTEN_DRDY        (0x01)              // data ready interrupt enable

#define AREG_CFG5_INTPIN_DRDY       (0x01)              // interrupt pin for data ready on INT1 (if not set - it is INT2)

/* fifo size */
#define ACC_MAX_FIFO_ELEMS          (8)

/* selftest valid threshold */
#define SELFTEST_DIFF_MIN           (200)
#define SELFTEST_DIFF_MAX           (600)

/* substates for different states */
#define SUBST_INI_CHIPTEST          (0)
#define SUBST_INI_INISET1           (1)
#define SUBST_INI_INISET2           (2)
#define SUBST_INI_SELFTEST          (3)
#define SUBST_INI_SELFTEST_RESULT   (4)
#define SUBST_INI_FINALIZE          (5)

#define SUBST_READ_NORMALMODE       (0)
#define SUBST_READ_WAITEVENT        (1)
#define SUBST_READ_WAITRESULT       (2)
#define SUBST_READ_SHUTDOWN         (3)

#define ACQ_CONT_NONE       0
#define ACQ_CONT_ON         1
#define ACQ_CONT_SHTDN      2

typedef enum
{
    accdrvst_initializing = 0,
    accdrvst_idle,
    accdrvst_sensor_read,
    accdrvst_error,
} tAccDrvState;


typedef struct
{
    tAccDrvState        state;
    uint32              substate;           /* it can have a sequence number for init, low power, or a substate value */
    tResult             error_code;

    uint8               fail_ctr;           /* failure retrial counter */
    uint8               timeout_ctr;        /* timeout to failure */

    uint8               acq_cont;           /* continuous acqusition mode */

    tNxpAccelResult     fifo_data[ACC_MAX_FIFO_ELEMS];      /* data fifo with the results */
    uint8               fifo_r;             /* fifo read pointer */
    uint8               fifo_w;             /* fifo write pointer */
    uint8               fifo_c;             /* fifo data count */

    uint8               selftest;           /* selftest in progress */

    tNxpAccelResult     selftst_data;       /* measurement with the selftest */

    uint8               hw_buff[8];         /* workbuffer from the sensor in i2c */
} tAccDrvInternals;

/*--------------------------------------------------
 *                              Defines
 *--------------------------------------------------*/

/*--------------------------------------------------
 *                              Exported interfaces
 *--------------------------------------------------*/


#endif /* DRIVERS_NXPACCEL_NXPACCEL_INTERNALS_H_ */
