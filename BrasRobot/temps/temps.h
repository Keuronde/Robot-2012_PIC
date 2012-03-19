extern volatile unsigned int centisecondes;

void Temps_Init(void);
unsigned int getTemps(void);
unsigned  int getTemps_s(void);
unsigned int getTemps_cs(void);
unsigned int getTemps_micro_s(void);
#define TIMER_INIT 50536
#define TIMER_L    0x68
#define TIMER_H    0xC5


// A ne pas utiliser - Do not use
// Macro à utiliser en interruption
#define Temps_Int() {        \
	if(PIR2bits.TMR3IF){     \
		PIR2bits.TMR3IF = 0; \
		INTCONbits.GIEH = 0; \
		INTCONbits.GIEL = 0; \
		TMR3H = TIMER_H;     \
		TMR3L = TIMER_L;     \
		INTCONbits.GIEH = 1; \
		INTCONbits.GIEL = 1; \
		centisecondes++;     \
	}                        \
}
