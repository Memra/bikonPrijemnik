#include "uart2.h"

// TSOP
void initUART2(void)
{
    U2BRG = 63;//28800 za kvarc 14.7456Mhz i PLL 8

    IEC1bits.U2RXIE = 1;//omogucavamo rx1 interupt

    U2STAbits.URXISEL = 0; //rx interrupt when a char is received

    U2MODEbits.UARTEN = 1;
    //U2STAbits.UTXEN=1; // nema potrebe za predajom???
}

/*********************************************************************
* Function Name     : WriteUART1                                     *
* Description       : This function writes data into the UxTXREG,    *
* Parameters        : unsigned int data the data to be written       *
* Return Value      : None                                           *
*********************************************************************/

void WriteUART2(unsigned int data)
{
    if(U2MODEbits.PDSEL == 3)
        U2TXREG = data;
    else
        U2TXREG = data & 0xFF;
}



