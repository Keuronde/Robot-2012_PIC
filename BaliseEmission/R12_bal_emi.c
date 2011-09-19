#include <p18cxxx.h>
#include <timers.h>
#include <pwm.h>

/** D E F I N E D ********************************************************/
// Identifiant balise
#define ID_BALISE 0x44
// Emission IR
#define NB_MESSAGES 16
#define NB_MSG_TOTAL 48
// Clignotement LED
#define F_1HZ 90
#define F_5HZ 18
/** V A R I A B L E S ********************************************************/
#pragma udata
volatile unsigned char timer_led;
volatile unsigned char timer_emi;
/** P R I V A T E  P R O T O T Y P E S ***************************************/
void MyInterrupt(void);
void MyInterrupt_L(void);
void Init(void);


/** V E C T O R  R E M A P P I N G *******************************************/

extern void _startup (void);        // See c018i.c in your C18 compiler dir
#pragma code _RESET_INTERRUPT_VECTOR = 0x000800
void _reset (void)
{
    _asm goto _startup _endasm
}
#pragma code

#pragma code _HIGH_INTERRUPT_VECTOR = 0x000808
void _high_ISR (void)
{
    MyInterrupt();
}

#pragma code _LOW_INTERRUPT_VECTOR = 0x000818
void _low_ISR (void)
{
    MyInterrupt_L();
}
#pragma code


#pragma interrupt MyInterrupt 
void MyInterrupt(void){
	// code de "Rustre Corner"
	// Adapté et modifié par S. KAY
	unsigned char sauv1;
	unsigned char sauv2;

	sauv1 = PRODL;
	sauv2 = PRODH;
	

	
	if(INTCONbits.TMR0IF == 1){
		INTCONbits.TMR0IF = 0;
		//WriteTimer0(0xffff - 2074); // avec un préscaler de 128,
									// on est à 180,7 Hz
		TMR0H=0xF7;
		TMR0L=0xE5;
		timer_led++;
		timer_emi++;
		if(timer_emi > NB_MSG_TOTAL)
			timer_emi=0;
		
		// Emission UART
		if(timer_emi < NB_MESSAGES){
			TXREG = ID_BALISE;
		}
		
		
	}


	PRODL = sauv1;
	PRODH = sauv2;		

}

#pragma interrupt MyInterrupt_L
void MyInterrupt_L(void){

}


#pragma code




void main(void){
	char t_diode;
    Init();
    
    t_diode = F_1HZ;
    while(1){
		if(timer_led > t_diode){
			PORTCbits.RC1 = !PORTCbits.RC1;
			timer_led = 0;
		}

    }

}

void Init(){
	// Activation des interruptions
   	INTCONbits.GIEH = 1; // Activation interruptions hautes
  	INTCONbits.GIEL = 1; // Activation interruptions basses
  	RCONbits.IPEN=1; // Activation des niveau d'interruptions
  	
    // Initialisation du Timer 0 pour la base de temps
	OpenTimer0(	TIMER_INT_ON &  // interruption ON
				T0_16BIT &		// Timer 0 en 16 bits
				T0_SOURCE_INT & // Source interne (Quartz + PLL)
				T0_PS_1_32);		// 128 cycle, 1 incrémentation
	WriteTimer0(0xffff - 2074);
	timer_emi = 0;
	timer_led = 0;
	
	// Initialisation de l'UART
	TXSTAbits.TX9  = 0; // Mode 8 bits
	TXSTAbits.TXEN = 1; // Activation du module UART de transmission
	TXSTAbits.BRGH = 0; // Gestion de la base de temps
	BAUDCONbits.BRG16 = 1; // Gestion de la base de temps
	SPBRGH = 3; // Gestion de la base de temps => 0x0361 = 829
	SPBRG = 61; // Gestion de la base de temps
	RCSTAbits.SPEN=1;
	TRISCbits.TRISC6 = 1;
	TRISCbits.TRISC7 = 1;
	
	// Configuration de la MLI
	OpenTimer2( TIMER_INT_OFF & T2_PS_1_4 & T2_POST_1_1 );
	OpenPWM1(82);
	SetDCPWM1((unsigned int)164);
	
	
	
	// On allume la LED
	TRISCbits.TRISC1 = 0;
	PORTCbits.RC1 = 1;
	
}













