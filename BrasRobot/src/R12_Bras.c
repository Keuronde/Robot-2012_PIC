#include <p18cxxx.h>
#include <timers.h>
#include <delays.h>
#include <pwm.h>
#include "../include/config.h"
#include "../include/servo.h"
#include "../include/moteurs.h"
#include "../include/temps.h"
#include "../include/i2c_s.h"
#include "../../Interfaces/interfaceBrasLingot.h"

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
// TEMPO (en centisecondes)
#define TEMPO_SERVO_CS (unsigned char) 40
// 50536 = 0xC568

/** V A R I A B L E S ********************************************************/
#pragma udata
volatile unsigned char timer_led;
volatile unsigned char timer_emi;


enum etat_cre_t {
/* Attraper le lingo */
    CRE_RENTREE=0,
    CRE_AVANCE,
    CRE_SORTIE,
    CRE_RECULE
};


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
	Servo_Int();


}

#pragma interrupt MyInterrupt_L
void MyInterrupt_L(void){
	
	if(PIR2bits.TMR3IF){
		PIR2bits.TMR3IF = 0; // On réarme le Timer3
		//WriteTimer3(TIMER_INIT);
		TMR3H = TIMER_H;
		TMR3L = TIMER_L;
		centisecondes++;
	}

}


#pragma code




void main(void){
	enum etat_bras_t e_bras_gauche=REPLIE;
	enum etat_bras_t e_bras_droit =REPLIE;
	union message_bras_t message;
	char i=0;
	unsigned char delai_sg = 0,delai_sd = 0;
	unsigned char recu[NB_STRATEGIE_2_BRAS];
	unsigned int temps, temps_old;
    Init();
    init_i2c(0x33);
    message.MESSAGE = 0;
    M1_ENABLE = 1;
    M1_SENS = 1;
    
    M2_ENABLE = 1;
    M2_SENS = 1;
    
  
  

	Servo_Set(DOIGT_G_FERME);
	Servo_Set(DOIGT_D_FERME);
	temps = getTemps_s();
	temps_old = temps;
    while(1){
		// Gestion de l'I2C
		com_i2c();
		
		// Reception I2C - Gestion des action
		if(rec_i2c((unsigned char *) &message) == 1){
			if(message.COMMANDE_BRAS == CDE_BRAS_OUVERT){ // Ouvre les doigts
				Servo_Set(DOIGT_G_OUVERT);
				Servo_Set(DOIGT_D_OUVERT);
				delai_sg = TEMPO_SERVO_CS;
				delai_sd = TEMPO_SERVO_CS;
			}
			if(message.COMMANDE_BRAS == CDE_BRAS_ATTRAPE){ // Attrappe les lingos
				e_bras_gauche = OUVRE_DOIGT;
				e_bras_droit = OUVRE_DOIGT;
			}
			if(message.COMMANDE_BRAS == CDE_BRAS_DEPOSE){ // Dépose les lingots
				Servo_Set(DOIGT_G_OUVERT);
				Servo_Set(DOIGT_D_OUVERT);
				e_bras_gauche = ROUVRE_DOIGT;
				e_bras_droit = ROUVRE_DOIGT;
				delai_sg = TEMPO_SERVO_CS;
				delai_sd = TEMPO_SERVO_CS;
			}
		}
		
		// On récupère l'heure 
		temps = getTemps_cs();
		if(temps != temps_old){
			temps_old = temps;
			
			// gestion des délais
			if(	delai_sg > 0){
				delai_sg--;
			}
			if(	delai_sd > 0){
				delai_sd--;
			}
		}
		
		if(PORTBbits.RB4 == 0){
			e_bras_gauche = OUVRE_DOIGT;
			e_bras_droit = OUVRE_DOIGT;
			Servo_Set(DOIGT_G_OUVERT);
			Servo_Set(DOIGT_D_OUVERT);
			delai_sg = TEMPO_SERVO_CS;
			delai_sd = TEMPO_SERVO_CS;
		}
		
		
		//Machine à état de gestion des bras
		// Bras Droit
		switch (e_bras_droit){
			case REPLIE:
				if (CT_M2_AR == 1){
					M2_Avance();
				}else{
					M2_Stop();
				}
				break;
			// Attraper le lingo 
			case OUVRE_DOIGT:
				if (delai_sd == 0){
					e_bras_droit = AVANCE_BRAS;
				}
				break;
			case AVANCE_BRAS:
				M2_Avance();
				if (CT_M2_AV == 0){
					e_bras_droit = FERME_DOIGT;
					delai_sd = TEMPO_SERVO_CS;
					Servo_Set(DOIGT_D_TIRE);
				}
				break;
			case FERME_DOIGT:
				if (CT_M2_AV == 0){
					M2_Recule();
				}else{
					M2_Stop();
				}
				if(delai_sd == 0){
					e_bras_droit = RECULE_BRAS;
				}
				break;
			case RECULE_BRAS:
				
				M2_Recule();
				if (CT_M2_AR == 1){
					M2_Stop();
					e_bras_droit = RENTRE_LINGOT;
					Servo_Set(DOIGT_D_RABAT);
				}
				break;
			case RENTRE_LINGOT:
				e_bras_droit = REPLIE;
				break;
			// Deposer le lingo 
			case ROUVRE_DOIGT:	
				if (delai_sd == 0){
					e_bras_droit = POUSSE_LINGOT;
				}			
				break;
			case POUSSE_LINGOT:
				M2_Avance();
				if (CT_M2_AV == 0){
					e_bras_droit = RENTRE_BRAS;
				}
				break;
			case RENTRE_BRAS:
				M2_Recule();
				if (CT_M2_AR == 1){
					e_bras_droit = REPLIE;
					Servo_Set(DOIGT_D_FERME);
				}
				break;
			default:
				break;
		}
		
		
		// Bras Gauche
		switch (e_bras_gauche){
			case REPLIE:
				if (CT_M1_AR == 1){
					M1_Avance();
				}else{
					M1_Stop();
				}
				break;
			// Attraper le lingo 
			case OUVRE_DOIGT:
				if (delai_sg == 0){
					e_bras_gauche = AVANCE_BRAS;
				}
				break;
			case AVANCE_BRAS:
				M1_Avance();
				if (CT_M1_AV == 0){
					e_bras_gauche = FERME_DOIGT;
					delai_sg = TEMPO_SERVO_CS;
					Servo_Set(DOIGT_G_TIRE);
				}
				break;
			case FERME_DOIGT:
				if (CT_M1_AV == 0){
					M1_Recule();
				}else{
					M1_Stop();
				}
				if(delai_sg == 0){
					e_bras_gauche = RECULE_BRAS;
				}
				break;
			case RECULE_BRAS:
				
				M1_Recule();
				if (CT_M1_AR == 1){
					M1_Stop();
					e_bras_gauche = RENTRE_LINGOT;
					Servo_Set(DOIGT_G_RABAT);
				}
				break;
			case RENTRE_LINGOT:
				e_bras_gauche = REPLIE;
				break;
			// Deposer le lingo 
			case ROUVRE_DOIGT:	
				if (delai_sg == 0){
					e_bras_gauche = POUSSE_LINGOT;
				}			
				break;
			case POUSSE_LINGOT:
				M1_Avance();
				if (CT_M1_AV == 0){
					e_bras_gauche = RENTRE_BRAS;
				}
				break;
			case RENTRE_BRAS:
				M1_Recule();
				if (CT_M1_AR == 1){
					e_bras_gauche = REPLIE;
					Servo_Set(DOIGT_G_FERME);
				}
				break;
			default:
				break;
		}

		
    }

}

void Init(){
	// Toutes les pattes en digital (pas d'analogique)
    ADCON1 |= 0x0F;
    // Contacteur en sortie (pour le test)
        
	TRISA = 0xF0;
	
	TRIS_CT1 = 1;
	TRIS_CT2 = 1;
	TRIS_CT3 = 1;
	TRIS_CT8 = 0;
	TRIS_CT9 = 0;
	TRIS_CT10 = 0;
	
	// Contacteurs crémaillère
	TRIS_CT_M1_AV = 1;
	TRIS_CT_M1_AR = 1;
	TRIS_CT_M2_AV = 1;
	TRIS_CT_M2_AR = 1;
	
	CT10 = 1;
	Delay10KTCYx(0);
	Delay10KTCYx(0);

	CT10 = 0;
	Delay10KTCYx(0);
	Delay10KTCYx(0);

	
	
	
	TRIB_BOUTON = 1;
	


	Servo_Init();
	Moteurs_Init();
	Temps_Init();
	
	// On active toutes les interruptions
	INTCONbits.GIE = 1;
	INTCONbits.PEIE = 1;
	// On active les priorité d'interruption
	RCONbits.IPEN = 1;
	
	T1CONbits.RD16 = 1;

	
}



