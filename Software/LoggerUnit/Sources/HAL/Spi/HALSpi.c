/**
 *
 * SPI driver first version
 * Static configuration
 * 1 Channel only
 * Fixed Chip select
 */

/*--------------------------------------------------
 * 		    		Includes
 *--------------------------------------------------*/

#include <string.h>
#include "HALSpi.h"
#include "HALSpi_Internals.h"

/*--------------------------------------------------
 * 		    		Defines
 *--------------------------------------------------*/

#define DEV_SPI                 (NRF_SPIM1)
#define IRQ_SPI                 (SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQn)

#define DATA_CHUNK_SIZE         (255u)

/*--------------------------------------------------
 * 	       Local Variables and prototypes
 *--------------------------------------------------*/

const static tSpiChannelConfig   lSpiConfig[] =
{
    { HALSPI_CHIPSEL_CYPFLASH },
};

static tSpiInternals             lSpi;

static void local_ISRSpiSetupTx(tSpiChannelParams *pChparam);
static void local_ISRSpiSetupRx(tSpiChannelParams *pChparam);

static void local_SpiSetupTx(void);
static void local_SpiSetupRx(void);


/*--------------------------------------------------
 *             Interrupt Service Function
 *--------------------------------------------------*/

void ISRHandler_Spi(void)
{
    uint32 ch;
    /* note: lSpi.op_ch and lSpi.ch[ch] can not be changed by application till spi is busy */
    ch = lSpi.op_ch;
    if (DEV_SPI->EVENTS_ENDRX)
    {
        DEV_SPI->EVENTS_ENDRX = 0;
        if (lSpi.ch[ch].rx_remaining)
        {
            local_ISRSpiSetupRx(&lSpi.ch[ch]);
        }
        else
        {
            /* no reception buffer to set up - deactivate IRQ source for it */
            DEV_SPI->INTENCLR = SPIM_INTENSET_ENDRX_Msk;
            DEV_SPI->RXD.MAXCNT = 0;
            /* note: rx/tx size can not be changed from application till Spi is busy, and Application isn't monitoring it either */
            lSpi.ch[ch].rx_size = 0;
        }
    }
    if (DEV_SPI->EVENTS_ENDTX)
    {
        DEV_SPI->EVENTS_ENDTX = 0;
        if (lSpi.ch[ch].tx_remaining)
        {
            local_ISRSpiSetupTx(&lSpi.ch[ch]);
        }
        else
        {
            /* no reception buffer to set up - deactivate IRQ source for it */
            DEV_SPI->INTENCLR = SPIM_INTENSET_ENDTX_Msk;
            DEV_SPI->TXD.MAXCNT = 0;
            /* note: rx/tx size can not be changed from application till Spi is busy, and Application isn't monitoring it either */
            lSpi.ch[ch].tx_size = 0;
        }
    }

    if (DEV_SPI->INTENSET == 0)
    {
        /* note: it is set by application only if spi is not in busy - can be cleaned by ISR only, application will only read it */
        lSpi.busy = false;
    }
    else
    {
        /* we still have stuff to transmit */
        DEV_SPI->TASKS_START = 1;
    }
}


/*--------------------------------------------------
 *   		   Interface Functions
 *--------------------------------------------------*/

/* Device handling functions */

void HALSPI_Init(void)
{
    /* primitive hardcoded implementation */

    /* set the pins for the peripheral */
    DEV_SPI->PSEL.SCK = PIN_FLS_CLK;
    DEV_SPI->PSEL.MISO = PIN_FLS_SO;
    DEV_SPI->PSEL.MOSI = PIN_FLS_SI;
    /* enable the peripheral and set it up */
    DEV_SPI->ENABLE = SPIM_ENABLE_ENABLE_Enabled;
    DEV_SPI->FREQUENCY = SPI_FREQUENCY_FREQUENCY_M8;
    DEV_SPI->CONFIG = 0x00;     /* keep it default */
    DEV_SPI->ORC = 0x00;        /* use 0x00 as padding for TX */

    /* set up interrupt handling for the SPI */
    NVIC_SetPriority(IRQ_SPI, 0);
    NVIC_EnableIRQ(IRQ_SPI);

    /* initialize local variables */
    memset(&lSpi, 0, sizeof(tSpiInternals));
    lSpi.op_ch = SPI_CHANNEL_FREE;
}

tSpiStatus HALSPI_GetStatus(tSpiChannelType ch)
{
    if ((lSpi.busy) ||                                                  /* busy state if peripheral is communicating */
        ((lSpi.op_ch != SPI_CHANNEL_FREE) && (lSpi.op_ch != ch)))       /* OR not the current channel is chipselected */
    {
        return SPI_BUSY;
    }
    else if (lSpi.op_ch == ch)                                           /* current channel is chipselected */
    {
        return SPI_READY;
    }
    /* if we reached this far - means SPI is idle */
    return SPI_IDLE;
}

tResult HALSPI_StartTransfer(tSpiChannelType ch)
{
    if (ch != lSpi.op_ch)
    {
        return RES_INVALID;                 /* current channel is not chipselected */
    }
    if (lSpi.busy)
    {
        return RES_BUSY;                    /* peripheral is busy transmitting */
    }
    if ((lSpi.ch[ch].rx_size == 0) &&
        (lSpi.ch[ch].tx_size == 0))
    {
        return RES_OK;                      /* nothing to do */
    }

    /* set by application, cleared by ISR */
    /* note: no mutex is needed - will maintain true value till ISR will not reset it */
    lSpi.busy = true;

    /* set up TX and Rx */
    /* note: no mutex is needed - IRQ is not active till transfer is not started */
    local_SpiSetupTx();
    local_SpiSetupRx();

    /* clear the interrupt flags */
    DEV_SPI->EVENTS_ENDRX = 0;
    DEV_SPI->EVENTS_ENDTX = 0;
    /* start the transfer */
    DEV_SPI->EVENTS_STARTED = 0;
    DEV_SPI->TASKS_START = 1;
    /* activate interrupt sources */
    if (lSpi.ch[ch].rx_size)
    {
        DEV_SPI->INTENSET = SPIM_INTENSET_ENDRX_Msk;
    }
    if (lSpi.ch[ch].tx_size)
    {
        DEV_SPI->INTENSET = SPIM_INTENSET_ENDTX_Msk;
    }

    return RES_OK;
}


/* Chip select handling functions */
tResult HALSPI_SetCS(tSpiChannelType ch)
{
    if (lSpi.op_ch != SPI_CHANNEL_FREE)
    {
        if ((ch != lSpi.op_ch) || (lSpi.busy))
        {
            return RES_BUSY;   /* other channel is using the chip select */
        }
        else
        {
            return RES_OK;     /* already in chip select */
        }
    }
    GPIO_FLS_CS->OUTCLR  = (1U << lSpiConfig[ch].cs_pin);
    lSpi.op_ch = ch;
    return RES_OK;
}

void HALSPI_ReleaseCS(tSpiChannelType ch)
{
    if (lSpi.op_ch != ch)
    {
        return;                 /* not our channel - do nothing */
    }

    if (lSpi.busy)
    {
        /* in case if channel is busy - stop any transfer */
        DEV_SPI->INTENCLR = SPIM_INTENSET_ENDRX_Msk | SPIM_INTENSET_ENDTX_Msk;
        DEV_SPI->EVENTS_ENDRX = 0;
        DEV_SPI->EVENTS_ENDTX = 0;
        DEV_SPI->EVENTS_STOPPED = 0;
        DEV_SPI->TASKS_STOP = 1;
        while (DEV_SPI->EVENTS_STOPPED == 0)
        {
        }
        lSpi.busy = false;
    }
    GPIO_FLS_CS->OUTSET  = (1U << lSpiConfig[ch].cs_pin);
    lSpi.op_ch = SPI_CHANNEL_FREE;
}


/* Data handling functions */
tResult HALSPI_TxData(tSpiChannelType ch, uint16 cnt , uint8 *buf)
{ 
    if (lSpi.busy && (ch == lSpi.op_ch))
    {
        return RES_BUSY;
    }

    if ((cnt == 0) || (buf == NULL))
    {
        /* if size or buffer is 0 then clear the buffer parameters */
        lSpi.ch[ch].tx_buff = NULL;
        lSpi.ch[ch].tx_size = 0;
        lSpi.ch[ch].tx_remaining = 0;
    }
    else
    {
        lSpi.ch[ch].tx_buff = buf;
        lSpi.ch[ch].tx_size = cnt;
        lSpi.ch[ch].tx_remaining = cnt;
    }
    return RES_OK;
}

tResult HALSPI_RxData(tSpiChannelType ch, uint16 cnt , uint8 *buf)
{
    if (lSpi.busy && (ch == lSpi.op_ch))
    {
        return RES_BUSY;
    }

    if ((cnt == 0) || (buf == NULL))
    {
        /* if size or buffer is 0 then clear the buffer parameters */
        lSpi.ch[ch].rx_buff = NULL;
        lSpi.ch[ch].rx_size = 0;
        lSpi.ch[ch].rx_remaining = 0;
    }
    else
    {
        lSpi.ch[ch].rx_buff = buf;
        lSpi.ch[ch].rx_size = cnt;
        lSpi.ch[ch].rx_remaining = cnt;
    }
    return RES_OK;
}

/*--------------------------------------------------
 *   		   Local Functions
 *--------------------------------------------------*/

static void local_SpiSetupTx(void)
{
    uint32 ch;

    /* note: the used parameters are only changeable by application if the channel is not busy.
     *       if busy then only the ISR can operate them */
    ch = lSpi.op_ch;

    if (lSpi.ch[ch].tx_size)
    {
        /* note: safe to call - can be called from application if IRQs are inactive */
        local_ISRSpiSetupTx(&lSpi.ch[ch]);
    }
    else
    {
        DEV_SPI->TXD.MAXCNT = 0;
    }
}

static void local_SpiSetupRx(void)
{
    uint32 ch;

    /* note: the used parameters are only changeable by application if the channel is not busy.
     *       if busy then only the ISR can operate them */
    ch = lSpi.op_ch;

    if (lSpi.ch[ch].rx_size)
    {
        /* note: safe to call - can be called from application if IRQs are inactive */
        local_ISRSpiSetupRx(&lSpi.ch[ch]);
    }
    else
    {
        DEV_SPI->RXD.MAXCNT = 0;
    }

}

static void local_ISRSpiSetupTx(tSpiChannelParams *pChparam)
{
    uint16 cnt;

    cnt = pChparam->tx_remaining;
    if (cnt > DATA_CHUNK_SIZE)
    {
        cnt = DATA_CHUNK_SIZE;
    }

    DEV_SPI->TXD.PTR = (uint32)&pChparam->tx_buff[0];
    DEV_SPI->TXD.MAXCNT = (uint8)cnt;
    pChparam->tx_remaining -= cnt;
    pChparam->tx_buff += cnt;
}

static void local_ISRSpiSetupRx(tSpiChannelParams *pChparam)
{
    uint16 cnt;

    cnt = pChparam->rx_remaining;
    if (cnt > DATA_CHUNK_SIZE)
    {
        cnt = DATA_CHUNK_SIZE;
    }

    DEV_SPI->RXD.PTR = (uint32)&pChparam->rx_buff[0];
    DEV_SPI->RXD.MAXCNT = (uint8)cnt;
    pChparam->rx_remaining -= cnt;
    pChparam->rx_buff += cnt;
}
