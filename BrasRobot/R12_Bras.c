#include <p18cxxx.h>
#include <timers.h>
#include <delays.h>
#include <pwm.h>
#include "config.h"
#include "servo/servo.h"
#include "moteurs/moteurs.h"
#include "temps/temps.h"

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
#define TEMPO_SERVO_CS (unsigned int) 50
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

enum etat_bras_t {
/* Attraper le lingo */
    REPLIE=0,
    OUVRE_DOIGT,
    AVANCE_BRAS,
    FERME_DOIGT,
    RECULE_BRAS,
    RENTRE_LINGOT,
/* Deposer le lingo */
	ROUVRE_DOIGT,
	POUSSE_LINGOT,
	RENTRE_BRAS
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
	
	Temps_Int();

}


#pragma code




void main(void){
	enum etat_bras_t e_bras_gauche=REPLIE;
	enum etat_bras_t e_bras_droit =REPLIE;
	char i=0;
	unsigned int temps_cs, tempo_cs;
    Init();
    

    M1_ENABLE = 1;
    M1_SENS = 1;
    
    M2_ENABLE = 1;
    M2_SENS = 1;
    
  
  
	// Test algo bras gauche
	e_bras_gauche = OUVRE_DOIGT;
	//Servo_Set(DOIGT_G_OUVERT);
    while(1){
		// On récupère l'heure 
		if(temps_cs != getTemps_s()){
			temps_cs = getTemps_s();
			i++;
			if (i>4){
				i=1;
			}
		}
		
		switch (i){
			case 1:
				CT7  =0;
				CT10 =1;
				break;
			case 2:
				CT10 = 0;
				CT9  = 1;
				break;
			case 3:
				CT9 = 0;
				CT8 = 1;
				break;
			case 4:
				CT8 =0;
				CT7 =1;
				break;
			default:
				break;
		}
		
		//Machine à état de gestion des bras
		/*
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
				e_bras_gauche = AVANCE_BRAS;
				break;
			case AVANCE_BRAS:
				M1_Avance();
				if (CT_M1_AV == 0){
					e_bras_gauche = FERME_DOIGT;
				}
				break;
			case FERME_DOIGT:
				if (CT_M1_AV == 0){
					M1_Recule();
				}else{
					M1_Stop();
				}
				e_bras_gauche = RECULE_BRAS;
				break;
			case RECULE_BRAS:
				
				M1_Recule();
				if (CT_M1_AR == 1){
					M1_Stop();
					e_bras_gauche = RENTRE_LINGOT;
				}
				break;
			case RENTRE_LINGOT:
				e_bras_gauche = REPLIE;
				break;
			// Deposer le lingo 
			case ROUVRE_DOIGT:
				break;
			case POUSSE_LINGOT:
				break;
			case RENTRE_BRAS:
				break;
			default:
				break;
		}

		*/
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
	TRIS_CT7 = 0;
	TRIS_CT8 = 0;
	TRIS_CT9 = 0;
	TRIS_CT10 = 0;
	
	// Contacteurs crémaillère
	TRIS_CT_M1_AV = 1;
	TRIS_CT_M1_AR = 1;
	
	CT10 = 1;
	Delay10KTCYx(0);
	Delay10KTCYx(0);

	CT10 = 0;
	Delay10KTCYx(0);
	Delay10KTCYx(0);

	
	
	
	TRIB_BOUTON = 1;
	OpenTimer2(TIMER_INT_OFF & T2_PS_1_4 & T2_POST_1_1);
	


	Servo_Init();
	Moteurs_Init();
	Temps_Init();

	
}



