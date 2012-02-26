#include <pwm.h>
#include "moteurs.h"

// Variables privées
enum etat_moteur_t {
/* Attraper le lingo */
    M_ARRET=0,
    M_AVANCE,
    M_RECULE
};
enum etat_moteur_t e_moteur_1;
enum etat_moteur_t e_moteur_2;

void Moteurs_Init(){
	TRIS_M1_ENABLE = 0;
	TRIS_M2_ENABLE = 0;
	TRIS_M1_SENS = 0;
	TRIS_M2_SENS = 0;
    OpenPWM2(0xff);
    OpenPWM1(0xff);
    e_moteur_1 = M_ARRET;
    e_moteur_2 = M_ARRET;
}


// Moteur 1
void M1_Avance(void){
	if(e_moteur_1 != M_AVANCE){
		M1_SENS = 1;
		SetDCPWM1(V_MOTEUR);
		e_moteur_1 = M_AVANCE;
	}
}

void M1_Recule(void){
	if(e_moteur_1 != M_RECULE){
		M1_SENS = 0;
		SetDCPWM1(V_MOTEUR);
		e_moteur_1 = M_RECULE;
	}
}

void M1_Stop(void){
	if(e_moteur_1 != M_ARRET){
		SetDCPWM1(0);
		e_moteur_1 = M_ARRET;
	}
}


// Moteur 2
void M2_Avance(void){
	if(e_moteur_2 != M_AVANCE){
		M2_SENS = 1;
		SetDCPWM2(V_MOTEUR);
		e_moteur_2 = M_AVANCE;
	}
}

void M2_Recule(void){
	if(e_moteur_2 != M_RECULE){
		M2_SENS = 0;
		SetDCPWM2(V_MOTEUR);
		e_moteur_2 = M_RECULE;
	}
}

void M2_Stop(void){
	if(e_moteur_2 != M_ARRET){
		SetDCPWM2(0);
		e_moteur_2 = M_ARRET;
	}
}
