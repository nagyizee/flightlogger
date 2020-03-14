#include "base.h"
#include "HALport.h"
#include "nrf52_bitfields.h"

/*--------------------------------------------------
 *                  Defines
 *--------------------------------------------------*/

typedef struct
{
    uint32  pin_nr;
    uint32  pin_cfg;
} tNrfPortCfg;

/*--------------------------------------------------
 *         Local Variables and prototypes
 *--------------------------------------------------*/

static const tNrfPortCfg    lPinConfig[] = {
    {PIN_TESTPAD_0,   ((GPIO_PIN_CNF_DIR_Output << 0) | (GPIO_PIN_CNF_INPUT_Disconnect << 1) | (GPIO_PIN_CNF_PULL_Disabled << 2) | (GPIO_PIN_CNF_DRIVE_S0S1 << 8) | (GPIO_PIN_CNF_SENSE_Disabled << 16))},
    {PIN_TESTPAD_1,   ((GPIO_PIN_CNF_DIR_Output << 0) | (GPIO_PIN_CNF_INPUT_Disconnect << 1) | (GPIO_PIN_CNF_PULL_Disabled << 2) | (GPIO_PIN_CNF_DRIVE_S0S1 << 8) | (GPIO_PIN_CNF_SENSE_Disabled << 16))},
    {PIN_I2C_SCK,     ((GPIO_PIN_CNF_DIR_Input << 0)  | (GPIO_PIN_CNF_INPUT_Connect << 1)    | (GPIO_PIN_CNF_PULL_Disabled << 2) | (GPIO_PIN_CNF_DRIVE_S0D1 << 8) | (GPIO_PIN_CNF_SENSE_Disabled << 16))},
    {PIN_I2C_SD,      ((GPIO_PIN_CNF_DIR_Input << 0)  | (GPIO_PIN_CNF_INPUT_Connect << 1)    | (GPIO_PIN_CNF_PULL_Disabled << 2) | (GPIO_PIN_CNF_DRIVE_S0D1 << 8) | (GPIO_PIN_CNF_SENSE_High << 16))},
    {PIN_ANA_PWRSRC,  ((GPIO_PIN_CNF_DIR_Input << 0)  | (GPIO_PIN_CNF_INPUT_Disconnect << 1) | (GPIO_PIN_CNF_PULL_Disabled << 2) | (GPIO_PIN_CNF_DRIVE_S0S1 << 8) | (GPIO_PIN_CNF_SENSE_Disabled << 16))},
    {PIN_ANA_EXT,     ((GPIO_PIN_CNF_DIR_Input << 0)  | (GPIO_PIN_CNF_INPUT_Disconnect << 1) | (GPIO_PIN_CNF_PULL_Disabled << 2) | (GPIO_PIN_CNF_DRIVE_S0S1 << 8) | (GPIO_PIN_CNF_SENSE_Disabled << 16))},
    {PIN_PPM_IN,      ((GPIO_PIN_CNF_DIR_Input << 0)  | (GPIO_PIN_CNF_INPUT_Connect << 1)    | (GPIO_PIN_CNF_PULL_Pulldown << 2) | (GPIO_PIN_CNF_DRIVE_S0S1 << 8) | (GPIO_PIN_CNF_SENSE_High << 16))},
    {PIN_ACC_INT,     ((GPIO_PIN_CNF_DIR_Input << 0)  | (GPIO_PIN_CNF_INPUT_Connect << 1)    | (GPIO_PIN_CNF_PULL_Pulldown << 2) | (GPIO_PIN_CNF_DRIVE_S0S1 << 8) | (GPIO_PIN_CNF_SENSE_High << 16))},
    {PIN_IF_TX_OUT,   ((GPIO_PIN_CNF_DIR_Output << 0) | (GPIO_PIN_CNF_INPUT_Disconnect << 1) | (GPIO_PIN_CNF_PULL_Disabled << 2) | (GPIO_PIN_CNF_DRIVE_H0H1 << 8) | (GPIO_PIN_CNF_SENSE_Disabled << 16))},
    {PIN_IF_RX_IN,    ((GPIO_PIN_CNF_DIR_Input << 0)  | (GPIO_PIN_CNF_INPUT_Connect << 1)    | (GPIO_PIN_CNF_PULL_Pullup << 2)   | (GPIO_PIN_CNF_DRIVE_S0S1 << 8) | (GPIO_PIN_CNF_SENSE_High << 16))},
    {PIN_IF_CTRL,     ((GPIO_PIN_CNF_DIR_Output << 0) | (GPIO_PIN_CNF_INPUT_Disconnect << 1) | (GPIO_PIN_CNF_PULL_Disabled << 2) | (GPIO_PIN_CNF_DRIVE_S0S1 << 8) | (GPIO_PIN_CNF_SENSE_Disabled << 16))},
    {PIN_LED_ON,      ((GPIO_PIN_CNF_DIR_Output << 0) | (GPIO_PIN_CNF_INPUT_Disconnect << 1) | (GPIO_PIN_CNF_PULL_Disabled << 2) | (GPIO_PIN_CNF_DRIVE_S0S1 << 8) | (GPIO_PIN_CNF_SENSE_Disabled << 16))},
    {PIN_LED_BLE,     ((GPIO_PIN_CNF_DIR_Output << 0) | (GPIO_PIN_CNF_INPUT_Disconnect << 1) | (GPIO_PIN_CNF_PULL_Disabled << 2) | (GPIO_PIN_CNF_DRIVE_S0S1 << 8) | (GPIO_PIN_CNF_SENSE_Disabled << 16))},
    {PIN_FLS_RESET,   ((GPIO_PIN_CNF_DIR_Output << 0) | (GPIO_PIN_CNF_INPUT_Disconnect << 1) | (GPIO_PIN_CNF_PULL_Disabled << 2) | (GPIO_PIN_CNF_DRIVE_S0S1 << 8) | (GPIO_PIN_CNF_SENSE_Disabled << 16))},
    {PIN_FLS_CLK,     ((GPIO_PIN_CNF_DIR_Output << 0) | (GPIO_PIN_CNF_INPUT_Connect << 1)    | (GPIO_PIN_CNF_PULL_Disabled << 2) | (GPIO_PIN_CNF_DRIVE_H0H1 << 8) | (GPIO_PIN_CNF_SENSE_Disabled << 16))},
    {PIN_FLS_SI,      ((GPIO_PIN_CNF_DIR_Output << 0) | (GPIO_PIN_CNF_INPUT_Disconnect << 1) | (GPIO_PIN_CNF_PULL_Disabled << 2) | (GPIO_PIN_CNF_DRIVE_H0H1 << 8) | (GPIO_PIN_CNF_SENSE_Disabled << 16))},
    {PIN_FLS_SO,      ((GPIO_PIN_CNF_DIR_Input << 0)  | (GPIO_PIN_CNF_INPUT_Connect << 1)    | (GPIO_PIN_CNF_PULL_Disabled << 2) | (GPIO_PIN_CNF_DRIVE_S0S1 << 8) | (GPIO_PIN_CNF_SENSE_High << 16))},
    {PIN_FLS_CS,      ((GPIO_PIN_CNF_DIR_Output << 0) | (GPIO_PIN_CNF_INPUT_Disconnect << 1) | (GPIO_PIN_CNF_PULL_Disabled << 2) | (GPIO_PIN_CNF_DRIVE_S0S1 << 8) | (GPIO_PIN_CNF_SENSE_Disabled << 16))},
    {PIN_PRESS_INT,   ((GPIO_PIN_CNF_DIR_Input << 0)  | (GPIO_PIN_CNF_INPUT_Connect << 1)    | (GPIO_PIN_CNF_PULL_Pulldown << 2) | (GPIO_PIN_CNF_DRIVE_S0S1 << 8) | (GPIO_PIN_CNF_SENSE_High << 16))}
};

//static const uint32 lPinOutDef = ((0 << PIN_TESTPAD_0)|(0 << PIN_TESTPAD_1)|(1 << PIN_IF_TX_OUT)|(0, PIN_IF_CTRL)|(0, PIN_LED_ON)|(0, PIN_LED_BLE)|(0, PIN_FLS_RESET)|(1, PIN_FLS_CS));

/*--------------------------------------------------
 *             Interface Functions
 *--------------------------------------------------*/

void HALPort_Init(void)
{
    int i;
    /* set up pin output default values */
    NRF_P0->OUT = 0x00U;

    /* set up pin configuration */
    for (i = 0; i < (sizeof(lPinConfig) / sizeof(tNrfPortCfg)); i++)
    {
        NRF_P0->PIN_CNF[lPinConfig[i].pin_nr] = lPinConfig[i].pin_cfg;
    }


}
