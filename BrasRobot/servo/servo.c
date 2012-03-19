/** I N C L U D E S **********************************************************/ 
#include <timers.h>	
#include <p18f2550.h>
#include "servo/servo.h"


/** V A R I A B L E S ********************************************************/
volatile char timer_servo = 1; // Compteur qui permet d'avoir une periode de 10 ms.
volatile char servo_courant = 0; // Compteur qui sauvegarde le servo courant.
volatile int pos_servo[NB_SERVO]; 		// Tableau qui contient les valeurs 



void Servo_Init(){
	char i;
	WriteTimer2(0);
	TRIS_SERVO1 = 0;
	TRIS_SERVO2 = 0;
	for(i=0;i<NB_SERVO;i++){
		pos_servo[i]=0x0400;
	}
	// Ouverture du timer
	OpenTimer2( TIMER_INT_ON & T2_PS_1_16 & T2_POST_1_1);
	IPR1bits.TMR2IP = 1;
    
}
void Servo_Set(int unsigned position, char num_servo){
	if(position > SERVO_MAX)
		position = SERVO_MAX;

	if(position < SERVO_MIN)
		position = SERVO_MIN;

	pos_servo[num_servo] = position;
}
int Servo_Get(char num){
	return pos_servo[num];
}


