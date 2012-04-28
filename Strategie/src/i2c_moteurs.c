#include "../include/i2c_moteurs.h"
#include "../include/i2c_m.h"
#include <p18cxxx.h>
#include ".../include/carte_strategie.h"
#include ".../Interfaces/interfaceMoteurs.h"

// Variables globales
// Envoi
char acquittement=0;
int pos_pap = 0;
char sens = 0;
char vitesse = 0;
// Reception
// Contacteurs non acquités (6 bits)
// Capteur sonique (2 bits)
// A tester faire passer la communication avec la carte moteur sur 2 octets d'envoi, 1 en réception
char contacteurs = 0;
char capteur_sonique = 0;
unsigned char a_envoyer_moteur =0;
union c_Moteur_t selection_capteurs_moteur=0xFF;
union c_Moteur_t etat_capteurs_moteurs=0x00;

void pap_set_pos(int pos){
    // On envoie un entier positif compris entre  0 et 400;
    while(pos < 0){
        pos += 800;
    }
    while(pos >= 800){
        pos -= 800;
    }
    pos_pap = pos;
    a_envoyer_moteur =1;
}

void Avance(){
    prop_set_vitesse(1);
    prop_set_sens(1);
    a_envoyer_moteur=1;
}
void Avance_lent(){
    prop_set_vitesse(0);
    prop_set_sens(1);
    a_envoyer_moteur=1;
}
void Recule(){
	ignore_contacteur_avant();
    prop_set_vitesse(1);
    prop_set_sens(0);
    a_envoyer_moteur=1;
}
void Recule_lent(){
	ignore_contacteur_avant();
    prop_set_vitesse(0);
    prop_set_sens(0);
    a_envoyer_moteur=1;
}

void prop_set_vitesse(char rapide){
    if(rapide){
        vitesse = 1;
    }else{
        vitesse = 0;
    }
    a_envoyer_moteur=1;
}

void prop_set_sens(char avant){
    if(avant> 0){
        sens = 1;
    }else{
        sens = 2;
    }
    a_envoyer_moteur =1;
    
}

char prop_get_sens_avant(void){
	if(sens == 1){
		return 1;
	}
	return 0;
}

void prop_stop(void){
    sens = 0;
    a_envoyer_moteur =1;
}

void ignore_contacteur(void){
    acquittement = 1;
}
void active_contacteur_avant(){
	selection_capteurs_moteur.CT_AV_D =1;
	selection_capteurs_moteur.CT_AV_G =1;
	a_envoyer_moteur =1;
}
void active_contacteur_avant_gauche(){
	selection_capteurs_moteur.CT_AV_G =1;
	a_envoyer_moteur =1;
}
void active_contacteur_avant_droit(){
	selection_capteurs_moteur.CT_AV_D =1;
	a_envoyer_moteur =1;
}

void ignore_contacteur_avant(){
	selection_capteurs_moteur.CT_AV_D =0;
	selection_capteurs_moteur.CT_AV_G =0;
	a_envoyer_moteur =1;
}
void ignore_contacteur_avant_gauche(){
	selection_capteurs_moteur.CT_AV_G =0;
	a_envoyer_moteur =1;
}
void ignore_contacteur_avant_droit(){
	selection_capteurs_moteur.CT_AV_D =0;
	a_envoyer_moteur =1;
}

void ignore_sonique_proche(){
	selection_capteurs_moteur.SONIC_PROCHE_D =0;
	selection_capteurs_moteur.SONIC_PROCHE_G =0;
	a_envoyer_moteur =1;
}
void ignore_sonique_loin(){
	selection_capteurs_moteur.SONIC_LOIN_D =0;
	selection_capteurs_moteur.SONIC_LOIN_G =0;
	a_envoyer_moteur =1;
}
void active_sonique_proche(){
	selection_capteurs_moteur.SONIC_PROCHE_D =1;
	selection_capteurs_moteur.SONIC_PROCHE_G =1;
	a_envoyer_moteur =1;
}
void active_sonique_loin(){
	selection_capteurs_moteur.SONIC_LOIN_D =1;
	selection_capteurs_moteur.SONIC_LOIN_G =1;
	a_envoyer_moteur =1;
}


char get_contacteurs(){
    return contacteurs;
}
char get_CT_AV_G(){
    return etat_capteurs_moteurs.CT_AV_G;
}
char get_CT_AV_D(){
    return etat_capteurs_moteurs.CT_AV_D;
}
char get_capteur_sonique(){
    return capteur_sonique;
}
char get_capteur_sonique_loin(){
    return contacteurs & 0x40;
}
char get_capteur_sonique_proche(){
    return contacteurs & 0x80;
}
unsigned char capteur_lent(){
	return (selection_capteurs_moteur.VALEUR & etat_capteurs_moteurs.VALEUR & CAPTEUR_RALENTI);
}
unsigned char capteur_stop(){
	return (selection_capteurs_moteur.VALEUR & etat_capteurs_moteurs.VALEUR & CAPTEUR_ARRET);
}

void GetDonneesMoteurs(void){
    a_envoyer_moteur = 1;
}
char GetEnvoiMoteurs(void){
    return a_envoyer_moteur;
}

char transmission_moteur(){
    // Préparation de l'envoi
    unsigned char envoi[NB_STRATEGIE_2_MOTEUR];
    unsigned char recu;
    if(a_envoyer_moteur == 1){
        // 9 bits pour la position du moteur pas à pas.
        envoi[0] = pos_pap & 0xFF;
        envoi[1] = (pos_pap >> 8) & 0x03;
        // 2 bits pour le sens du moteur
        envoi[1] |= ((sens & 0x03) << 2);
        // 1 bit pour la vitesse
        envoi[1] |= ((vitesse & 0x01) << 4);
        // 1 bit pour l'acquittement
        envoi[1] |= ((acquittement & 0x01) << 5);
        // Geston des capteurs de la carte moteur
        envoi[2] = selection_capteurs_moteur.VALEUR;
        
        // Initialisation de l'i2c
        // renvoi 1 si ok, 0 sinon
        if(transmission_i2c(ADRESSE_PAP,NB_STRATEGIE_2_MOTEUR,NB_MOTEUR_2_STRATEGIE,envoi)){
            a_envoyer_moteur=0;
            acquittement =0;
			if(!get_erreur_i2c()){
            while(i2c_en_cours());
		        get_i2c_data(&recu);
		        etat_capteurs_moteurs.VALEUR = recu;
      		}
            return 1;
        }
    }    
    return 0;
    
}
