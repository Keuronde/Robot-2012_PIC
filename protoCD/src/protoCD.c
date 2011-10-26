#include <p18cxxx.h>
#include <delays.h>
#include <timers.h>
#include "../include/servo.h"

/** P R I V A T E  P R O T O T Y P E S ***************************************/
void MyInterrupt(void);
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
    ;
}
#pragma code


#pragma interrupt MyInterrupt 
void MyInterrupt(void)
{
	// code de "Rustre Corner"
	// Adapté et modifié par S. KAY
	unsigned char sauv1;
	unsigned char sauv2;

	sauv1 = PRODL;
	sauv2 = PRODH;	


	if (PIR1bits.TMR2IF)
	{
		// On réarme le timer
		PIR1bits.TMR2IF = 0;
		WriteTimer2(0);
		timer_servo--;
		if(timer_servo <= 0){
			if(servo_courant == 0){
				SERVO1=1;
				timer_servo = pos_servo[0] >> 8; //4
				if((pos_servo[servo_courant] & 0x00FF) != 0){
					WriteTimer2((unsigned char)(0x100 - (pos_servo[servo_courant] & 0x00FF)));
				}else{
					WriteTimer2(0);
					timer_servo--;
				}
				
				servo_courant++;
			}else if(servo_courant == 1){
			  SERVO1=0;
				SERVO2=1;
				timer_servo = pos_servo[servo_courant] >> 8;
				if((pos_servo[servo_courant] & 0x00FF) != 0){
					WriteTimer2((unsigned char)(0x100 - (pos_servo[servo_courant] & 0x00FF)));
				}else{
					WriteTimer2(0);
					timer_servo--;
				}
				
				servo_courant++;
			}else{
				servo_courant = 0;
				SERVO2=0;
				timer_servo = 25;//25 - (pos_servo[servo_courant] >> 8);
			}
		}		
	}


	PRODL = sauv1;
	PRODH = sauv2;		
}


#pragma code

void main(void){
  // Init :
  Servo_Init();
  Servo_Set(BRAS_BAS);
  Servo_Set(PINCE_OUVERT);
  
  
  while(1){
	  Servo_Set(PINCE_FERMEE);
	  // Attente 0,5s
	  Delay10KTCYx(0);
	  Delay10KTCYx(0);
	  
	  Servo_Set(BRAS_HAUT);
	  // Attente 1s
	  Delay10KTCYx(0);
	  Delay10KTCYx(0);
	  Delay10KTCYx(0);
	  Delay10KTCYx(0);
	  
	  Servo_Set(PINCE_LACHE);
	  // Attente 1s
	  Delay10KTCYx(0);
	  Delay10KTCYx(0);
	  Delay10KTCYx(0);
	  Delay10KTCYx(0);
	  Servo_Set(PINCE_FERMEE);
	  // Attente 0,5s
	  Delay10KTCYx(0);
	  Delay10KTCYx(0);
	  
	  Servo_Set(BRAS_BASCULE);
	  // Attente 1s
	  Delay10KTCYx(0);
	  Delay10KTCYx(0);
	  Delay10KTCYx(0);
	  Delay10KTCYx(0);
	  
	  
	  
	  Servo_Set(PINCE_OUVERT);
	  // Attente 0,5s
	  Delay10KTCYx(0);
	  Delay10KTCYx(0);
	  
	  Servo_Set(BRAS_BAS);
	  // Attente 2s
	  Delay10KTCYx(0);
	  Delay10KTCYx(0);
	  Delay10KTCYx(0);
	  Delay10KTCYx(0);
	  Delay10KTCYx(0);
	  Delay10KTCYx(0);
	  Delay10KTCYx(0);
	  Delay10KTCYx(0);
	  
  }
  
}
