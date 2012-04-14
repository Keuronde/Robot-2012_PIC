#include <timers.h>
#include "../include/temps.h"
#include <p18cxxx.h>

#define TIMER_INIT (65535 - 48000)

static unsigned long temps_en_4ms;



void Temps_Init(){
	OpenTimer3( TIMER_INT_ON &
		T3_16BIT_RW &
		T3_SOURCE_INT &
		T3_PS_1_1 &
		T3_OSC1EN_OFF &
		T3_SYNC_EXT_OFF );
	temps_en_4ms = 0;
}

void Temps_Int(){
	if(PIR2bits.TMR3IF){
		WriteTimer3(TIMER_INIT);
		PIR2bits.TMR3IF = 0; // On réarme le Timer3
		temps_en_4ms++;
	}
}

unsigned int getTemps_4ms(){
	unsigned int _temp;
	// Desactivation des interruptions
	INTCONbits.GIEH = 0;
	INTCONbits.GIEL = 0;
	_temp = temps_en_4ms;
	// Réactivation des interruptions
	INTCONbits.GIEH = 1;
	INTCONbits.GIEL = 1;
	return temps_en_4ms;
}

