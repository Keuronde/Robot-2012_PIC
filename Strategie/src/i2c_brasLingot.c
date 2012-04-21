#include <p18cxxx.h>
#include "../include/i2c_m.h"
#include ".../Interfaces/interfaceBrasLingot.h"

char a_envoyer_lingot=0;
union message_bras_t message_bras=0x00;

void lingot_ouvre_doigt(void){
	message_bras.COMMANDE_BRAS = CDE_BRAS_OUVERT;
	a_envoyer_lingot = 1;
}
void lingot_attrappe(void){
	message_bras.COMMANDE_BRAS = CDE_BRAS_ATTRAPPE;
	a_envoyer_lingot = 1;
}
void lingot_depose(void){
	message_bras.COMMANDE_BRAS = CDE_BRAS_DEPOSE;
	a_envoyer_lingot = 1;
}

char transmission_lingot(){
    unsigned char recu;
    if(a_envoyer_lingot == 1){
        // Initialisation de l'i2c
        // renvoi 1 si ok, 0 sinon
        if(transmission_i2c(ADRESSE_LINGOT,NB_STRATEGIE_2_BRAS,NB_BRAS_2_STRATEGIE,&(message_bras.MESSAGE))){
            a_envoyer_lingot=0;
			if(!get_erreur_i2c()){
				while(i2c_en_cours());
		        get_i2c_data(&recu);
      		}
            return 1;
        }
    }    
    return 0;
    
}