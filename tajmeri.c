#include <xc.h>
#include <timer.h>
#include "tajmeri.h"

/**************************************************************************************
Naziv funkcije: initTIMER1
Opis funkcije: Inicijalizuje tajmer 1 da broji po 10us, tj. da pravi prekide na 
svakih 10uss
Parametri: nema
Povratna vrednost: nema
**************************************************************************************/
void initTIMER1(void)			//konfiguracija Timer1 da broji po 10 us 
{
	//unsigned int match_value = 60;  //60   
        unsigned int match_value = 120; //
	ConfigIntTimer1(T1_INT_PRIOR_4 & T1_INT_ON);		//prioritet i ukljucivanje tajmera
	WriteTimer1(0);
	OpenTimer1(T1_ON & T1_GATE_OFF & T1_IDLE_CON & T1_PS_1_1 & T1_SYNC_EXT_OFF & T1_SOURCE_INT,match_value );

}

/**************************************************************************************
Naziv funkcije: initTIMER2
Opis funkcije: Inicijalizuje tajmer 2 da broji po 1ms, tj. da pravi prekide na 
svakih 1ms
Parametri: nema
Povratna vrednost: nema
**************************************************************************************/
void initTIMER2(void)			//konfiguracija Timer2 da broji po 1ms 
{
	unsigned int match_value = 29491;    // ide na 1 ms sa kvarcom 14.7456Mhz
       
	ConfigIntTimer2(T2_INT_PRIOR_2 & T2_INT_ON);		//prioritet i ukljucivanje tajmera
	WriteTimer2(0);
	OpenTimer2(T2_ON & T2_GATE_OFF & T2_IDLE_CON & T2_PS_1_1 &  T2_SOURCE_INT,match_value );

}
   