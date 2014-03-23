#include "uart.h"

void initUART1(void)
{
    U1BRG = 15; // baud rate 115200

    U1MODEbits.ALTIO = 1;//biramo koje pinove koristimo za komunikaciju osnovne ili alternativne

    IEC0bits.U1RXIE = 1; // ukljucivanje rx i tx interrupta
    IEC0bits.U1TXIE = 1;

    U1STAbits.UTXISEL = 1; //tx Interrupt when a character is transferred to the Transmit Shift register<- ovo vazi za 0!
    U1STAbits.URXISEL = 0; //rx interrupt when a char is received

    U1MODEbits.UARTEN = 1;
    U1STAbits.UTXEN = 1; // ukljucivanje predajnika mora posle ukljucivanja modula
}


char BusyUART1(void)
{  
    return(!U1STAbits.TRMT);
}

/*********************************************************************
* Function Name     : WriteUART1                                     *
* Description       : This function writes data into the UxTXREG,    *
* Parameters        : unsigned int data the data to be written       *
* Return Value      : None                                           *
*********************************************************************/

void WriteUART1(unsigned int data)
{
    if(U1MODEbits.PDSEL == 3)
        U1TXREG = data;
    else
        U1TXREG = data & 0xFF;
}
/*void initUART1 (void)//----------------------------------------------------------
{
 ConfigIntUART1(UART_RX_INT_PR4 & UART_TX_INT_PR4 & UART_RX_INT_EN & UART_TX_INT_EN);
 OpenUART1(UART_EN & UART_IDLE_CON & UART_ALTRX_ALTTX & UART_DIS_WAKE & UART_DIS_LOOPBACK & UART_DIS_ABAUD & UART_NO_PAR_8BIT & UART_1STOPBIT
          ,UART_INT_TX & UART_TX_PIN_NORMAL & UART_TX_ENABLE & UART_INT_RX_CHAR & UART_ADR_DETECT_DIS & UART_RX_OVERRUN_CLEAR
          ,0x00BF); //baud rate = 115200;
}
*/
void ConfigIntUART1(unsigned int config)
{
    /* clear IF flags */
    _U1RXIF = 0;
    _U1TXIF = 0;

    /* set priority */
    _U1RXIP = 0x0007 & config;
    _U1TXIP = (0x0070 & config) >> 4;

    /* enable/disable interrupt */
    _U1RXIE = (0x0008 & config) >> 3;
    _U1TXIE = (0x0080 & config) >> 7;
}


