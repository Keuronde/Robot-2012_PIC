#include "config.h"
/** D E F I N E **************************************************************/
// Constantes
#define SERVO_MIN 0x02D0 
#define SERVO_MAX 0x07C0
#define NB_SERVO 2


// SERVO 1 
#define DOIGT_G_OUVERT 0x2D0,0
#define DOIGT_G_TIRE   0x4D0,0
#define DOIGT_G_RABAT  0x5D0,0
#define DOIGT_G_FERME  0x710,0
// SERVO 2 (marqué servo 1 sur la carte réelle)
#define CMUCAM_HAUT 0x3D0,1
#define CMUCAM_BAS  0x4D0,1

/** V A R I A B L E S ********************************************************/
extern volatile char timer_servo; // Compteur qui permet d'avoir une periode de 10 ms.
extern volatile char servo_courant; // Compteur qui sauvegarde le servo courant.
extern volatile int pos_servo[NB_SERVO]; 		// Tableau qui contient les valeurs 



/** P R I V A T E  P R O T O T Y P E S ***************************************/
void Servo_Set(unsigned int,char);
void Servo_Init(void);
int Servo_Get(char);


#define Servo_Int(){                                     \
	if (PIR1bits.TMR2IF)                                 \
	{                                                    \
		PIR1bits.TMR2IF = 0;                             \
		WriteTimer2(0);                                  \
		timer_servo--;                                   \
		if(timer_servo <= 0){                            \
			if(servo_courant == 0){                      \
				SERVO1=1;                                \
				timer_servo = pos_servo[0] >> 8;         \
				if((pos_servo[servo_courant] & 0x00FF) != 0){      \
					WriteTimer2((unsigned char)(0x100 - (pos_servo[servo_courant] & 0x00FF))); \
				}else{                                   \
					WriteTimer2(0);                      \
					timer_servo--;                       \
				}                                        \
				servo_courant = 1;                       \
			}else if(servo_courant == 1){                \
    			SERVO1=0;                                \
				SERVO2=1;                                \
				timer_servo = pos_servo[1] >> 8;         \
				if((pos_servo[servo_courant] & 0x00FF) != 0){     \
					WriteTimer2((unsigned char)(0x100 - (pos_servo[servo_courant] & 0x00FF))); \
				}else{                                   \
					WriteTimer2(0);                      \
					timer_servo--;                       \
				}                                        \
				servo_courant = 2;                       \
			}else{                                       \
				servo_courant = 0;                       \
				SERVO2=0;                                \
				timer_servo = 25;                        \
			}                                            \
		}                                                \
	}                                                    \
}
