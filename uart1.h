#ifndef UART1_H
#define UART1_H
#include <xc.h>


void initUART1(void);
char BusyUART1(void);
void WriteUART1(unsigned int data);

//MY_UART1-----------------------------------------------------

#define TX_BUFFER_SIZE 55 //bar 10 jer punimo odjednom sa 9 bajtova
char tx_buffer[TX_BUFFER_SIZE];

#if TX_BUFFER_SIZE<256
unsigned char tx_wr_index,tx_rd_index,tx_counter;
#else
unsigned int tx_wr_index,tx_rd_index,tx_counter;
#endif


#define RX_BUFFER_SIZE 10
char rx_buffer[RX_BUFFER_SIZE];

#if RX_BUFFER_SIZE<256
unsigned char rx_wr_index,rx_rd_index,rx_counter;
#else
unsigned int rx_wr_index,rx_rd_index,rx_counter;
#endif

#endif