#include "../include/i2c_m.h"
#include "../include/i2c_servo.h"
#include <p18cxxx.h>
#include "../include/carte_strategie.h"

unsigned char envoi_servo;
unsigned char a_envoyer=0;
unsigned char IS_Droit=0,IS_Gauche=0,Etat_Gauche=0,Etat_Droit=0;
unsigned char presence_pion;


void SetServoPArG(char haut){
    if(haut){
        envoi_servo |= 0x01;
    }else{
        envoi_servo &= 0xFE;
    }
    a_envoyer = 1;
}

void SetServoPArD(char haut){
    if(haut){
        envoi_servo |= 0x02;
    }else{
        envoi_servo &= 0xFD;
    }
    a_envoyer = 1;
}

void SetServoPAv(char haut){
    if(haut){
        envoi_servo |= 0x04;
    }else{
        envoi_servo &= 0xFB;
    }
    a_envoyer = 1;
}

void SetServoCMUcam(char haut){
    if(haut){
        envoi_servo |= 0x08;
    }else{
        envoi_servo &= 0xF7;
    }
    a_envoyer = 1;
}

void SetCremaillere(char haut){
    if(!haut){
        envoi_servo |= 0x20;
    }else{
        envoi_servo &= 0xDF;
    }
    a_envoyer = 1;
}

void GetDonneesServo(void){
    a_envoyer = 1;
}
char GetEnvoiServo(void){
    return a_envoyer;
}

char transmission_servo(void){

    if(a_envoyer == 1){
        unsigned char _chaine[3];
        if(transmission_i2c(ADRESSE_SERVO,1,3,&envoi_servo)){
            a_envoyer =0;
            LED_ROUGE=0;
            while(i2c_en_cours());
            LED_ROUGE=1;
			if(!get_erreur_i2c()){
				get_i2c_data( _chaine );
				Etat_Gauche = _chaine[0];
				Etat_Droit = _chaine[1];
				IS_Gauche = _chaine[2] & 0x01;
				IS_Droit = _chaine[2] & 0x02;
				envoi_servo=0;
				
			}
            return 1;
        }
    }
    return 0;
}

char get_IS_Droit(void){
    return IS_Droit;
} 
char get_IS_Gauche(void){
    return IS_Gauche;
} 


char get_Etat_Droit(void){
    return Etat_Droit;
} 
char get_Etat_Gauche(void){
    return Etat_Gauche;
} 
