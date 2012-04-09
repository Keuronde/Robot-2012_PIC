#include "../include/asservissement.h"
#include "../include/i2c_moteurs.h"



// 10 degrés
#define SEUIL_ANGLE_LENT_INIT  (long) 200000
// 5 degrés
#define INCREMENT_ANGLE_LENT  (long) 100000
// 1 degrés
//#define SEUIL_ANGLE_LENT  (long)  20000
// 3 degrés
#define SEUIL_ANGLE_ARRET (long)  60000

// Variable du module
char asser_actif=0; 					// Pour l'asservissement
char tempo=0,tempo_lent=0;
enum etat_asser_t etat_asser=FIN_ASSER; // Pour l'asservissement
int consigne_pap=0;
int consigne_pap_I=0;
int consigne_pap_P=0;

void Asser_gestion(long * consigne_angle,long * angle){
	static long seuil_angle_lent;
	static char sens_rotation;
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
				Avance();			// Commencer à avancer
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
			if( get_CT_AV_G() || get_CT_AV_D() || get_capteur_sonique_proche() ){
			}else if(get_capteur_sonique_proche()){
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
			pap_set_pos(consigne_pap);
				

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
					Avance();
					sens_rotation = 1;
				}else{
					Recule();
					sens_rotation = 0;
				}
				etat_asser = TOURNE;
				seuil_angle_lent = SEUIL_ANGLE_LENT_INIT;
				tempo = 100;
				tempo_lent = 400;
			}
			break;
		case TOURNE:
	
			if((*consigne_angle-*angle) > 0){
				Avance();
				if(sens_rotation != 1){
					sens_rotation = 1;
					seuil_angle_lent += INCREMENT_ANGLE_LENT;
				}
			}else{
				Recule();
				if(sens_rotation != 0){
					sens_rotation = 0;
					seuil_angle_lent += INCREMENT_ANGLE_LENT;
				}
			}
	
		
			if((*consigne_angle-*angle) < seuil_angle_lent && (*consigne_angle-*angle) > -seuil_angle_lent){
				prop_set_vitesse(0);
				tempo_lent--;
				if(tempo_lent == 0){
					prop_stop();
					etat_asser = FIN_TOURNE;
				}
			}else{
				prop_set_vitesse(1);
				tempo_lent=400;
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
			if( prop_get_sens_avant() ){
				if( get_capteur_sonique_loin() || get_capteur_sonique_proche()){
					if( !get_CT_AV_D() && !get_CT_AV_G() ){
						ignore_contacteur();
					}
				}
			}
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



void active_asser(char avance_droit, long _angle,long * consigne_angle){
	*consigne_angle = _angle;
	if(avance_droit == ASSER_AVANCE){
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
