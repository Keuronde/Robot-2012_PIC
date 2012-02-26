#include <timers.h>
#include "temps.h"
#include <p18cxxx.h>


// 50536 = 0xC568
#define TIMER_INIT 50536
#define TIMER_L    0x68
#define TIMER_H    0xC5

volatile unsigned int centisecondes;



void Temps_Init(){
	OpenTimer3( TIMER_INT_ON &
		T3_16BIT_RW &
		T3_SOURCE_INT &
		T3_PS_1_8 &
		T3_OSC1EN_OFF &
		T3_SYNC_EXT_OFF );
	IPR2bits.TMR3IP = 0; // Interruption basse
	centisecondes = 0;
}

void Temps_Int(){
	if(PIR2bits.TMR3IF){
		WriteTimer3(TIMER_INIT);
		/*TMR3H = TIMER_H;
		TMR3L = TIMER_L;*/
		centisecondes++;
		PIR2bits.TMR3IF = 0; // On réarme le Timer3
	}
}

unsigned int getTemps_cs(){
	unsigned int a;
	PIE2bits.TMR3IE = 0;
	a = centisecondes;
	PIE2bits.TMR3IE = 1;
	return a;
}
unsigned char getTemps_s(){
	return (unsigned char) getTemps_cs()/100;
}
unsigned int getTemps_micro_s(){
	return  (ReadTimer3() - TIMER_INIT) / 1.5;
}
