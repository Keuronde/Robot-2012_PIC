#include <p18cxxx.h>
#include <timers.h>
#include <delays.h>
#include <pwm.h>
#include "config.h"
#include "servo/servo.h"

/** D E F I N E D ********************************************************/
// Identifiant balise
#define ID_BALISE 0x44
// Emission IR
#define NB_MESSAGES 16
#define NB_MSG_TOTAL 48
// Clignotement LED
#define F_1HZ 90
#define F_5HZ 18
// LED
#define TRIS_LED TRISBbits.TRISB7
#define LED LATBbits.LATB7
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
	
	Servo_Int();

	PRODL = sauv1;
	PRODH = sauv2;		

}

#pragma interrupt MyInterrupt_L
void MyInterrupt_L(void){

}


#pragma code




void main(void){
	int servopos = SERVO_MIN;
	int pas = 0x0040;
	char ok =1;
    Init();
    Servo_Set(servopos,1);
	Servo_Set(servopos,0);


    M1_ENABLE = 1;
    M1_SENS = 1;
    
    M2_ENABLE = 1;
    M2_SENS = 1;
    
  
    while(1){
		if(CT6 == 1){ // CT arriere
			// Avance
			M1_SENS = 1;
			CT10 = 1;
			CT9 = 0;
		}else{
			CT9 = 1;
			CT10 = 0;
		}
		if(CT5 == 0){ // CT avant
			// Recule
			M1_SENS = 0;
		}
		if((BOUTON == 0) && (ok ==1)) {
			servopos += pas;
			if (servopos ==  (SERVO_MAX +pas)){
				servopos = SERVO_MIN;
			}else if (servopos > SERVO_MAX){
				servopos = SERVO_MAX;
			}
			Servo_Set(servopos,1);
			Servo_Set(servopos,0);
			Delay10KTCYx(0);
			ok = 0;
		}
		if(BOUTON ==1){
			ok = 1;
		}
		
    }

}

void Init(){
	// Toutes les pattes en digital (pas d'analogique)
    ADCON1 |= 0x0F;
    // Contacteur en sortie (pour le test)
	TRIS_CT1 = 1;
	TRIS_CT2 = 1;
	TRIS_CT3 = 1;
	TRIS_CT4 = 1;
	TRIS_CT5 = 1;
	TRIS_CT6 = 1;
	TRIS_CT7 = 1;
	TRIS_CT8 = 1;
	TRIS_CT9 = 0;
	TRIS_CT10 = 0;
	
	CT10 = 1;
	Delay10KTCYx(0);
	Delay10KTCYx(0);

	CT10 = 0;
	Delay10KTCYx(0);
	Delay10KTCYx(0);

	
	TRIS_M1_ENABLE = 0;
	TRIS_M2_ENABLE = 0;
	TRIS_M1_SENS = 0;
	TRIS_M2_SENS = 0;
	
	TRIB_BOUTON = 1;
	OpenTimer2(TIMER_INT_OFF & T2_PS_1_4 & T2_POST_1_1);
	OpenPWM1(0xff);
//	OpenPWM2(0xff);
	SetDCPWM1(0x0320);
//	SetDCPWM2(0x02AA);


	Servo_Init();

	
}



