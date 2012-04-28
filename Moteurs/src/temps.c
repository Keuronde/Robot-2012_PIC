#include <timers.h>
#include "../include/temps.h"
#include <p18cxxx.h>

#define TIMER_INIT (65535 - 3480)

static unsigned int temps_en_290us;



void Temps_Init(){
	OpenTimer3( TIMER_INT_ON &
		T3_16BIT_RW &
		T3_SOURCE_INT &
		T3_PS_1_1 &
		T3_OSC1EN_OFF &
		T3_SYNC_EXT_OFF );
	temps_en_290us = 0;
}

void Temps_Int(){
	if(PIR2bits.TMR3IF){
		WriteTimer3(TIMER_INIT);
		PIR2bits.TMR3IF = 0; // On réarme le Timer3
		temps_en_290us++;
	}
}

unsigned int getTemps_290us(){
	unsigned int _temp;
	// Desactivation des interruptions
	INTCONbits.GIEH = 0;
	INTCONbits.GIEL = 0;
	_temp = temps_en_290us;
	// Réactivation des interruptions
	INTCONbits.GIEH = 1;
	INTCONbits.GIEL = 1;
	return _temp;
}

