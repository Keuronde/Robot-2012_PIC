#include <p18cxxx.h>
#include <delays.h>
#include <timers.h>
#include "../Interfaces/BrasCD.h"
#include "../include/carte_servo.h"
#include "../include/servo.h"
#include "../include/temps.h"
#include "../include/i2c_s.h"


/** D E F I N E **************************************************************/
// Tempo en centiseconde
#define TEMPO_FERME_DOIGT 100
#define TEMPO_LEVE_BRAS 100
#define TEMPO_DEPOSE_BRAS 100
#define TEMPO_LACHE_PIECE 100
#define TEMPO_ATTRAPE_PIECE 10
#define TEMPO_CALE_PIECE 100
#define TEMPO_V1   50
#define TEMPO_V2   100

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

}
#pragma code


#pragma interrupt MyInterrupt 
void MyInterrupt(void)
{

	if (PIR1bits.TMR2IF)
	{
		// On réarme le timer
		PIR1bits.TMR2IF = 0;
		TMR2 =(0);
		timer_servo--;
		if(timer_servo <= 0){
			if(servo_courant == 0){
				SERVO1=1;
				timer_servo = pos_servo[0] >> 8; //4
				if((pos_servo[servo_courant] & 0x00FF) != 0){
					TMR2 = (unsigned char)(0x100 - (pos_servo[servo_courant] & 0x00FF));
				}else{
					TMR2 = 0;
					timer_servo--;
				}
				
				servo_courant++;
			}else if(servo_courant == 1){
				SERVO1=0;
				SERVO2=1;
				timer_servo = pos_servo[servo_courant] >> 8;
				if((pos_servo[servo_courant] & 0x00FF) != 0){
					TMR2 = (unsigned char)(0x100 - (pos_servo[servo_courant] & 0x00FF));
				}else{
					TMR2 = 0;
					timer_servo--;
				}
				
				servo_courant++;
			}else if(servo_courant == 2){
				SERVO2=0;
				SERVO3=1;
				timer_servo = pos_servo[servo_courant] >> 8;
				if((pos_servo[servo_courant] & 0x00FF) != 0){
					TMR2 = ((unsigned char)(0x100 - (pos_servo[servo_courant] & 0x00FF)));
				}else{
					TMR2 = 0;
					timer_servo--;
				}
				
				servo_courant++;
			}else if(servo_courant == 3){
				SERVO3=0;
				SERVO4=1;
				timer_servo = pos_servo[servo_courant] >> 8;
				if((pos_servo[servo_courant] & 0x00FF) != 0){
					TMR2 = ((unsigned char)(0x100 - (pos_servo[servo_courant] & 0x00FF)));
				}else{
					TMR2 = (0);
					timer_servo--;
				}
				
				servo_courant++;
			}else if(servo_courant == 4){
				SERVO4=0;
				SERVO5=1;
				timer_servo = pos_servo[servo_courant] >> 8;
				if((pos_servo[servo_courant] & 0x00FF) != 0){
					TMR2 =((unsigned char)(0x100 - (pos_servo[servo_courant] & 0x00FF)));
				}else{
					TMR2 =(0);
					timer_servo--;
				}
				
				servo_courant++;
			}else{
				servo_courant = 0;
				SERVO5=0;
				timer_servo = 25;//25 - (pos_servo[servo_courant] >> 8);
			}
		}		
	}

}






#pragma code


void main(void){
	char tempo_bg,tempo_bd,tempo_vitale=0;
	unsigned char recu[10];
	char envoi[NB_ENV];
	unsigned int temps, temps_old;
	
	envoi[0]=3;
	envoi[1]=4;
	// Init :
	Temps_Init();
	Servo_Init();
	Servo_Set(BRAS_BASCULE_G);
	Servo_Set(PINCE_OUVERT_G);
	Servo_Set(PINCE_OUVERT_D);
	Servo_Set(BRAS_BASCULE_D);
	init_i2c(0x33);
	TRIS_BOOT = 1;
	// On active toutes les interruptions
	INTCONbits.GIE = 1;
	INTCONbits.PEIE = 1;
	// On n'active pas les priorité d'interruption
	RCONbits.IPEN = 1;
	
	ADCON1 = 0x0F; // Tout numérique, pas d'analogique
	TRIS_LED_OK = 0;
	TRIS_IS_GAUCHE = 1;
	TRIS_IS_DROIT = 1;
	LED_OK = 1;
	Delay10KTCYx(0);
	Delay10KTCYx(0);
	Delay10KTCYx(0);
	LED_OK = 0;
	Delay10KTCYx(0);
	Delay10KTCYx(0);
	Delay10KTCYx(0);
	LED_OK = 1;
	Delay10KTCYx(0);
	Delay10KTCYx(0);
	Delay10KTCYx(0);
	LED_OK = 0;
	Delay10KTCYx(0);
	Delay10KTCYx(0);
	Delay10KTCYx(0);
	LED_OK = 1;

	etat_bras_gauche = E_BRAS_INIT;
	etat_bras_droit = E_BRAS_INIT;
	tempo_bg = 0;
	tempo_bd = 0;
	temps = getTemps_cs();
	temps_old = temps;
	while(1)
	{
		// Fonction qui étaient en interruption
		// Gestion du temps
		if(PIR2bits.TMR3IF){
			PIR2bits.TMR3IF = 0; // On réarme le Timer3
			TMR3H = TIMER_H;
			TMR3L = TIMER_L;
			centisecondes++;
		}
		// Gestion de l'I2C
		com_i2c();
		// Fin des fonction d'interruption
		
		// Code principal
		temps = centisecondes;
		if(temps != temps_old){
			temps_old = temps;
			
			// gestion des délais
			if( tempo_bg > 0){
				tempo_bg--;
			}
			if( tempo_bd > 0){
				tempo_bd--;
			}
			if(tempo_vitale >0){
				tempo_vitale--;
			} 
		}
		
		// Clignotement
		if(tempo_vitale == 0){
			tempo_vitale=TEMPO_V1;
			LED_OK = !LED_OK;
		}
		
		// Reception I2C - Gestion des action
		if(rec_i2c(recu) == 1){
			if(recu[0] & 0x01){ // Bras Gauche
				switch(etat_bras_gauche){
					case E_BRAS_INIT:
						etat_bras_gauche = E_BRAS_ATTENTE_PIECE;
						break;
					case E_BRAS_ATTENTE_PLEIN:
						etat_bras_gauche = E_BRAS_HAUT_RESSERRE;
						break;
				}
			}
			if(recu[0] & 0x10){ // Bras Gauche
				switch(etat_bras_droit){
					case E_BRAS_INIT:
						etat_bras_droit = E_BRAS_ATTENTE_PIECE;
						break;
					case E_BRAS_ATTENTE_PLEIN:
						etat_bras_droit = E_BRAS_HAUT_RESSERRE;
						break;
				}
			}
		}
		
		// Debug
		if(BOOT == 0){
			etat_bras_gauche = E_BRAS_ATTENTE_PIECE;
			etat_bras_droit = E_BRAS_ATTENTE_PIECE;
		}
		
		
		// Gestion du bras gauche
		switch (etat_bras_gauche){
			case E_BRAS_INIT :
				Servo_Set(BRAS_HAUT_G);
				Servo_Set(PINCE_OUVERT_G);
				tempo_bg=TEMPO_ATTRAPE_PIECE;
				break;
			case E_BRAS_ATTENTE_PIECE:
				Servo_Set(BRAS_BAS_G);
				Servo_Set(PINCE_OUVERT_G);
				etat_bras_gauche=E_BRAS_BAS_OUVERT;
				break;
			case E_BRAS_BAS_OUVERT:
				if(IS_GAUCHE){
					if (tempo_bg == 0){
						etat_bras_gauche = E_BRAS_BAS_FERME;
						Servo_Set(PINCE_FERMEE_G);
						tempo_bg=TEMPO_FERME_DOIGT;
					}
				}else{
					tempo_bg=TEMPO_ATTRAPE_PIECE;
				}
				break;
			case E_BRAS_BAS_FERME:
				if (tempo_bg == 0){
					etat_bras_gauche = E_BRAS_HAUT_FERME;
					Servo_Set(BRAS_HAUT_G);
					tempo_bg=TEMPO_LEVE_BRAS;
				}
				break;
			case E_BRAS_HAUT_FERME:
				if (tempo_bg == 0){
					etat_bras_gauche = E_BRAS_HAUT_LACHE;
					Servo_Set(PINCE_LACHE_G);
					tempo_bg=TEMPO_CALE_PIECE;
				}
				break;
			case E_BRAS_HAUT_LACHE:
				if (tempo_bg == 0){
					etat_bras_gauche = E_BRAS_ATTENTE_PLEIN;
					Servo_Set(PINCE_FERMEE_G);
					tempo_bg=TEMPO_CALE_PIECE;
				}
				break;
			case E_BRAS_ATTENTE_PLEIN :
				break;
			case E_BRAS_HAUT_RESSERRE:
				if (tempo_bg == 0){
					etat_bras_gauche = E_BRAS_DEPOSE_FERME;
					Servo_Set(BRAS_BASCULE_G);
					tempo_bg=TEMPO_DEPOSE_BRAS;
				}
				break;
			
			case E_BRAS_DEPOSE_FERME:
				if (tempo_bg == 0){
					etat_bras_gauche = E_BRAS_DEPOSE_OUVERT;
					Servo_Set(PINCE_OUVERT_G);
					tempo_bg=TEMPO_LACHE_PIECE;
				}
				break;
			case E_BRAS_DEPOSE_OUVERT:
				if (tempo_bg == 0){
					etat_bras_gauche = E_BRAS_INIT;
				}
				break;
			default :
				break;
			
		}
		
		// Gestion du bras droit
		switch (etat_bras_droit){
			case E_BRAS_INIT :
				Servo_Set(BRAS_HAUT_D);
				Servo_Set(PINCE_OUVERT_D);
				tempo_bd=TEMPO_ATTRAPE_PIECE;
				break;
			case E_BRAS_ATTENTE_PIECE:
				Servo_Set(BRAS_BAS_D);
				Servo_Set(PINCE_OUVERT_D);
				etat_bras_droit=E_BRAS_BAS_OUVERT;
				break;
			case E_BRAS_BAS_OUVERT:
				if(IS_DROIT){
					if (tempo_bd == 0){
						etat_bras_droit = E_BRAS_BAS_FERME;
						Servo_Set(PINCE_FERMEE_D);
						tempo_bd=TEMPO_FERME_DOIGT;
					}
				}else{
					tempo_bd=TEMPO_ATTRAPE_PIECE;
				}
				break;
			case E_BRAS_BAS_FERME:
				if (tempo_bd == 0){
					etat_bras_droit = E_BRAS_HAUT_FERME;
					Servo_Set(BRAS_HAUT_D);
					tempo_bd=TEMPO_LEVE_BRAS;
				}
				break;
			case E_BRAS_HAUT_FERME:
				if (tempo_bd == 0){
					etat_bras_droit = E_BRAS_HAUT_LACHE;
					Servo_Set(PINCE_LACHE_D);
					tempo_bd=TEMPO_CALE_PIECE;
				}
				break;
			case E_BRAS_HAUT_LACHE:
				if (tempo_bd == 0){
					etat_bras_droit = E_BRAS_ATTENTE_PLEIN;
					Servo_Set(PINCE_FERMEE_D);
					tempo_bd=TEMPO_CALE_PIECE;
				}
				break;
			case E_BRAS_ATTENTE_PLEIN :
				break;
			case E_BRAS_HAUT_RESSERRE:
				if (tempo_bd == 0){
					etat_bras_droit = E_BRAS_DEPOSE_FERME;
					Servo_Set(BRAS_BASCULE_D);
					tempo_bd=TEMPO_DEPOSE_BRAS;
				}
				break;
			
			case E_BRAS_DEPOSE_FERME:
				if (tempo_bd == 0){
					etat_bras_droit = E_BRAS_DEPOSE_OUVERT;
					Servo_Set(PINCE_OUVERT_D);
					tempo_bd=TEMPO_LACHE_PIECE;
				}
				break;
			case E_BRAS_DEPOSE_OUVERT:
				if (tempo_bd == 0){
					etat_bras_droit = E_BRAS_INIT;
				}
				break;
			default :
				break;
			
		}
		
		
		
		// Préparation de l'envoi I2C
		envoi[0] = etat_bras_gauche;
		envoi[1] = etat_bras_droit;
		envoi[2] = 0;
		if (IS_GAUCHE){
			envoi[2] |= 0x01;
		}
		
		envoi_i2c(envoi);
	
	
	}
  
}
