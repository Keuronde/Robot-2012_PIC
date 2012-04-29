#include <p18cxxx.h>
#include <delays.h>
#include <timers.h>
#include "../Interfaces/BrasCD.h"
#include "../include/asservissement.h"
#include "../include/carte_strategie.h"
#include "../include/i2c_m.h"
#include "../include/i2c_moteurs.h"
#include "../include/i2c_servo.h"
#include "../include/i2c_brasLingot.h"
#include "../include/WMP.h"
#include "../include/WCC.h"
#include "../include/cmucam.h"

/* M A C R O **********************************************************/
#define TEMPO_TOTEM_LOIN 150
#define TEMPO_TOTEM_PROCHE 100

/** T Y P E   P R I V E S ****************************************************/

enum etat_strategie_t {
    INIT=0,
    SORTIR_CASE,
    VERS_CD_1,
    ATTRAPE_CD_1,
    ATTRAPE_CD_2,
    ATTRAPE_CD_2_1,
    ATTRAPE_CD_3,
    ATTRAPE_CD_4,
    ATTRAPE_CD_5,
    VERS_LINGOT1_1,
    VERS_LINGOT1_2,
    VERS_LINGOT1_3,
    VERS_LINGOT1_4,
    VERS_LINGOT1_5,
    VERS_LINGOT1_6,
    VERS_LINGOT1_7,
    DEPOSE_1,
    DEPOSE_2,
    DEPOSE_3,
	DEPOSE_4,
	DEPOSE_5,
	VERS_ILE_NORD_1,
	VERS_ILE_NORD_2,
	VERS_ILE_NORD_3,
	VERS_ILE_NORD_4,
	VERS_ILE_NORD_5,
	VERS_ILE_NORD_6,
	VERS_ILE_NORD_7,
	VERS_ILE_NORD_8,
	VERS_ILE_NORD_9,
	VERS_CD_ILE_1,
	VERS_CD_ILE_2,
	VERS_CD_ILE_3,
	VERS_CD_ILE_4,
	VERS_CD_ILE_5,
	VERS_TOTEM_1,
	VERS_TOTEM_2,
	VERS_TOTEM_3,
	TOTEM_CONTACT_DROIT_1,
	TOTEM_CONTACT_DROIT_2,
	TOTEM_CONTACT_DROIT_3,
	TOTEM_CONTACT_DROIT_4,
	TOTEM_CONTACT_GAUCHE_1,
	TOTEM_CONTACT_GAUCHE_2,
	TOTEM_CONTACT_GAUCHE_3,
	TOTEM_CONTACT_GAUCHE_4,
	TOTEM_ATTRAPPE_LINGOTS_1,
	TOTEM_OUVRE_DOIGTS_1,
	TOTEM_OUVRE_DOIGTS_2,
	TOTEM_ATTRAPPE_LINGOTS_2,
	TOTEM_SORTIE_1,
	TOTEM_SORTIE_2,
    EVITEMENT_RECULE,
    TEST_CD_DROIT_1,
    TEST_CD_DROIT_2,
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

enum etat_action_t {
	ATTRAPE_CD_DROIT_INIT,
	ATTRAPE_CD_DROIT_TRACKING,
	ATTRAPE_CD_DROIT_TOURNE,
	ATTRAPE_CD_DROIT_AVANCE,
	ATTRAPE_CD_DROIT_LEVE,
	ATTRAPE_CD_GAUCHE_INIT,
	ATTRAPE_CD_GAUCHE_TRACKING,
	ATTRAPE_CD_GAUCHE_TOURNE,
	ATTRAPE_CD_GAUCHE_AVANCE,
	ATTRAPE_CD_GAUCHE_LEVE,
	FIN_ACTION
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
char couleur;
// Initialisées
#pragma idata




/** P R I V A T E  P R O T O T Y P E S ***************************************/
void MyInterrupt(void);
void Init(void);
char getTimer(void);
long ANGLE_DEGRES(int X);





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
	char contact_totem_gauche=0;
	char contact_totem_droit=0;
    
    char DernierEnvoi=0;
	char essai_cmucam;
	
	char ignore_pion = 0;
	
	
	char a_lacher=0;
	char evitement =0;
	char tempo_e=20,tempo_relay=0;

    
    
	
    
    enum etat_poussoirs_t etat_poussoirs=INIT;
    enum etat_action_t etat_action=FIN_ACTION;
    enum etat_strategie_t etat_strategie=INIT, old_etat_strategie;
    //enum etat_strategie_t etat_strategie=DEPOSE_5, old_etat_strategie;


    
	Init();
    RELAIS =0;
    // Tant que les capteur_soniques ne son pas prêt
    ignore_sonique_loin();
    ignore_sonique_proche();

	while(1){
	    char timer;
	    int tempo_s,tempo_s2,tempo_totem;
	    int tempo_action,tempo_avance;
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
        //GetDonneesServo();
        
        switch (etat_strategie){
			case TEST_CD_DROIT_1:
				etat_action = ATTRAPE_CD_DROIT_INIT;
				etat_strategie = TEST_CD_DROIT_2;
				break;
			case TEST_CD_DROIT_2:
				if (etat_action == FIN_ACTION) {
					
				}
				break;
        	case INIT :
				active_asser(ASSER_AVANCE,0,&consigne_angle);
				tempo_s = 350;
				etat_strategie = SORTIR_CASE;
				break;
			case SORTIR_CASE:
				tempo_s--;
				if(tempo_s == 0){
					active_asser(ASSER_TOURNE,ANGLE_DEGRES(45),&consigne_angle);
					etat_strategie = ATTRAPE_CD_1;
				}
				break;
			case ATTRAPE_CD_1 :
				if (fin_asser()){
					etat_action = ATTRAPE_CD_GAUCHE_INIT;
					etat_strategie = ATTRAPE_CD_5;	
				}
				break;
			case ATTRAPE_CD_5:
				if (etat_action == FIN_ACTION) {
					etat_strategie = VERS_LINGOT1_1;
				}
				break;
			case VERS_LINGOT1_1:
				tempo_s++;
				if(tempo_s > 250){
					active_asser(ASSER_TOURNE,ANGLE_DEGRES(135),&consigne_angle);
					etat_strategie = VERS_LINGOT1_2;
				}
				break;
			case VERS_LINGOT1_2:
				if (fin_asser()){
					active_asser(ASSER_AVANCE,ANGLE_DEGRES(135),&consigne_angle);
					tempo_s = 200;
					etat_strategie = VERS_LINGOT1_3;
				}
				break;
			case VERS_LINGOT1_3:
				tempo_s--;
				if (tempo_s == 0){
					cherche_lingot();
					CMUcam_active();
					etat_strategie = VERS_LINGOT1_4;
				}
				break;
			case VERS_LINGOT1_4:
				if ((CMUcam_get_Etat() == TRACKING) || (CMUcam_get_Etat() == TRACKING_PROCHE)){
					LED_ROUGE =1;
					LED_BLEUE =1;
					active_asser(ASSER_AVANCE,consigne_angle,&consigne_angle);
					etat_strategie = VERS_LINGOT1_5;
				}
				break;
			case VERS_LINGOT1_5:
				if ( CMUcam_get_Etat() == TRACKING_PROCHE ){
					CMUcam_reset();
					active_asser(ASSER_TOURNE,ANGLE_DEGRES(180),&consigne_angle);
					etat_strategie = VERS_LINGOT1_6;
				}
				break;
			case VERS_LINGOT1_6:
				if ( fin_asser() ){
					active_asser(ASSER_AVANCE,ANGLE_DEGRES(180),&consigne_angle);
					etat_strategie = VERS_LINGOT1_7;
					tempo_s = 250;
				}
				break;
			case VERS_LINGOT1_7:
				tempo_s--;
				if ( tempo_s == 0 ){
					desactive_asser();
					prop_stop();
					etat_strategie = DEPOSE_1;
				}
				break;
			case DEPOSE_1:
				active_asser(ASSER_RECULE,ANGLE_DEGRES(180),&consigne_angle);
				etat_strategie = DEPOSE_2;
				tempo_s = 250;
				break;
			case DEPOSE_2:
				tempo_s--;
				if ( tempo_s == 0 ){
					desactive_asser();
					prop_stop();
					etat_strategie = DEPOSE_3;
				}
				break;
			case DEPOSE_3:
				active_asser(ASSER_TOURNE,ANGLE_DEGRES(0),&consigne_angle);
				etat_strategie = DEPOSE_4;
				break;
			case DEPOSE_4:
				if(fin_asser()){
					SetServoPArG(1);
					etat_strategie = DEPOSE_5;
					LED_BLEUE =1;
				}
				break;
			case DEPOSE_5:
				GetDonneesServo();
				if(get_Etat_Gauche() == E_BRAS_INIT){
					active_asser(ASSER_TOURNE,ANGLE_DEGRES(0),&consigne_angle);
					tempo_s = 50;
					etat_strategie = VERS_ILE_NORD_1;
				}
				break;
			case VERS_ILE_NORD_1:
				if (fin_asser()){
					cherche_lingot();
					CMUcam_active();
					CMUcam_desactive_asser();
					active_asser(ASSER_AVANCE,ANGLE_DEGRES(25),&consigne_angle);
					tempo_s = 500;
					etat_strategie = VERS_ILE_NORD_4;
				}
				break;
			case VERS_ILE_NORD_2:
				tempo_s--;
				if (tempo_s == 0){
					prop_stop();
					desactive_asser();
					cherche_lingot();
					CMUcam_active();
					etat_strategie = VERS_ILE_NORD_3;
				}
				break;
			case VERS_ILE_NORD_3:
				if ((CMUcam_get_Etat() == TRACKING) || (CMUcam_get_Etat() == TRACKING_PROCHE)){
					LED_ROUGE =1;
					LED_BLEUE =1;
					active_asser(ASSER_AVANCE,consigne_angle,&consigne_angle);
					etat_strategie = VERS_ILE_NORD_4;
				}
				break;
			case VERS_ILE_NORD_4:
				if (cmucam_ile_proche()){
					CMUcam_reset();
					active_asser(ASSER_TOURNE,ANGLE_DEGRES(0),&consigne_angle);
					etat_strategie = VERS_CD_ILE_1;
				}
				break;
			case VERS_CD_ILE_1 :
				if (fin_asser()){
					etat_action = ATTRAPE_CD_DROIT_INIT;
					etat_strategie = VERS_CD_ILE_2;
				}
				break;
			case VERS_CD_ILE_2:
				if (etat_action == FIN_ACTION){
					etat_strategie = VERS_CD_ILE_4;
				}
                break;
            case VERS_CD_ILE_3:
				active_asser(ASSER_TOURNE,ANGLE_DEGRES(45),&consigne_angle);
				etat_strategie = VERS_CD_ILE_4;
				break;
			case VERS_CD_ILE_4:
				//if (fin_asser()){
					etat_action = ATTRAPE_CD_GAUCHE_INIT;
					etat_strategie = VERS_CD_ILE_5;
				//}
				break;
			case VERS_CD_ILE_5:
				if (etat_action == FIN_ACTION){
					etat_strategie = VERS_TOTEM_1;
				}
				break;
			case VERS_TOTEM_1:
				active_asser(ASSER_TOURNE,ANGLE_DEGRES(0),&consigne_angle);
				etat_strategie = VERS_TOTEM_2;
				break;
			case VERS_TOTEM_2:
				if(fin_asser()){
					active_asser_lent(ASSER_AVANCE,ANGLE_DEGRES(0),&consigne_angle);
					etat_strategie = VERS_TOTEM_3;
				}
				break;
			case VERS_TOTEM_3:
				GetDonneesMoteurs();
				if (get_CT_AV_D()){
					desactive_asser();
					pap_set_pos(PAP_DROIT);
					tempo_s = 100;
					contact_totem_droit = 1;
					if(contact_totem_droit & contact_totem_gauche){
						tempo_totem = TEMPO_TOTEM_PROCHE;
					}else{
						tempo_totem = TEMPO_TOTEM_LOIN;
					}
					etat_strategie = TOTEM_CONTACT_DROIT_1;
				}
				if (get_CT_AV_G()){
					desactive_asser();
					pap_set_pos(PAP_DROIT);
					tempo_s = 100;
					contact_totem_gauche = 1;
					if(contact_totem_droit & contact_totem_gauche){
						tempo_totem = TEMPO_TOTEM_PROCHE;
					}else{
						tempo_totem = TEMPO_TOTEM_LOIN;
					}
					etat_strategie = TOTEM_CONTACT_GAUCHE_1;
				}
				break;
			case TOTEM_CONTACT_DROIT_1:
				tempo_s--;
				if (tempo_s == 0){
					Avance();
					ignore_contacteur_avant_droit();
					tempo_s = 30;
					etat_strategie = TOTEM_CONTACT_DROIT_2;
				}
				break;
			case TOTEM_CONTACT_DROIT_2:
				GetDonneesMoteurs();
				tempo_s--;
				if (get_CT_AV_D() && get_CT_AV_G()){
					prop_stop();
					WMP_set_Angle(0); // On se recale
					etat_strategie = TOTEM_ATTRAPPE_LINGOTS_1;
				}else if (tempo_s == 0){
					prop_stop();
					pap_set_pos(PAP_MAX_ROT/2);
					tempo_s = 50;
					etat_strategie = TOTEM_CONTACT_DROIT_3;
				}
				break;
			case TOTEM_CONTACT_DROIT_3:
				tempo_s--;
				if (tempo_s == 0){
					Recule();
					tempo_s = tempo_totem;
					etat_strategie = TOTEM_CONTACT_DROIT_4;
				}
				break;
			case TOTEM_CONTACT_DROIT_4:
				tempo_s--;
				if (tempo_s == 0){
					etat_strategie = VERS_TOTEM_1;
				}
				break;
			case TOTEM_CONTACT_GAUCHE_1:
				tempo_s--;
				if (tempo_s == 0){
					Avance();
					ignore_contacteur_avant_gauche();
					tempo_s = 30;
					etat_strategie = TOTEM_CONTACT_GAUCHE_2;
				}
				break;
			case TOTEM_CONTACT_GAUCHE_2:
				GetDonneesMoteurs();
				tempo_s--;
				if (get_CT_AV_D() && get_CT_AV_G()){
					prop_stop();
					WMP_set_Angle(0); // On se recale
					etat_strategie = TOTEM_ATTRAPPE_LINGOTS_1;
				}else if (tempo_s == 0){
					prop_stop();
					pap_set_pos(-(PAP_MAX_ROT/2));
					tempo_s = 50;
					etat_strategie = TOTEM_CONTACT_GAUCHE_3;
				}
				break;
			case TOTEM_CONTACT_GAUCHE_3:
				tempo_s--;
				if (tempo_s == 0){
					Recule();
					tempo_s = tempo_totem;
					etat_strategie = TOTEM_CONTACT_GAUCHE_4;
				}
				break;
			case TOTEM_CONTACT_GAUCHE_4:
				tempo_s--;
				if (tempo_s == 0){
					etat_strategie = VERS_TOTEM_1;
				}
				break;
			case TOTEM_ATTRAPPE_LINGOTS_1:
				active_asser(ASSER_RECULE,ANGLE_DEGRES(0),&consigne_angle);
				tempo_s = 200;
				etat_strategie = TOTEM_OUVRE_DOIGTS_1;
				break;
			case TOTEM_OUVRE_DOIGTS_1:
				tempo_s--;
				if (tempo_s == 0){
					prop_stop();
					desactive_asser();
					lingot_ouvre_doigt();
					etat_strategie = TOTEM_OUVRE_DOIGTS_2;
					tempo_s = 125;
				}
				break;
			case TOTEM_OUVRE_DOIGTS_2:
				tempo_s--;
				if(tempo_s == 0){
					active_asser_lent(ASSER_AVANCE,ANGLE_DEGRES(0),&consigne_angle);
					etat_strategie = TOTEM_ATTRAPPE_LINGOTS_2;
				}
				break;
			case TOTEM_ATTRAPPE_LINGOTS_2:
				GetDonneesMoteurs();
				if (get_CT_AV_D() || get_CT_AV_G()){
					prop_stop();
					desactive_asser();
					lingot_attrappe();
					tempo_s= 600; // 4s
					etat_strategie = TOTEM_SORTIE_1;
				}
				break;
			case TOTEM_SORTIE_1:
				tempo_s--;
				if (tempo_s == 0){
					active_asser(ASSER_RECULE,ANGLE_DEGRES(0),&consigne_angle);
					tempo_s = 300;
					etat_strategie = TOTEM_SORTIE_2;
				}
				break;
			case TOTEM_SORTIE_2:
				tempo_s--;
				if (tempo_s == 0){
					active_asser(ASSER_TOURNE,ANGLE_DEGRES(180),&consigne_angle);
					etat_strategie = TEST_SERVO_2_1;
				}
				break;
            case TEST_SERVO_1:
				GetDonneesServo();
				if(get_IS_Gauche()){
					LED_BLEUE=1;
				}else{
					LED_BLEUE=0;
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
                tempo_s = 4000;
                break;
            case TEST_DROIT_2:
				tempo_s--;
				if (tempo_s == 0){
					ignore_contacteur_avant();
				}
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
        *  ACTIONS COMPLEXES   *
        *                      *
        ***********************/

        switch(etat_action){
			// Attrappe CD Bras Droit
			case ATTRAPE_CD_DROIT_INIT:
				cherche_CD_droit();
				CMUcam_active();
				etat_action = ATTRAPE_CD_DROIT_TRACKING;
				break;
			case ATTRAPE_CD_DROIT_TRACKING:
				if ((CMUcam_get_Etat() == TRACKING) || (CMUcam_get_Etat() == TRACKING_PROCHE)){
					LED_ROUGE =1;
					LED_BLEUE =1;
					active_asser(ASSER_TOURNE,consigne_angle,&consigne_angle);
					CDBrasDroit();
					etat_action = ATTRAPE_CD_DROIT_TOURNE;
				}
				break;
			case ATTRAPE_CD_DROIT_TOURNE:
				if (fin_asser()){
					active_asser_lent(ASSER_AVANCE,angle,&consigne_angle);
					etat_action = ATTRAPE_CD_DROIT_AVANCE;
				}
				break;
			case ATTRAPE_CD_DROIT_AVANCE:
				GetDonneesServo();
				if(get_Etat_Droit() >= E_BRAS_BAS_FERME){
					tempo_action++;
					if (tempo_action > 20){
						desactive_asser();
						CMUcam_reset();
						prop_stop();
						etat_action = ATTRAPE_CD_DROIT_LEVE;
						tempo_action = 0;
					}
				}else{
					tempo_action=0;
				}
				break;
			case ATTRAPE_CD_DROIT_LEVE:
				GetDonneesServo();
				if(get_Etat_Droit() >= E_BRAS_ATTENTE_PLEIN){
					etat_action = FIN_ACTION;
				}
				break;
				
			// Attrappe CD Bras Gauche
			case ATTRAPE_CD_GAUCHE_INIT:
				cherche_CD_gauche();
				CMUcam_active();
				etat_action = ATTRAPE_CD_GAUCHE_TRACKING;
				break;
			case ATTRAPE_CD_GAUCHE_TRACKING:
				if ((CMUcam_get_Etat() == TRACKING) || (CMUcam_get_Etat() == TRACKING_PROCHE)){
					LED_ROUGE =1;
					LED_BLEUE =1;
					active_asser(ASSER_TOURNE,consigne_angle,&consigne_angle);
					CDBrasGauche();
					etat_action = ATTRAPE_CD_GAUCHE_TOURNE;
				}
				break;
			case ATTRAPE_CD_GAUCHE_TOURNE:
				if (fin_asser()){
					active_asser_lent(ASSER_AVANCE,angle,&consigne_angle);
					etat_action = ATTRAPE_CD_GAUCHE_AVANCE;
				}
				break;
			case ATTRAPE_CD_GAUCHE_AVANCE:
				GetDonneesServo();
				if(get_Etat_Gauche() >= E_BRAS_BAS_FERME){
					LED_OK = 1;
					LED_OK1 = 1;
					LED_CMUCAM = 0;
					tempo_action++;
					if (tempo_action > 20){
						desactive_asser();
						CMUcam_reset();
						prop_stop();
						etat_action = ATTRAPE_CD_GAUCHE_LEVE;
						tempo_action = 0;
					}
				}else{
					tempo_action=0;
				}
				break;
			case ATTRAPE_CD_GAUCHE_LEVE:
				GetDonneesServo();
				if(get_Etat_Gauche() >= E_BRAS_ATTENTE_PLEIN){
					etat_action = FIN_ACTION;
				}
				break;
			case FIN_ACTION:
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
            transmission_lingot();
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
		while(1){
			LED_BLEUE = 1;
			LED_ROUGE = 1;
			LED_OK = 1;
			Delay10KTCYx(100);
			LED_BLEUE = 0;
			LED_ROUGE = 0;
			LED_OK = 0;
			Delay10KTCYx(100);
		}
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
    LED_BLEUE = 1;
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
    
    LED_OK1 = 1;
    LED_OK=1;
	LED_ROUGE=0;
	LED_BLEUE=0;
	LED_CMUCAM=1;
	

    do{
		if (COULEUR){
			LED_ROUGE = 1;
			LED_BLEUE = 0;
			couleur = 1;
		}else{
			LED_BLEUE = 1;
			LED_ROUGE = 0;
			couleur = 0;
		}
	}while(TIRETTE);
    
    LED_BLEUE=0;
    LED_OK1 = 0;
    LED_OK=0;
    LED_CMUCAM=0;
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

long ANGLE_DEGRES(int X){
	if (couleur == 0){
		return (long)((long)(X) * (long)20000);
	}else{
		return (long)((long)(-X) * (long)20000);
	}
}		

