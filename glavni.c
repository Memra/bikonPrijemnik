#include "uart1.h"
#include "uart2.h"
#include "tajmeri.h"
#include <math.h>
#include <ports.h>
#include <libq.h>

_FWDT(WDT_OFF);
_FOSC(CSW_FSCM_OFF & XT_PLL8);

#define BEACON_IDE  'M'

#define STATIC_BEACON_1 0x55
#define STATIC_BEACON_2 0x77
#define STATIC_BEACON_3 0xDB

#define NAPAJANJE_TRAFOA LATBbits.LATB7
#define GASENJE_MOTORA LATEbits.LATE3
#define LED_B1 LATEbits.LATE4
#define LED_B2 LATEbits.LATE5
#define LED_B3 LATFbits.LATF6
#define LED_B4 LATDbits.LATD2
#define POZICIJA1 PORTFbits.RF5
#define POZICIJA2 PORTFbits.RF1

volatile unsigned char tempRX1=0,tempRX2=0,receive_buff[3],transmit_buff[10],komanda;
volatile unsigned int T_counter;
volatile unsigned int pom, ceo_krug;
volatile unsigned int bikon1, bikon2, bikon3, bikonX, min[3],max[3], maxX, minX,max_temp[3], min_temp[3];
volatile unsigned char obisao_krug,ulovio_bikonX;
volatile unsigned char BIKON1, BIKON2, BIKON3;
volatile unsigned char ivica, flagMAX,flagMIN;
volatile unsigned char b1_1, b2_1 ,b3_1;
volatile unsigned char ulovio_bikon1, ulovio_bikon2, ulovio_bikon3;
volatile float fi1, fi2, alfa, beta, gama;
volatile long x,y;
const float RAD=0.017453292519943295769236907684886;
volatile  unsigned char ivica;
volatile unsigned char kreni;
volatile unsigned char bikon_start[3];
volatile unsigned int vremeStart[3], vremeStop[3];

volatile int sendX, sendY;

// FUNKCIJA ZA KASNJENJE----------------------------------------------------------------------------
void delay_x1ms(unsigned int value)
{
    T_counter = 0;
    while(T_counter <= value);
}

void MyPutchar(unsigned char c)
{
   /* while(tx_counter == TX_BUFFER_SIZE); //ako soft bufer pun ceka da se uprazni mesto, ovo pokusavamo izbeci
    SRbits.IPL = 7;                                   //upotrebom bafera duzine koja odgovara max broju uzastopnih upisa
    if (U1STAbits.UTXBF) //ako hard bufer pun puni soft buffer
 {
  tx_buffer[tx_wr_index]=c;
  if (++tx_wr_index == TX_BUFFER_SIZE) tx_wr_index=0;
  ++tx_counter;
 }
 else
  U1TXREG=c;


    SRbits.IPL = 0;*/
    while(tx_counter == TX_BUFFER_SIZE);

    //if(tx_counter != TX_BUFFER_SIZE)
   // {
        SRbits.IPL = 7;
        if(tx_counter || (U1STAbits.UTXBF == 1))
        {
            tx_buffer[tx_wr_index] = c;
            if(++tx_wr_index == TX_BUFFER_SIZE)
                tx_wr_index = 0;

            ++tx_counter;
        }
        else
            U1TXREG = c;

        SRbits.IPL = 0;
   // }
}

void dvobajtno(int ulaz)
{
    MyPutchar(ulaz >> 8);
    MyPutchar(ulaz);
}


// INICIJALIZACIJA------------------------------------------------------------------------------
void Inicijalizacija(void)
{
		//inicijalizacije razne:
	// INICIJALIZACIJA PORTOVA
	ADPCFGbits.PCFG7=1;
	TRISBbits.TRISB7=0;
	TRISDbits.TRISD2=0;
	TRISEbits.TRISE4=0;
	TRISEbits.TRISE5=0;
	TRISFbits.TRISF6=0;
	TRISEbits.TRISE3=0;
	TRISFbits.TRISF5=1;
	TRISFbits.TRISF1=1;
	ADPCFG = 0xFFFF; // all PORTB = Digital

	initUART1();
	initUART2();

	pom=0;
	ceo_krug=0;
	obisao_krug=0;

	ConfigINT0(RISING_EDGE_INT & EXT_INT_PRI_5 & EXT_INT_ENABLE);
        ConfigINT2(RISING_EDGE_INT & EXT_INT_PRI_4 & EXT_INT_ENABLE);
	initTIMER2();

	kreni=0;
	min[0]=min[1]=min[2]=max[0]=max[1]=max[2]=0;

        NAPAJANJE_TRAFOA=1;
	GASENJE_MOTORA=0;

	LED_B1=0;
	LED_B2=0;
	LED_B3=0;
	LED_B4=0;
	delay_x1ms(500);
	LED_B1=1;
	LED_B2=1;
	LED_B3=1;
	LED_B4=1;
	delay_x1ms(500);
	LED_B1=0;
	LED_B2=0;
	LED_B3=0;
	LED_B4=0;
	delay_x1ms(500);
	LED_B1=1;
	LED_B2=1;
	LED_B3=1;
	LED_B4=1;

	//INTCON1bits.NSTDIS = 1; // onemoguceni ugnjezdeni interrupti
	BIKON1=BIKON2=BIKON3=0;
	bikon1=bikon2=bikon3=0;
	ulovio_bikonX=0;
	bikonX=0;
	ivica=flagMAX=flagMIN=0;
	b1_1=b2_1=b3_1=0;
	ulovio_bikon1=ulovio_bikon2=ulovio_bikon3=0;
	fi1=fi2=alfa=beta=gama=0;
	x=y=0;
	ivica=0;
}


// INTERAPT TAJMERA1-----------------------------------------------------------------------------
void __attribute__((__interrupt__, auto_psv)) _T1Interrupt(void)
{

    IFS0bits.T1IF = 0;   
    pom++; // oznacava vreme, koristi se za merenje uglova

} 

/*************************************************
 Generise se nakon poslatog karaktera, 
 ako soft bafer nije prazan uzima 1 karakter
 iz soft bafera i upisuje ga u hard bafer
 F-je slanja i prijema UART i odogvarajuci interapti
 su u najvecoj meri preuzeti iz gotovih f-ja progama CodeVision 
**************************************************/ 

void __attribute__((interrupt, auto_psv)) _U1TXInterrupt(void) 
{
    IFS0bits.U1TXIF = 0;
/*
    if(tx_counter)
    {
        --tx_counter;
        U1TXREG=tx_buffer[tx_rd_index];
        if(++tx_rd_index == TX_BUFFER_SIZE)
            tx_rd_index=0;
    }

  */

    if(tx_counter)
    {
        while(tx_counter && (U1STAbits.UTXBF == 0))
	{
            --tx_counter;
            U1TXREG = tx_buffer[tx_rd_index];
            if(++tx_rd_index == TX_BUFFER_SIZE)
                tx_rd_index = 0;
	}
    }
}

volatile unsigned char send = 0;
// SERIJSKI INTERAPT 1---------------------------------------------------------------
void __attribute__((__interrupt__, auto_psv)) _U1RXInterrupt(void)
{
    IFS0bits.U1RXIF = 0;

    tempRX1 = U1RXREG;
  //  komanda=tempRX1;
    receive_buff[2]=receive_buff[1];
    receive_buff[1]=receive_buff[0];
    receive_buff[0]=tempRX1;

    if(receive_buff[0]=='S' && receive_buff[1]=='S' && receive_buff[2]=='S')
    {
        kreni = 1;
        GASENJE_MOTORA = 1;
    }

    if(receive_buff[0]=='Q' && receive_buff[1]=='Q' && receive_buff[2]=='Q')
    {
        GASENJE_MOTORA = 0;
        LED_B1 = 0;
	LED_B2 = 0;
	LED_B3 = 0;
	LED_B4 = 0;
        //INTCON1bits.NSTDIS = 1;
        //while(1);
    }

    if(receive_buff[2] == BEACON_IDE && receive_buff[1] == 'F' && receive_buff[0] == 'E')
    {
        dvobajtno(sendX);
        dvobajtno(sendY);
    }
}  

// SPOLJASNJI INTERAPT0--------------------------------------------------------------------
void __attribute__((__interrupt__, auto_psv)) _INT0Interrupt(void)
{ 																		//Interrupt on INT2	
    IFS0bits.INT0IF = 0;                								//interrupt flag cleared
    ceo_krug=pom;
    obisao_krug=1;
}
volatile unsigned char status;
 //SERIJSKI INTERAPT 2-------------------------------------------------------------------------
void __attribute__((__interrupt__, auto_psv)) _U2RXInterrupt(void)
{
    IFS1bits.U2RXIF = 0;

    status = U2STAbits.PERR | U2STAbits.FERR | U2STAbits.OERR;
    tempRX2 = U2RXREG;
    
    //if(status == 0)
   // {
        switch(tempRX2)
	{
            case STATIC_BEACON_2: // BIKON 2
               LED_B2 = 0;
               if(bikon_start[1] == 0)
                {
                    bikon_start[1] = 1;
                    vremeStart[1] = vremeStop[1] = pom;
                }
                else
                {
                    vremeStop[1] = pom;
                    BIKON2 = 1;
                }
                break;

	case STATIC_BEACON_3: // BIKON 3
            LED_B3 = 0;
            if(bikon_start[2] == 0)
            {
                bikon_start[2] = 1;
                vremeStart[2] = vremeStop[2] = pom;
            }
            else
            {
                vremeStop[2] = pom;
                BIKON3 = 1;
            }

            break;

	case STATIC_BEACON_1: //BIKON 1
            LED_B1 = 0;
            if(bikon_start[0] == 0)
            {
                bikon_start[0] = 1;
                vremeStart[0] = vremeStop[0] = pom;
            }
            else
            {
                vremeStop[0] = pom;
                BIKON1 = 1;
            }

            break;
	} // end of switch
  //  } // end of if

    U2STAbits.OERR = 0;
} 

//------------------------------------------------------------------------------

volatile int time = 0;

// INTERAPT TAJMERA2-------------------------------------------------------------------------
void __attribute__((__interrupt__, auto_psv)) _T2Interrupt(void)
{

   TMR2 = 0;
   IFS0bits.T2IF = 0;
   T_counter++;

}
/*
long absinth(long a)//apsutna vrednost
{
	return a>0 ? a:-a; 
}*/

void NewLine()
{
	while(BusyUART1());
	WriteUART1(13);
	while(BusyUART1());
	WriteUART1(10);
}

void SendNum(int value)
{
	unsigned char c10000, c1000, c100, c10, c1;
        if(value < 0)
        {
            MyPutchar('-');
            value *= -1;
        }
	c1=value % 10;
	value/=10;
	c10=value % 10;
	value/=10;
	c100=value % 10;
	value/=10;
	c1000=value % 10;
	value/=10;
	c10000=value % 10;
	value/=10;
//	c100000=value % 10;
//	while(BusyUART1());
//	MyPutchar(c100000+'0');
	while(BusyUART1());
	MyPutchar(c10000+'0');
	while(BusyUART1());
	MyPutchar(c1000+'0');
	while(BusyUART1());
	MyPutchar(c100+'0');
	while(BusyUART1());
	MyPutchar(c10+'0');
	while(BusyUART1());
      MyPutchar(c1+'0');
}
//-----------------------------------------------------------------------------------------
void TraziBikone(void)
{
    unsigned int temp = 0;

    if(BIKON1)	//uhvacen je bikon 1
    {
	min[0] = max[0] = (vremeStart[0] + vremeStop[0]) / 2;
        ulovio_bikon1 = 1;
    }
    else if(bikon_start[0] == 1)
    {
        temp = 0;
        while(!BIKON1)
        {
            if(++temp >= 3)
                break;

            delay_x1ms(1);
        }

        min[0] = max[0] = (vremeStart[0] + vremeStop[0]) / 2;
        ulovio_bikon1 = 1;
    }

    if(BIKON2)	// uhvacen je bikon 2
    {
	min[1] = max[1] = (vremeStart[1] + vremeStop[1]) / 2;
        ulovio_bikon2 = 1;
    }
    else if(bikon_start[1] == 1)
    {
        temp = 0;
        while(!BIKON2)
        {
            if(++temp >= 3)
                break;

            delay_x1ms(1);
        }

        min[1] = max[1] = (vremeStart[1] + vremeStop[1]) / 2;
        ulovio_bikon2 = 1;
    }

    if(BIKON3)	//uhvacen je bikon 3
    {
	min[2] = max[2] =  (vremeStart[2] + vremeStop[2]) / 2;
        ulovio_bikon3 = 1;
    }
    else if(bikon_start[2] == 1)
    {
        temp = 0;
        while(!BIKON3)
        {
            if(++temp >= 3)
                break;

            delay_x1ms(1);
        }

        min[2] = max[2] = (vremeStart[2] + vremeStop[2]) / 2;
        ulovio_bikon3 = 1;
    }
}

//-------------------------------------------------------------------------
void ReinicijalizujPromenljive(void)
{	

    BIKON1 = BIKON2 = BIKON3 = 0;
    bikon_start[0] = bikon_start[1] = bikon_start[2] = 0;
  
	//	ivica=flagMAX=flagMIN=0;
//		b1_1=b2_1=b3_1=0;
		//ulovio_bikon1=ulovio_bikon2=ulovio_bikon3=0;
    obisao_krug = 0;

   // vremeBuff[0] = vremeBuff[1] = vremeBuff[2] = 0;
  //  brojOcitavanja[0] = brojOcitavanja[2] = brojOcitavanja[1] = 0;
		
}

//-------------------------------------------------------------------------------
void IspisBikon(void)
{
	if(ulovio_bikon1==1) 
	{
	   while(BusyUART1());
	   WriteUART1('1');
	         
	}
	else 
	{
	   while(BusyUART1());
	   WriteUART1('5');
	}  
	if(ulovio_bikon2==1) 
	{
		while(BusyUART1());
	    WriteUART1('2');
	}
	else 
	{
		while(BusyUART1());
	    WriteUART1('6');
	}
	if(ulovio_bikon3==1)
	{	
		while(BusyUART1());
	    WriteUART1('3');}
	else 
	{
		while(BusyUART1());
		WriteUART1('7');
	}
	WriteUART1(' ');
	WriteUART1(' ');
}

float tangens(float angle)
{
    _Q16 intValue = _Q16tan((1L << 16) * angle);

    return (intValue / (float)(1L << 16));
}

//-------------------------------------------------------------------------------------
void RacunajPoziciju(volatile long* iks, volatile long* ipsilon)
{
    volatile float x,y,tmp;
    volatile float x1,x2,y1,y2;

    alfa = (max[0] + min[0]) * (180.0 / ceo_krug);
    beta = (max[1] + min[1]) * (180.0 / ceo_krug);
    gama = (max[2] + min[2]) * (180.0 / ceo_krug);

    fi1 = beta - alfa;
    fi1 = fi1 < 0 ? fi1 + 360 : fi1;
   // if(fi1 < 0)
     //   fi1+=360;

    fi2 = alfa - gama;
    fi2 = fi2 < 0 ? fi2 + 360 : fi2;
    //if(fi2 < 0)
     //   fi2+=360;

    x = tangens(fi1 * RAD);
    y = tangens(fi2 * RAD);

    x1 = 525 - 1500 / x;
    y1 = 1500 + 525 / x;
    x2 = -525 + 1500 / y;
    y2 = 1500 + 525 / y;

    x = y2 - y1;
    y = x1 - x2;
    tmp = 2 * (x1 * y2 - x2 * y1) / (x * x + y * y);
    x *= tmp;
    y *= tmp;

    x += 1000;

    *iks = x;
    *ipsilon = y;
}
/*
//---------------------------------------------------------------------------------------------------
void Koordinate(float fi1, float fi2, volatile long* iks, volatile long* ipsilon)
{
	float x,y,tmp;
	float x1,x2,y1,y2;

	//kaljkuljacije:
	x = tangens(fi1 * RAD);
	y = tangens(fi2 * RAD);

	x1 = 525 - 1500 / x;
	y1 = 1500 + 525 / x;
	x2 = -525 + 1500 / y;
	y2 = 1500 + 525 / y;

	x = y2 - y1;
	y = x1 - x2;
	tmp = 2 * (x1 * y2 - x2 * y1) / (x * x + y * y);
	x *= tmp;
	y *= tmp;
	
	*iks = x;
	*ipsilon = y;
}
*/
//--------------------------------------------------------------------------------------------------
void IspisKoordinate_ASCII(void)
{
if((ulovio_bikon1!=1)||(ulovio_bikon2!=1)||(ulovio_bikon3!=1))
{
	MyPutchar('X');
//	delay_x1ms(5);
	MyPutchar(0xFF);
	MyPutchar(0xFF);
//	delay_x1ms(5);
	MyPutchar('Y');
//	delay_x1ms(5);
	MyPutchar(0xFF);
	MyPutchar(0xFF);
//	MyPutchar('e');
}
else
{
	MyPutchar('X');
	delay_x1ms(5);
        x += 1050;

SendNum(x);
MyPutchar(' ');
MyPutchar(' ');

	delay_x1ms(5);
	MyPutchar('Y');
	delay_x1ms(5);
SendNum(y);
NewLine();
//	MyPutchar('e');
}
}

//---------------------------------------------------------------------------------------------------
int main(void)
{
    while(!OSCCONbits.LOCK); // wait for PLL to lock
    Inicijalizacija();

    while(kreni == 0);
    GASENJE_MOTORA = 1;
    initTIMER1();
		
    while(1)
    {
	while(obisao_krug == 0);
        TraziBikone();

	T1CONbits.TON=0;	// zaustavi tajmer1
	WriteTimer1(0);		//resetuj tajmer1    
	pom=0;
	ReinicijalizujPromenljive();
	//	IspisBikon();

	if(ulovio_bikon1==1 && ulovio_bikon2==1 && ulovio_bikon3==1)
	{
            RacunajPoziciju(&x, &y);
            
            SRbits.IPL = 7;
            sendX = x;
            sendY = y;
            SRbits.IPL = 0;

           /* if(send)
            {
                MyPutchar(BEACON_IDE);
                dvobajtno(sendX);
                dvobajtno(sendY);  
                send = 0;
            }*/
        }

        ulovio_bikon1=ulovio_bikon2=ulovio_bikon3=0;
	min[0]=min[1]=min[2]=max[0]=max[1]=max[2]=0;     

	T1CONbits.TON=1;	// pusti tajmer1

	while(obisao_krug==0);
	pom = 0;
        ReinicijalizujPromenljive();


/*		MyPutchar(' ');
		SendNum(pom);*/
	LED_B1=LED_B2=LED_B3=1;
//		pom=0;
//		
		
    }	// od glavnog while-a


    return 0;
}


