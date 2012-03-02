#include <timers.h>
#include "temps.h"
#include <p18cxxx.h>


// 50536 = 0xC568
#define TIMER_INIT 50536
#define TIMER_L    0x68
#define TIMER_H    0xC5

volatile unsigned char centisecondes;



void Temps_Init(){
	/*OpenTimer3( TIMER_INT_ON &
		T3_16BIT_RW &
		T3_SOURCE_INT &
		T3_PS_1_8 &
		T3_OSC1EN_OFF &
		T3_SYNC_EXT_OFF );*/
	
	centisecondes = 0; // Initialisation de la variable
	// Ecriture du timer
	TMR3L = 0;
	TMR3H = TIMER_H;
	TMR3L = TIMER_L;
	// Interrutpion timer :
	IPR2bits.TMR3IP = 0; // Interruption basse
	PIR2bits.TMR3IF = 0; // On enlève le drapeau
	PIE2bits.TMR3IE = 1; // On active l'interruption.
	T3CON = 0x31;	// Ecriture/lecture en mode 8 bits
					// Prescaler = 1:8
					// Horloge interne
	
}

void Temps_Int(void){
	if(PIR2bits.TMR3IF){
		PIR2bits.TMR3IF = 0; // On réarme le Timer3
		// Desactivation des interruptions
		INTCONbits.GIEH = 0;
		INTCONbits.GIEL = 0;
		//WriteTimer3(TIMER_INIT);
		// Ecriture du timer
		TMR3H = TIMER_H;
		TMR3L = TIMER_L;
		// Réactivation des interruptions
		INTCONbits.GIEH = 1;
		INTCONbits.GIEL = 1;
		centisecondes++;
		
	}
}


unsigned int getTemps_cs(){
	unsigned int a;
	PIE2bits.TMR3IE = 0;
	INTCONbits.GIE = 0;
	INTCONbits.PEIE = 0;
	a = centisecondes;
	INTCONbits.GIE = 1;
	INTCONbits.PEIE = 1;
	PIE2bits.TMR3IE = 1;
	return a;
}
unsigned  int getTemps_s(){
	return (unsigned  int) getTemps_cs()/128;
}
unsigned int getTemps_micro_s(){
	return  (ReadTimer3() - TIMER_INIT) / 1.5;
}
