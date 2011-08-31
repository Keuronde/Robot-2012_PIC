#include <p18cxxx.h>
#include <timers.h>

/** D E F I N E D ********************************************************/
#define ROUGE 0
#define BLEU 1
#define VIDE 2

/** V A R I A B L E S ********************************************************/
#pragma udata

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
	

/*	
	if(INTCONbits.TMR0IF == 1){
		INTCONbits.TMR0IF = 0;
		WriteTimer0(65535 - 48000); //pour un préscaler de 1 : 12000 = 1ms
                                    // On est à 4 ms.
		timer++;
	}
*/

	PRODL = sauv1;
	PRODH = sauv2;		

}

#pragma interrupt MyInterrupt_L
void MyInterrupt_L(void){

}


#pragma code




void main(void){
    Init();
    
    
    while(1){

    }

}

void Init(){
   	INTCONbits.GIEH = 1; // Activation interruptions hautes
  	INTCONbits.GIEL = 1; // Activation interruptions basses
  	RCONbits.IPEN=1; // Activation des niveau d'interruptions
    // Initialisation du Timer 0 pour la base de temps
	OpenTimer0(	TIMER_INT_ON &  // interruption ON
				T0_16BIT &		// Timer 0 en 16 bits
				T0_SOURCE_INT & // Source interne (Quartz + PLL)
				T0_PS_1_1);		// 1 cycle, 1 incrémentation
	TRISCbits.TRISC1 = 0;
	PORTCbits.RC1 = 1;
}













