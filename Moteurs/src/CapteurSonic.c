/** I N C L U D E S **********************************************************/ 
#include <delays.h>

#include <p18cxxx.h>
#include "../include/CapteurSonic.h"
#include "../include/temps.h"
#include "../include/carte_moteurs.h"
enum etat_cs_t {
	E_CS_INIT=0,
	E_CS_CDE_D,
	E_CS_ATTENTE_REP_D,
	E_CS_LECTURE_REP_D,
	E_CS_TEMPO_D,
	E_CS_CDE_G,
	E_CS_ATTENTE_REP_G,
	E_CS_LECTURE_REP_G,
	E_CS_TEMPO_G
} etat_cs=E_CS_INIT;



/** V A R I A B L E S ********************************************************/

static unsigned char distance_5cm_g,distance_5cm_d=0;
unsigned int tempo_sonique;

// Fonciton à appeller périodiquement, toutes les 290 µs
void CS_gestion(){
	switch (etat_cs){
		case E_CS_CDE_D:
			TRIS_SONIC_D = 0; // Patte en sortie
			SONIC_D = 1;    // Etat haut
			Delay10TCYx(12);
			// On arrete l'impulsion
			SONIC_D = 0;    // Etat bas
			// On écoute le port
			TRIS_SONIC_D = 1; // Patte en entrée
			Delay1TCY();
			tempo_sonique=0;
			etat_cs = E_CS_ATTENTE_REP_D;
			break;
		case E_CS_ATTENTE_REP_D:
			tempo_sonique++;
			if (SONIC_D == 1){
				etat_cs = E_CS_LECTURE_REP_D;
				distance_5cm_d = 0;
			}
			if (tempo_sonique > 18){ // 5 ms
				etat_cs = E_CS_TEMPO_D;
				distance_5cm_d = 0;
			}
			break;
		case E_CS_LECTURE_REP_D:
			tempo_sonique++;
			distance_5cm_d++;
			if (SONIC_D == 0){
				etat_cs = E_CS_TEMPO_D;
			}
			break;
		case E_CS_TEMPO_D:
			tempo_sonique++;
			if (tempo_sonique > 103){ // 30 ms
				etat_cs = E_CS_CDE_G;
			}
			break;
		case E_CS_CDE_G:
			TRIS_SONIC_G = 0; // Patte en sortie
			SONIC_G = 1;    // Etat haut
			Delay10TCYx(12);
			// On arrete l'impulsion
			SONIC_G = 0;    // Etat bas
			// On écoute le port
			TRIS_SONIC_G = 1; // Patte en entrée
			Delay1TCY();
			tempo_sonique=0;
			etat_cs = E_CS_ATTENTE_REP_G;
			break;
		case E_CS_ATTENTE_REP_G:
			tempo_sonique++;
			if (SONIC_G == 1){
				etat_cs = E_CS_LECTURE_REP_G;
				distance_5cm_g = 0;
			}
			if (tempo_sonique > 18){ // 5 ms
				etat_cs = E_CS_TEMPO_G;
				distance_5cm_g = 0;
			}
			break;
		case E_CS_LECTURE_REP_G:
			tempo_sonique++;
			distance_5cm_g++;
			if (SONIC_G == 0){
				etat_cs = E_CS_TEMPO_G;
			}
			break;
		case E_CS_TEMPO_G:
			tempo_sonique++;
			if (tempo_sonique > 103){ // 30 ms
				etat_cs = E_CS_INIT;
			}
			break;
		default:
			break;
	}
	
}

char CS_LecturePrete(void){
	if ((etat_cs == E_CS_INIT) || (etat_cs == E_CS_TEMPO_G)){
		return 1;
	}
	return 0;
}
void CS_Lecture(){
	etat_cs = E_CS_CDE_D;
}

unsigned int getDistance(){
	static unsigned char distance_5cm;
	distance_5cm = distance_5cm_d;
	if (distance_5cm > distance_5cm_g){
		distance_5cm = distance_5cm_g;
	}
	return (unsigned int) (distance_5cm * 5);
}
