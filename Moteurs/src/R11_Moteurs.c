#include <p18cxxx.h>
#include <delays.h>
#include "../include/carte_moteurs.h"
#include "../include/PaP.h"
#include "../include/i2c_s.h"
#include "../include/M_prop.h"
#include "../include/temps.h"
#include "../include/CapteurSonic.h"
#include "../../Interfaces/interfaceMoteurs.h"



/** V A R I A B L E S ********************************************************/
#pragma udata

/** P R I V A T E  P R O T O T Y P E S ***************************************/
void MyInterrupt(void);
void Init(void);


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
	
    com_i2c();
    mPaP_int();
    Temps_Int();

	PRODL = sauv1;
	PRODH = sauv2;		

}


#pragma code



void main(void){
    unsigned char recu[NB_STRATEGIE_2_MOTEUR];
    unsigned char envoi;
    // Gestion des capteurs
    union c_Moteur_t etat_capteurs, selection_capteur;
    char valeur_sonic_loin[10];
    char valeur_sonic_proche[10];
    char CT_acquittement;
    char index_sonic=0;
    int consigne_recu=0;
    int consigne_recu_old=0;
    char vitesse=0;
    char sens,i;
    int tempo_led=0;
    char vitesse_cde;
    char acquittement;
    unsigned char vitesse_fine;
    unsigned int T_old;
    int  t_v0;
    
    // Init 
    Init();
    selection_capteur.VALEUR = 0xFF;
    etat_capteurs.VALEUR = 0;
    
    for(i=0;i<10;i++){
		valeur_sonic_proche[i]=0;
		valeur_sonic_loin[i]=0;
	}
    // Fin init
    
    
    for(index_sonic = 0;index_sonic< 10; index_sonic++){
        valeur_sonic_loin[index_sonic] = 0;
        valeur_sonic_proche[index_sonic] = 0;
    }
    
    CS_Lecture();
// On allume les LEDs


    while(1){
        int i;
        char total_sonic_proche,total_sonic_loin;
        unsigned int distance;
        
		if (T_old != getTemps_290us()){
			T_old = getTemps_290us();
			tempo_led++;
			CS_gestion();
			
		}
		// Debug Temps
        /*if (tempo_led == 3448){
			tempo_led = 0;
			LED_OK = !LED_OK;
		}*/
		
        // Reception d'ordre
        if(rec_i2c(recu)){
//        	LED_OK = 1;

            // Consigne PaP
            consigne_recu = (int)recu[0] + (((int) recu[1] & 3) << 8);
            //if(consigne_recu != consigne_recu_old){
	            set_consigne(consigne_recu);
            //}
            consigne_recu_old = consigne_recu;
            // Sens M Propulsion
            sens = (recu[1] >> 2) & 0x03;
            
            
            if(sens == 0){
                Stop();
            }else{
                if(sens == 1){
                    Avance();
                }else{
                    Recule();
                }
            }
            
            
            // Vitess M_Propulsion
            vitesse_cde = (recu[1] >> 4) & 0x03;
            vitesse_fine = recu[3];
            
            
            // Acquitement contacteur
            selection_capteur.VALEUR = recu[2];
            
            // Se servir de ce bit pour l'activation/desactivaiton du moteur pas à pas.
            //acquittement = (recu[1] >> 5) & 0x01;
            
        }
        
        // Lecture des capteurs
        etat_capteurs.CT_AV_D = CT2;
        etat_capteurs.CT_AV_G = CT3;
        
        envoi_i2c((char *) &(etat_capteurs.VALEUR));
        
        // Gestion de la vitesse  
        // Vitesse commandées
        if(vitesse_cde == 3){
			vitesse = 3;
        }else if(vitesse_cde == 1){
			vitesse = 2;
        }else if(vitesse_cde == 0){
            vitesse = 1;
        }
        
        // On ignore les consignes de sens, c'est la carte stratégie qui nous indique quel capteur utiliser.
        if( selection_capteur.VALEUR & etat_capteurs.VALEUR & CAPTEUR_RALENTI){
			vitesse = 1;
		}
        if( (selection_capteur.VALEUR & etat_capteurs.VALEUR & CAPTEUR_ARRET) ){
			vitesse = 0;
		}
		
        
        // Si le robot est sensé avancer, on ajuste la vitesse
        switch(vitesse){
		case 3:
			V_fine(vitesse_fine);
			break;
        case 2:
            V_rapide();
            break;
            
        case 1:
            V_lent();
            break;
            
        case 0:
        default :
            V_stop();
            break;
        }
        
        
        
        
        // Lecture du capteur sonique
        // Ne pas lire le capteur sonique si la denière lecture date de moins de 50ms.
        // XXX Rajouter une condition ici
        // Quelque chose comme :
        if(CS_LecturePrete()){

            
            distance = getDistance();
            CS_Lecture();
           // distance = _45_CM;
            
            if(index_sonic == 3){
                index_sonic = 0;
            }

            if(distance < _85_CM ){
                valeur_sonic_loin[index_sonic]=1;
            }else{
                valeur_sonic_loin[index_sonic]=0;
            }
            
            if(distance < _55_CM ){
                valeur_sonic_proche[index_sonic]=1;
            }else{
                valeur_sonic_proche[index_sonic]=0;
            }
            
            
            total_sonic_proche = 0;
            total_sonic_loin = 0;
            for(i=0;i<3;i++){
                total_sonic_proche += valeur_sonic_proche[i];
                total_sonic_loin += valeur_sonic_loin[i];
            }
            if(total_sonic_proche > 2){
                etat_capteurs.SONIC_PROCHE_D = 1;
                //LED_OK =1;
            }else{
                etat_capteurs.SONIC_PROCHE_D = 0;
                //LED_OK =0;
            }
            
            if(total_sonic_loin > 2){
                etat_capteurs.SONIC_LOIN_D = 1;
                LED_OK =1;
            }else{
                etat_capteurs.SONIC_LOIN_D = 0;
                LED_OK =0;
            }
            
            index_sonic++;
        }
        // Fin Capteur Sonique
        
        
    }

}


void Init(){
	INTCONbits.GIE = 1;
	INTCONbits.PEIE = 1;
    init_io();
    init_i2c(0x31); // Les interruptions doivent être activées pour que l'I2C foncitonne	
    MoteurPInit();
    M_prop_init();
    Temps_Init();

    LED_OK = 1;
    Delay10KTCYx(0);
    LED_OK = 0;
    Delay10KTCYx(0);
    LED_OK = 1;
    Delay10KTCYx(0);
    LED_OK = 0;
    Delay10KTCYx(0);
    LED_OK = 1;
    Delay10KTCYx(0);
    LED_OK = 0;
    Delay10KTCYx(0);
    TRIS_CT1 = 0; // sortie
    


}












