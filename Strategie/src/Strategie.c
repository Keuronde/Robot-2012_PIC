#include <p18cxxx.h>
#include <delays.h>
#include <timers.h>
#include "../include/asservissement.h"
#include "../include/carte_strategie.h"
#include "../include/i2c_m.h"
#include "../include/i2c_moteurs.h"
#include "../include/i2c_servo.h"
#include "../include/WMP.h"
#include "../include/WCC.h"
#include "../include/cmucam.h"

/** T Y P E   P R I V E S ****************************************************/

enum etat_strategie_t {
    INIT=0,
    SORTIR_CASE,
    VERS_CD_1,
    ATTRAPE_CD_1,
    ATTRAPE_CD_2,
    EVITEMENT_RECULE,
    TEST_SERVO_1,
    TEST_SERVO_2_1,
    TEST_SERVO_2_2,
    TEST_SERVO_2_3,
    TEST_LECTURE_MOTEUR_1,
    TEST_ACQUITTEMENT_1,
    TEST_ACQUITTEMENT_2,
    TEST_ACQUITTEMENT_3,
    TEST_SERVO_CMUCAM_1,
    TEST_SERVO_CMUCAM_2,
    TEST_SERVO_CMUCAM_3,
    TEST_SERVO_CMUCAM_4,
    TEST_ABSENCE_PION,
    TEST_ABSENCE_PION_INIT,
    TEST_ABSENCE_PION_1,
    TEST_ABSENCE_PION_2,
    TEST_ABSENCE_PION_3,
    TEST_ABSENCE_PION_4,
    TEST_DROIT_1,
    TEST_DROIT_2,
    TEST_LIAISON,
	TEST_RELAIS_1,
	TEST_RELAIS_2,
	TEST_RELAIS_3,
	TEST_RELAIS_4,
	TEST_SONIC,
	TEST_PIED_1,
	TEST_PIED_2,
	TEST_PIED_3,
	TEST_PAS_1,
	TEST_PAS_2,
	TEST_PAS_3,
	TEST_PAS_4,
	TEST_PAS_5,
	TEST_PAS_6,
	TEST_PAS_7,
	TEST_PAS_8,
	TEST_CMUCAM_1,
	TEST_CMUCAM_2,
	TEST_ASSER_1,
	TEST_ASSER_2,
	TEST_ASSER_3,
	TEST_ALLERRETOUR_1,
	TEST_ALLERRETOUR_2,
	TEST_ALLERRETOUR_3,
	TEST_ALLERRETOUR_4
};



enum etat_poussoirs_t {
	INIT=0,
	BAISSES,
	LEVES_AVANT_PION,
	LEVES_SUR_PION,
	LEVES_APRES_PION,
};



/** V A R I A B L E S ********************************************************/
// Non initialisées
#pragma udata
volatile unsigned char timer; 
volatile int nb3ms; 
unsigned char mTimer;
long consigne_angle; // Pour l'asservissement

// Initialisées
#pragma idata




/** P R I V A T E  P R O T O T Y P E S ***************************************/
void MyInterrupt(void);
void Init(void);
char getTimer(void);






#define MOTEUR 1
#define SERVO 2
#define ROUGE 3
#define BLEU 1

#define DEBUG_SONIC 0
#define DEBUG_CC_GAUCHE 0
#define DEBUG_CC_DROIT 0
#define DEBUG_CC_AVANT 0
#define DEBUG_CC_DEUX 0
#define DEBUG_WMP 0


#define PION_1_XMIN (unsigned int)52
#define PION_1_XMAX (unsigned int)180
#define PION_1_YMIN (unsigned int)132
#define PION_1_YMAX (unsigned int)234
#define PION_1_XMILIEU 116
#define PION_1_YMILIEU 183

#define PION_2_XMIN (unsigned int)88
#define PION_2_XMAX (unsigned int)194
#define PION_2_YMIN (unsigned int)242
#define PION_2_YMAX (unsigned int)284



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
void MyInterrupt(void){
	// code de "Rustre Corner"
	// Adapté et modifié par S. KAY
	unsigned char sauv1;
	unsigned char sauv2;

	sauv1 = PRODL;
	sauv2 = PRODH;
	
	// Compteur de temps : 1 incrément toutes les 3 ms
   	if(INTCONbits.TMR0IF == 1){
		INTCONbits.TMR0IF = 0;
//		WriteTimer0(65535 - 36000); //pour un préscaler de 1 : 12000 = 1ms
									//pour un préscaler de 32 : 375 = 1 ms
		WriteTimer0(65535 - 48000); //pour un préscaler de 1 : 12000 = 1ms
		timer++;
		
		if(timer == 255){
		    nb3ms--; 
		}
		
	}
	
	gestion_i2c();
	CMUcam_int();

	PRODL = sauv1;
	PRODH = sauv2;		

}


#pragma code



void main(void){
    
    char etat =0;
    
    
    char tempo_p=0;
    
    
    char sens =1;
    unsigned char id_pion_1=255;
    unsigned char id_pion_2=255;
    long _5degres = 100000;
    long _3degres =  60000;
    long angle;
    int pos_pap_offset=0;
    int pos_pap_offset_new=0;
    
    
    char DernierEnvoi=0;
	char couleur;
	char essai_cmucam;
	
	char ignore_pion = 0;
	
	
	char a_lacher=0;
	char evitement =0;
	char tempo_e=20,tempo_relay=0;

    
    
	
    
    enum etat_poussoirs_t etat_poussoirs=INIT;
    enum etat_strategie_t etat_strategie=INIT, old_etat_strategie;
//    enum etat_strategie_t etat_strategie=PARTIR_CASE_1, old_etat_strategie;
    
    
    
    

    Init();
    RELAIS =0;

	while(1){
	    char timer;
	    int tempo_s;
	    char i,j;

	    while(mTimer == getTimer());
        // Calculer et récupérer l'angle du gyroscope (A22)
        mTimer =getTimer(); // On récupère le numéro (0 à 255) du pas de temps pour le gyroscope
        
        WMP_calcul(mTimer); // On actualise l'angle
        angle = WMP_get_Angle(); // Récupérer l'angle du gyrosocpe
		
        /**************************
        *                         *
        * Gestion de la stratégie *
        *                         *
        **************************/
        GetDonneesServo();
        
        switch (etat_strategie){
        	case INIT :
				active_asser(ASSER_AVANCE,0,&consigne_angle);
				tempo_s = 250;
				etat_strategie = SORTIR_CASE;
				break;
			case SORTIR_CASE:
				tempo_s--;
				if(tempo_s == 0){
					active_asser(ASSER_TOURNE,900000,&consigne_angle);
					etat_strategie = ATTRAPE_CD_1;
				}
				break;
			case ATTRAPE_CD_1 :
				if (fin_asser()){
					setCouleur('W');
					CMUcam_active();
					etat_strategie = ATTRAPE_CD_2;
				}
				break;
			case ATTRAPE_CD_2:
				if ((CMUcam_get_Etat() == TRACKING) || (CMUcam_get_Etat() == TRACKING_PROCHE)){
					LED_ROUGE =1;
					LED_BLEUE =1;
					active_asser(ASSER_TOURNE,0,&consigne_angle);
					etat_strategie =TEST_SERVO_1;
				}
                break;
            case TEST_SERVO_1:
				if( fin_asser() ) {
					CMUcam_attrapeCDGauche();
					active_asser(ASSER_TOURNE,0,&consigne_angle);
					etat_strategie =TEST_SERVO_2_1;
				}
				break;
            case TEST_SERVO_2_1 :
                /*LED_ROUGE =1;
                SetServoPArG(HAUT);
                SetServoPArD(HAUT);
                SetServoPAv(HAUT);
				RELAIS=1;
                etat_strategie = TEST_SERVO_2_2;
                tempo_s = 0;*/
                break;
            case TEST_SERVO_2_2 :
                LED_BLEUE =1;
                tempo_s++;
                if(tempo_s > 1000){
                    SetServoPArG(BAS);
                    SetServoPArD(BAS);
                    SetServoPAv(BAS);
					LED_CMUCAM=0;
                    etat_strategie = TEST_SERVO_2_3;
                } 
                break;
            case TEST_SERVO_2_3 :
                LED_ROUGE =0;
                break;
            case TEST_LECTURE_MOTEUR_1:
                break;
            case TEST_ACQUITTEMENT_1:
                Avance();
                etat_strategie = TEST_ACQUITTEMENT_2;
                LED_OK1=1;
                break;    
            case TEST_ACQUITTEMENT_2:
                LED_OK1=0;
                LED_OK=1;
                GetDonneesMoteurs();
                if(get_CT_AV_G()){
                    ignore_contacteur();
                    etat_strategie = TEST_ACQUITTEMENT_3;
                }
                break;
            case TEST_ACQUITTEMENT_3:
                LED_OK1=1;
                LED_OK=1;
                break;
            case TEST_SERVO_CMUCAM_1:
                SetServoCMUcam(1);
                tempo_s=0;
                etat_strategie = TEST_SERVO_CMUCAM_2;
                break;
            case TEST_SERVO_CMUCAM_2:
                tempo_s++;
                if(tempo_s > 300){
                    etat_strategie = TEST_SERVO_CMUCAM_3;
                    tempo_s=0;
                }
                break;
            case TEST_SERVO_CMUCAM_3:
                SetServoCMUcam(0);
                tempo_s=0;
                etat_strategie = TEST_SERVO_CMUCAM_4;
                break;
            case TEST_SERVO_CMUCAM_4:
                tempo_s++;
                if(tempo_s > 300){
                    etat_strategie = TEST_SERVO_CMUCAM_1;
                    tempo_s=0;
                }
                break;
            case TEST_DROIT_1:
				active_asser(ASSER_AVANCE,0,&consigne_angle);
                etat_strategie = TEST_DROIT_2;
                break;
            case TEST_DROIT_2:
                break;
            case TEST_LIAISON:
                GetDonneesServo();
                GetDonneesMoteurs();
                SetServoPAv(BAS);
                break;
			case TEST_SONIC:
				// Si on a recu qqch de la carte moteur
				if(get_capteur_sonique_proche()){
				    LED_OK =1;
				}else{
				    LED_OK =0;
				}
				
				if(get_capteur_sonique_loin()){
				    LED_OK1 =1;
				}else{
				    LED_OK1 =0;
				}
				break;
			case TEST_PIED_1:
				RELAIS=1;
				tempo_s++;
                if(tempo_s > 1000){
                    etat_strategie = TEST_PIED_2;
					SetCremaillere(BAS);
                    tempo_s=0;
                }
				break;
			case TEST_PIED_2:
				tempo_s++;
				if(tempo_s > 1000){
                    etat_strategie = TEST_PIED_1;
					SetCremaillere(HAUT);
                    tempo_s=0;
                }
				break;break;
			case TEST_PAS_1:
				desactive_asser();
				pap_set_pos(180);
				tempo_s = 0;
				etat_strategie = TEST_PAS_2;
				break;
			case TEST_PAS_2:
				tempo_s++;
				pap_set_pos(180);
				if(tempo_s > 500){
					tempo_s=0;
					etat_strategie = TEST_PAS_3;
				}
				break;
			case TEST_PAS_3:
				pap_set_pos(0);
				GetDonneesServo();
				etat_strategie = TEST_PAS_4;
				tempo_s = 0;
				break;
			case TEST_PAS_4:
				tempo_s++;
				pap_set_pos(0);
				if(tempo_s > 500){
					tempo_s=0;
					etat_strategie = TEST_PAS_1;
				}
				break;
				
			case TEST_PAS_5:
				active_asser(AVANCE_DROIT,0,&consigne_angle);
				tempo_s=0;
				etat_strategie = TEST_PAS_6;
				break;
			case TEST_PAS_6:
				tempo_s++;
				if(tempo_s > 1000){
					tempo_s=0;
					desactive_asser();
					etat_strategie = TEST_PAS_7;
				}
				break;
			case TEST_PAS_7:
				pap_set_pos(115); // Rotation par rapport au centre
//				pap_set_pos((int)-285); // Rotation par rapport au centre
				etat_strategie = TEST_PAS_8;
				break;
				
			case TEST_CMUCAM_1:
				CMUcam_active();
				etat_strategie = TEST_CMUCAM_2;
				break;
			case TEST_ABSENCE_PION:
				LED_OK=ABSENCE_PION;
				LED_OK1=ABSENCE_PION;
				LED_CMUCAM=ABSENCE_PION;
				break;
			case TEST_ABSENCE_PION_INIT:
				SetServoPArD(BAS);
				SetServoPArG(BAS);
				SetServoPAv(HAUT);
				etat_strategie = TEST_ABSENCE_PION_1;
				break;
			case TEST_ABSENCE_PION_1:
				if(! ABSENCE_PION){
					SetServoPAv(BAS);
					etat_strategie = TEST_ABSENCE_PION_2;
				}
				break;
			case TEST_ABSENCE_PION_2:
				if(! BOOT){
					SetServoPArD(HAUT);
					SetServoPArG(HAUT);
					SetServoPAv(HAUT);
					etat_strategie = TEST_ABSENCE_PION_3;
				}
				break;
			case TEST_ABSENCE_PION_3:
				if(ABSENCE_PION){
					tempo_s = 0;
					etat_strategie = TEST_ABSENCE_PION_4;
				}
				break;
			case TEST_ABSENCE_PION_4:
				tempo_s++;
				if(tempo_s > 1000){
					tempo_s = 0;
					etat_strategie = TEST_ABSENCE_PION_INIT;
				}
				break;
			case TEST_ASSER_1:
				// On donne une consigne de 90°
				active_asser(ASSER_TOURNE,1800000,&consigne_angle);
				/*consigne_angle = (long)-1800000; // -90 * 20000
				etat_asser = TOURNE_INIT;
				asser_actif = 1;*/
				etat_strategie = TEST_ASSER_2;
				break;
			case TEST_ASSER_2:
				if(fin_asser()){
					active_asser(ASSER_TOURNE,-1800000,&consigne_angle);
					etat_strategie = TEST_ASSER_3;
				}
				break;
			case TEST_ASSER_3:
				if(fin_asser()){
					active_asser(ASSER_TOURNE,1800000,&consigne_angle);
					etat_strategie = TEST_ASSER_2;
				}
				break;
			case TEST_ALLERRETOUR_1:
				if(fin_asser()){
					active_asser(ASSER_AVANCE,0,&consigne_angle);
					tempo_s = 0;
					etat_strategie = TEST_ALLERRETOUR_2;
				}
				break;
			case TEST_ALLERRETOUR_2:
				tempo_s++;
				if(tempo_s > 750){
					active_asser(ASSER_TOURNE,3600000,&consigne_angle);
					etat_strategie = TEST_ALLERRETOUR_3;
				}
				break;
			case TEST_ALLERRETOUR_3:
				if(fin_asser()){
					active_asser(ASSER_AVANCE,3600000,&consigne_angle);
					etat_strategie = TEST_ALLERRETOUR_4;
					tempo_s=0;
				}
				break;
            case TEST_ALLERRETOUR_4:
				tempo_s++;
				if(tempo_s>750){
					active_asser(ASSER_TOURNE,0,&consigne_angle);
					etat_strategie = TEST_ALLERRETOUR_1;
				}
				break;
            default:
                break;
        }

        
        /***********************
        *                      *
        *  Affichages / DEBUG  *
        *                      *
        ***********************/
		{
			if(DEBUG_SONIC){
				if(get_capteur_sonique_proche()){
					LED_OK =1;
				}else{
					LED_OK =0;
				}
				
				if(get_capteur_sonique_loin()){
					LED_OK1 =1;
				}else{
					LED_OK1 =0;
				}
			}

			if(DEBUG_CC_DROIT){
				if(get_CC_Droit() == 1){
					LED_BLEUE=1;
				}else{
					LED_BLEUE=0;
				}
				if(get_CC_Droit() == 3){
					LED_ROUGE=1;
				}else{
					LED_ROUGE=0;
				}
			}


			if(DEBUG_CC_GAUCHE){
				if(get_CC_Gauche() == 1){
					LED_BLEUE=1;
				}else{
					LED_BLEUE=0;
				}
				if(get_CC_Gauche() == 3){
					LED_ROUGE=1;
				}else{
					LED_ROUGE=0;
				}
			}
			if(DEBUG_CC_AVANT){
				if(get_CC_Avant() == 1){
					LED_BLEUE=1;
				}else{
					LED_BLEUE=0;
				}
				if(get_CC_Avant() == 3){
					LED_ROUGE=1;
				}else{
					LED_ROUGE=0;
				}
			}
			if(DEBUG_CC_DEUX){
				if(get_CC_Gauche() == BLEU){
					LED_BLEUE=1;
				}else{
					LED_BLEUE=0;
				}
				if(get_CC_Droit() == BLEU){
					LED_ROUGE=1;
				}else{
					LED_ROUGE=0;
				}
			}

			if(DEBUG_WMP){
				if((angle <= _3degres) && (angle >= (0 - _3degres))){
					LED_OK1 = 1;
				}else{
					LED_OK1 = 0;
				}
			}


			if(evitement ==1){
				tempo_e--;
				if(tempo_e == 0){
					GetDonneesMoteurs();
					tempo_e=25;
				}
				if(get_CT_AV_G() || get_CT_AV_D() || get_capteur_sonique_proche()){
					
						old_etat_strategie = etat_strategie;
						etat_strategie = EVITEMENT_RECULE;
						evitement=0;
					
				}
			}
		}
        
        /***********************
        *                      *
        * Gestion de la CMUcam *
        *                      *
        ***********************/

		CMUcam_gestion(&consigne_angle,&angle);
        
        
        /******************************
        *                             *
        * Gestion de l'asservissement *
        *                             *
        ******************************/
        
        Asser_gestion(&consigne_angle,&angle);
        


        /*****************************
        *                            *
        * Gestion des communications *
        *                            *
        *****************************/



        if( GetEnvoiMoteurs() && DernierEnvoi != MOTEUR  ){
	        // Echange avec la carte des servos si nécessaire
            transmission_moteur();
            DernierEnvoi = MOTEUR;
        }else{
			// Echange avec la carte moteurs si nécessaire
            transmission_servo();
            DernierEnvoi = SERVO;
        }


        

	}// Fin While
	LED_OK=0;
	LED_OK1=0;
	LED_CMUCAM=0;
	LED_ROUGE=0;
	LED_BLEUE=0;
	RELAIS=0;
	prop_stop();
	transmission_moteur();
	while(1);
} // Fin main



void Init(){

    init_io();
    init_i2c(); // Active les interruptions
    
    // Séquence visuelle (A11)
    Delay10KTCYx(0);
    LED_BLEUE = 1;
    Delay10KTCYx(0);
    LED_BLEUE = 0;
    LED_ROUGE = 1;
    Delay10KTCYx(0);
    LED_ROUGE = 0;
    LED_CMUCAM = 1;
    Delay10KTCYx(0);
    LED_CMUCAM = 0;
    LED_OK = 1;
    Delay10KTCYx(0);
    LED_OK = 0;
    LED_OK1 = 1;
    Delay10KTCYx(0);
    LED_OK1 = 0;
    
	LED_ROUGE=1;
	
    LED_OK1 = 0;

    CMUcam_Init();


    
    // WMP
	if(WMP_init()){
        LED_OK = 1;
    }else{
        LED_OK = 0;
    }
    Delay10KTCYx(0);
    Delay10KTCYx(0);
    
    // Notre compteur qui va nous rythmer notre robot sur un cycle de 3 ms.
    // Les interruptions doivent être activées pour que le compteur foncitonne "tout seul"
	// Initialisation du Timer 0
	OpenTimer0(	TIMER_INT_ON &  // interruption ON
				T0_16BIT &		// Timer 0 en 16 bits
				T0_SOURCE_INT & // Source interne (Quartz + PLL)
				T0_PS_1_1);		// 1 cycle, 1 incrémentation
	// Et hop, le timer est lancé
	WriteTimer0(65535 - 48000); // dt * 4ms
    

    // SUITE WMP
    LED_OK1 = 0;
    mTimer = getTimer();
    while(WMP_calibration()){           // Tant que la calibration est en cours
        while(mTimer == getTimer());
        mTimer = getTimer();
    }

    // A décommenter pour avoir un WMP stable
	WMP_init_2();
   
    while(WMP_calibration()){           // Tant que la calibration est en cours
        while(mTimer == getTimer());
        mTimer = getTimer();
    }
    
    LED_OK1 = 0;
    LED_OK=0;
	LED_ROUGE=0;
	LED_CMUCAM=1;
	

    //while(TIRETTE);
    LED_OK1 = 1;
    LED_OK=1;
    nb3ms=80; //80 -> environ 82 secondes en réalité, et 8 secondes de marge au cas où on tire la tirette avant l'initialisation complète

	pap_set_pos(0);
	transmission_moteur();
	Delay10KTCYx(0);
	Delay10KTCYx(0);
	Delay10KTCYx(0);
	SetCremaillere(HAUT);
    
   	WMP_init_timer(getTimer());
	mTimer = getTimer();
}



char getTimer(void){
    return timer;
}
