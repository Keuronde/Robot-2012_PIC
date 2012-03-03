#include <p18cxxx.h>
#include "servo/servo.h"
#include "moteurs/moteurs.h"
#include "temps/temps.h"


#define TIMER_L    0x68
#define TIMER_H    0xC5

/** V A R I A B L E S ********************************************************/
#pragma idata
volatile unsigned char timer_test=0;
volatile unsigned char T3_test_H=0;
volatile unsigned char T3_test_L=0;


/** P R I V A T E  P R O T O T Y P E S ***************************************/
void MyInterrupt(void);
void MyInterrupt_L(void);

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
	
	Servo_Int();
}

#pragma interrupt MyInterrupt_L
void MyInterrupt_L(void){
	// Timer 3 en interruption basse
	
	if(PIR2bits.TMR3IF){
		PIR2bits.TMR3IF = 0; // On réarme le Timer3

		TMR3H = TIMER_H;
		TMR3L = TIMER_L;

		centisecondes++;
	}
	//Temps_Int();
	
}


#pragma code
void main(void){
	char i=0;
	unsigned int temps_cs=0;
	unsigned int tmp=0,tmp1=0,tmp_old = 0;
	
    /* Initialisation **************************************/
    // Sorties
    TRISA = 0xF0;
    TRISB = 0x1F; // RB5, RB6, RB7 en sortie

    /*// Timer 3
    T3CON = 0xB1; // sans préscaler, Timer On
    T1CONbits.RD16 = 1;
    IPR2bits.TMR3IP = 0; // Interruption basse
	PIR2bits.TMR3IF = 0; // On enlève le drapeau
	PIE2bits.TMR3IE = 1; // On active l'interruption.*/
	
    // Timer 2
    T2CON = 0x7C; // Préscaler 1:16, Timer ON
    IPR1bits.TMR2IP = 1; // Interruption haute
	PIR1bits.TMR2IF = 0; // On enlève le drapeau
	PIE1bits.TMR2IE = 1; // On active l'interruption.
	
    // On active toutes les interruptions
	INTCONbits.GIE = 1;		// Hautes
	INTCONbits.PEIE = 1;	// Basses
	// On active les priorité d'interruption
	RCONbits.IPEN = 1;		// Priorites
	
	Servo_Init();
	Temps_Init();
	tmp = timer_test;
	tmp_old = tmp;


    while(1){
		// On récupère l'heure 
		//tmp = timer_test;
		tmp = getTemps_cs();
		if( (tmp != tmp_old) ){
			if(tmp != (unsigned int) (tmp_old+1)){
				LATA |= 0x03;
			}
			tmp_old = tmp;
			tmp1 = (tmp_old>>4);
		}
		
		if(temps_cs != tmp1)		
		{
			temps_cs = tmp1;
			i++;
			if (i>3){
				i=1;
			}
		}
		// Reset debug 
		if (PORTBbits.RB4 == 0){
			LATA = 0;
		}
		
		
		
		// Allumage LEDs
		switch (i){
			case 1:
				LATB = 0x20;
				break;
			case 2:
				LATB = 0x40;
				break;
			case 3:
				LATB = 0x80;
				break;
			default:
				break;
		}
    }
}



