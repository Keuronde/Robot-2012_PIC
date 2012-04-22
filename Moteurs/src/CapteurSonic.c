/** I N C L U D E S **********************************************************/ 
#include <delays.h>

#include <p18cxxx.h>
#include "../include/CapteurSonic.h"
#include "../include/temps.h"
#include "../include/carte_moteurs.h"
enum etat_cs_t {
	E_CS_INIT=0,
	E_CS_CDE,
	E_CS_ATTENTE_REP,
	E_CS_LECTURE_REP,
	E_CS_TEMPO
} etat_cs=E_CS_INIT;



/** V A R I A B L E S ********************************************************/

static unsigned char distance_5cm=0;
unsigned int tempo_sonique;

// Fonciton à appeller périodiquement, toutes les 290 µs
void CS_gestion(){
	switch (etat_cs){
		case E_CS_CDE:
			TRIS_SONIC = 0; // Patte en sortie
			SONIC = 1;    // Etat haut
			Delay10TCYx(12);
			// On arrete l'impulsion
			SONIC = 0;    // Etat bas
			// On écoute le port
			TRIS_SONIC = 1; // Patte en entrée
			Delay1TCY();
			tempo_sonique=0;
			etat_cs = E_CS_ATTENTE_REP;
			break;
		case E_CS_ATTENTE_REP:
			tempo_sonique++;
			if (SONIC == 1){
				etat_cs = E_CS_LECTURE_REP;
				distance_5cm = 0;
			}
			if (tempo_sonique > 18){ // 5 ms
				etat_cs = E_CS_CDE;
			}
			break;
		case E_CS_LECTURE_REP:
			distance_5cm++;
			if (SONIC == 0){
				etat_cs = E_CS_INIT;
//				tempo_sonique = 12;
			}
			break;
/*		case E_CS_TEMPO:
			tempo_sonique--;
			if (tempo_sonique == 0){
				etat_cs = E_CS_INIT;
			}
			break;*/
		default:
			break;
	}
	
}

char CS_LecturePrete(void){
	if (etat_cs == E_CS_INIT){
		return 1;
	}
	return 0;
}
void CS_Lecture(){
	etat_cs = E_CS_CDE;
}

unsigned int getDistance(){
	return (unsigned int) (distance_5cm * 5);
}
