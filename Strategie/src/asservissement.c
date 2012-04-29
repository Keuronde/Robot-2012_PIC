#include "../include/asservissement.h"
#include "../include/i2c_moteurs.h"



// 10 degrés
#define SEUIL_ANGLE_LENT_INIT  (long) 90000
// 5 degrés
#define INCREMENT_ANGLE_LENT  (long) 30000
// 1 degrés
//#define SEUIL_ANGLE_LENT  (long)  20000
// 3 degrés
#define SEUIL_ANGLE_ARRET (long)  60000

// Variable du module
char asser_actif=0; 					// Pour l'asservissement
char tempo=0,tempo_lent=0,tempo_inversion;
enum etat_asser_t etat_asser=FIN_ASSER; // Pour l'asservissement
int consigne_pap=0;
int consigne_pap_I=0;
int consigne_pap_P=0;
int consigne_prop = 0;
int consigne_prop_P=0;
int consigne_prop_I=0;

char vitesse_lente;
char recule;

void Asser_gestion(long * consigne_angle,long * angle){
	static long seuil_angle_lent;
	static char sens_rotation,inversion;
	// Essayer d'avancer droit (A24)   
	if(asser_actif){   
		switch (etat_asser){

		// Donner l'ordre d'avancer au robot. (A241)
		case AVANCE_DROIT_INIT:
			pap_set_pos(0); // On remet le moteur droit
			consigne_pap_I=0;	// On remet l'intégrateur à 0
			etat_asser = DROIT_TEMPO; // Passe à la correction
			break;
		case DROIT_TEMPO:
			tempo ++;
			if(tempo > 100){
				etat_asser = AVANCE_DROIT;
				if (recule == 0){
					if (vitesse_lente == 0){
						Avance();			// Commencer à avancer
					}else{
						Avance_lent();
					}
				}else{
					if (vitesse_lente == 0){
						Recule();			// Commencer à avancer
					}else{
						Recule_lent();
					}
				}
			}
			break;

		case AVANCE_DROIT:
		// Corriger la trajectoire (A243)
		  // Mesurer la déviation angulaire
		  // Corriger la position de la roue motrice        
		//            consigne_pap=(int)((long)angle/(long)24000); 
			// Avec un correcteur PI
			// On vérifie que l'on est pas à l'arrêt

			
			consigne_pap_P=(int)((long)(*consigne_angle - *angle)/(long)5000); // Pour un asservissement plus nerveux (anciennement 12000)

			
			// On s'occupe du terme intégrale que si on avance
			if( capteur_stop() ){
			}else if(capteur_lent()){
				consigne_pap_I=consigne_pap_I + consigne_pap_P/8;	
			}else{
				consigne_pap_I=consigne_pap_I + consigne_pap_P; // Consigne I
			}

			
			// Saturation de la commande intégrale
			if(consigne_pap_I > (int)720){
				consigne_pap_I = (int)720;
			}
			if(consigne_pap_I < (int)-720){
				consigne_pap_I = (int)-720;
			}
			consigne_pap = consigne_pap_P+ (consigne_pap_I / (int)8);
			// Saturation
			if(consigne_pap > PAP_MAX_ROT){
				consigne_pap = PAP_MAX_ROT;
			}
			if(consigne_pap < PAP_MIN_ROT){
				consigne_pap = PAP_MIN_ROT;
			}
			// Envoie de la commande à la carte moteur
			if (recule == 0){
				pap_set_pos(consigne_pap);
			}else{
				pap_set_pos(-consigne_pap);
			}
				

			etat_asser = 3; // Boucle sur la tempo
			

			break;
			

		case AVANCE_DROIT_TEMPO:
		// Attendre 100 ms (A244)
			tempo++;
			if(tempo > 33){ // On a atteint les 100 ms, on revient à l'état précédent.
//		        	GetDonneesMoteurs();
				etat_asser = 2;
				tempo = 0;
			}
			break;
		case TOURNE_INIT:
			prop_stop();
			pap_set_pos(PAP_MAX_ROT);
			etat_asser = TOURNE_TEMPO;
			tempo=0;
			break;
		case TOURNE_TEMPO:
			tempo ++;
			if(tempo > 100){
				if((*consigne_angle-*angle) > 0){
					Avance_lent();
					sens_rotation = 1;
				}else{
					Recule_lent();
					sens_rotation = 0;
				}
				etat_asser = TOURNE;
				seuil_angle_lent = SEUIL_ANGLE_LENT_INIT;
				tempo = 100;
				tempo_lent = 400;
				tempo_inversion = 1600;
				inversion=0;
			}
			break;
		case TOURNE:
			consigne_prop_P=(int)((long)(*consigne_angle - *angle)/(long)4000); // Pour un asservissement plus nerveux (anciennement 12000)
			consigne_prop_I= consigne_prop_I + consigne_prop_P;
			consigne_prop = consigne_prop_P;// + consigne_prop_I;
			if (consigne_prop > 0){
				Avance();
				if (consigne_prop > 255){
					consigne_prop = 255;
					consigne_prop_I = 0;
				}
				prop_set_vitesse_fine((unsigned char) consigne_prop);
			}else{
				Recule();
				consigne_prop = -consigne_prop;
				if (consigne_prop > 255){
					consigne_prop = 255;
					consigne_prop_I = 0;
				}
				prop_set_vitesse_fine((unsigned char) consigne_prop);
			}
			if((*consigne_angle-*angle) < SEUIL_ANGLE_ARRET && (*consigne_angle-*angle) > -SEUIL_ANGLE_ARRET){
				tempo--;
				if(tempo == 0){
					prop_stop();
					etat_asser = FIN_TOURNE;
				}

			}else{
				tempo=100;				
			}
			// Penser à ignorer le capteur sonique
			ignore_sonique_loin();
			ignore_sonique_proche();
			break;
		case TOURNE_VERS_AVANCE:
			pap_set_pos(PAP_DROIT);
			tempo = 0;
			etat_asser = TOURNE_VERS_AVANCE_2;
			break;
		case TOURNE_VERS_AVANCE_2:
			tempo++;
			if(tempo> 100){
				etat_asser = AVANCE_DROIT_INIT;
			}
			break;
		case FIN_TOURNE:
			break;
		}// Fin Switch
		
	}
}

void active_asser_lent(char avance_droit, long _angle,long * consigne_angle){
	active_asser(avance_droit, _angle, consigne_angle);
	vitesse_lente=1;
}

void active_asser(char avance_droit, long _angle,long * consigne_angle){
	*consigne_angle = _angle;
	recule = 0;
	vitesse_lente=0;
	if( (avance_droit == ASSER_AVANCE) || (avance_droit == ASSER_RECULE) ){
		if (avance_droit == ASSER_RECULE){
			recule = 1;
		}else{
			active_contacteur_avant();
		}
		if(etat_asser == FIN_TOURNE || etat_asser == TOURNE){
			etat_asser = TOURNE_VERS_AVANCE;
		}else{
			etat_asser = AVANCE_DROIT_INIT;
		}
	}else{
		etat_asser = TOURNE_INIT;	
	}
	
	asser_actif=1;
}

void desactive_asser(void){
	asser_actif=0;
}

char fin_asser(){
	if( (etat_asser == FIN_ASSER) || (etat_asser == FIN_TOURNE) ){
		return 1;
	}
	return 0;
}
char get_etat_asser(){
	// renvoie 1 si on avance, 0 sinon
	switch (etat_asser){
	case AVANCE_DROIT_INIT:
	case DROIT_TEMPO:
	case AVANCE_DROIT:
	case AVANCE_DROIT_TEMPO:
		return 1;
		break;
	default :
		return 0;

	}
}
