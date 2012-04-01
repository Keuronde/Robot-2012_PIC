#include "../include/carte_strategie.h"
#include "../include/asservissement.h"
#include "../include/CMUcam.h"
#include <p18f2550.h>
#include <string.h>


#define CMUCAM_MILIEU_X (int) 176
#define FACTEUR_CMUCAM_ANGLE (int) 2800

/** P R I V A T E   P R O T O T Y P E S *******************************/
long figureToConsigne(long * angle);

/** P R I V A T E   V A R I A B L E S *********************************/
volatile char CMUcam_in[NB_DATA_IN]; // Au maximum : 1 lettre, 5 lots de trois chiffres, 5 espaces, un caractère de fin
volatile char CMUcam_out[NB_DATA_OUT]; // 3 chiffre plus le caractère de fin.
volatile char CMUcam_in_index;
volatile char CMUcam_out_index;
volatile enum booleen CMUcam_RX_libre;
volatile enum booleen CMUcam_TX_libre;
volatile enum booleen _nouvelle_reception = NON;
volatile enum booleen _donnees_envoyees = NON;
volatile char _erreur_RC = 0;

char couleur_cmucam='P';
char cmucam_active=0;
char nb_essai_cmucam;
char tempo_cmucam=0;
enum etat_cmucam_t etat_cmucam=INIT;
char chaine[NB_DATA_IN]; // Reception CMUcam
figure_t mFigure;
unsigned char id_forme;
extern enum etat_asser_t etat_asser; // Pour l'asservissement
char cmucam_perdu=0;

// Gestion CMUcam
void CMUcam_gestion(long * consigne_angle,long * angle){
	if(cmucam_active){
        	switch(etat_cmucam){
            case INIT:
                if(cherche_couleur()){
                    etat_cmucam=RECUP_ID_1;
                    tempo_cmucam=200;
                    nb_essai_cmucam = 10;
                }
                break;
            case RECUP_ID_1:
            	tempo_cmucam--;
                if(rec_cmucam(chaine)){
                	tempo_cmucam=200;
                	if(get_erreur_RC()){
                		etat_cmucam=RE_RECUP_ID_1;
		        		break;
		        	}else{
			            if(chaine[0]=='g'){
					        chaine_to_figure(chaine,&mFigure);
					        etat_cmucam=TEST_ID;
		                }
                    }
                }
                if(tempo_cmucam==0){
                	etat_cmucam=RE_RECUP_ID_1;
                }
                break;
            case TEST_ID:
            	if(mFigure.y1==0 && mFigure.x1==0){
					etat_cmucam=RE_RECUP_ID_1;
					
            	}else{
            		etat_cmucam = ENVOI_ID;
            		id_forme=mFigure.id;
            	}
                break;
            case RE_RECUP_ID_1:
            	if(nouvelle_recherche()){
            		etat_cmucam=RE_RECUP_ID_2;
            		tempo_cmucam=200;
            	}
            	break;
            case RE_RECUP_ID_2:
            	tempo_cmucam--;
            	if(rec_cmucam(chaine)){
	            	tempo_cmucam=200;
            		if(get_erreur_RC()){
		        		etat_cmucam=RE_RECUP_ID_1;
		        		break;
		        	}else{
			        	if(chaine[0]=='g'){
				    		chaine_to_figure(chaine,&mFigure);
				    		etat_cmucam=TEST_ID;
		        		}
            		}
            	}
            	if(tempo_cmucam == 0){
            		etat_cmucam=RE_RECUP_ID_1;
            	}
            	break;
            case ENVOI_ID:
            	if(TX_libre()){
            		if(select_figure(id_forme)){
            			etat_asser=0;
	            		etat_cmucam=TRACKING;
            		}
            	}
            	break;
            case TRACKING:
            case TRACKING_PROCHE:
            	get_erreur_RC();
	            if(rec_cmucam(chaine)){
		            if(chaine[0]=='t'){
		            	chaine_to_figure(chaine,&mFigure);
				    	if(mFigure.x1!=0 && mFigure.y1!=0){
				    		int milieu;
				    		
				    		if(mFigure.y1 <= 68){
								etat_cmucam = TRACKING_PROCHE;
							}

				    		
				    		asser_actif=1;
					        LED_CMUCAM =1;
					        milieu = mFigure.x1/2 + mFigure.x0/2;
					        *consigne_angle = figureToConsigne(angle);// + ((long)(milieu - CMUCAM_MILIEU_X) * (long)FACTEUR_CMUCAM_ANGLE);
					        
					        cmucam_perdu=0;
				        }else{
				        	cmucam_perdu++;
				        	if(cmucam_perdu>4){
				        		etat_cmucam=PERDU;
				        	}
				            LED_CMUCAM =0;
				        }
                	}
               	}
               	break;
           case CMUCAM_RESET:
		    	if(TX_libre()){
		    		if(cmucam_reset()){
			    		cmucam_active=0;
		    		}
		    	}
		    	break;
        	}
        }
	}


long figureToConsigne(long * angle){
	char milieu;
	milieu = mFigure.x1/2 + mFigure.x0/2;
	return ((long)*angle + ((long)(milieu - CMUCAM_MILIEU_X) * (long)FACTEUR_CMUCAM_ANGLE));
}

// Initialisation
void CMUcam_Init(void){
	//Initialisation de la CMUcam
	TRISCbits.TRISC6 = 1; // Pattes configurées en sortie
	TRISCbits.TRISC7 = 1; // Pattes configurées en sortie
	SPBRGH = 0;
	SPBRG = 103;
	BAUDCONbits.BRG16 = 1;
	TXSTAbits.BRGH =1;  // High Speed
	TXSTAbits.SYNC = 0; // Asynchrone
	RCSTAbits.SPEN = 1; // Activation du module
	RCSTAbits.RX9 = 0; // Mots de 8 bits
	PIE1bits.RCIE = 1; // Activation des interruptions du port com en reception
	PIE1bits.TXIE = 0; // Activation des interruptions du port com en emission
	RCSTAbits.CREN = 1; // Activation reception
	TXSTAbits.TXEN = 1; // Activation Transmission
	
	// Indexs
	CMUcam_in_index = 0;
	CMUcam_out_index = 0;
	_nouvelle_reception = NON;
	_donnees_envoyees = NON;
	
	CMUcam_TX_libre=OUI;
	CMUcam_RX_libre=OUI;
	
	chaine[0]=0;
    chaine[1]=0;
    chaine[2]=0;
    chaine[3]=0;
    chaine[4]=0;
    chaine[5]=0;
    chaine[6]=0;
    chaine[7]=0;

}

void CMUcam_active(){
	cmucam_active=1;
	etat_cmucam=INIT;
}
void CMUcam_desactive(){
	cmucam_active=0;
}
enum etat_cmucam_t CMUcam_get_Etat(){
	return etat_cmucam;
}

void setCouleur(char _c){
	couleur_cmucam = _c	;
}

char cherche_couleur(void){
    CMUcam_out[0]=couleur_cmucam;
    CMUcam_out[1]=CMUCAM_FIN;
    return env_cmucam();
	
}

char cherche_pion(void){	
    CMUcam_out[0]='P';
    CMUcam_out[1]=CMUCAM_FIN;
    return env_cmucam();
}
char cherche_case_rouge(void){
    CMUcam_out[0]='R';
    CMUcam_out[1]=CMUCAM_FIN;
    return env_cmucam();
}
char cherche_case_bleue(void){
    CMUcam_out[0]='B';
    CMUcam_out[1]=CMUCAM_FIN;
    return env_cmucam();
}
char nouvelle_recherche(void){
    CMUcam_out[0]='/';
    CMUcam_out[1]=CMUCAM_FIN;
    if(env_cmucam()){
        _nouvelle_reception = NON;
        return 1;
    }
    return 0;
}
char get_erreur_RC(void){
	if(_erreur_RC == 1){
		_erreur_RC = 0;
		return 1;
	}
	return 0;
}

char cmucam_reset(void){
    CMUcam_out[0]='~';
    CMUcam_out[1]=CMUCAM_FIN;
    if(env_cmucam()){
        _nouvelle_reception = NON;
        return 1;
    }
    return 0;
}
char ask_figure(void){
    CMUcam_out[0]=' ';
    CMUcam_out[1]=CMUCAM_FIN;
    if(env_cmucam()){
        _nouvelle_reception = NON;
        return 1;
    }
    return 0;
}
char select_figure(unsigned char id){
    CMUcam_out[0]= id / 100 + '0';
    id = id % 100;
    CMUcam_out[1]= id / 10 + '0';
    id = id % 10;
    CMUcam_out[2]= id + '0'; 
    CMUcam_out[3]=CMUCAM_FIN;

    if(env_cmucam()){
        _nouvelle_reception = NON;
        return 1;
    }
    return 0;
}


char chaine_to_id(char *chaine){
    char i,j=0;
    unsigned char id=0;
    
    while(chaine[i] != CMUCAM_FIN && j<5){
        if(chaine[i] == ' '){
            j++;
        }
        i++;
    }
    while(chaine[i] >= '0' && chaine[i] <= '9'){
        id = id * 10;
        id = (chaine[i] - '0') + id;
        i++;
    }
    return id;
}

char chaine_to_figure(char *chaine,figure_t *figure){
    // chaine : "g 0 213 100 267 24"
    unsigned char i=0,j=0;
    unsigned int temp=0;
    figure->x0 = 0;
    figure->x1 = 0;
    figure->y0 = 0;
    figure->y1 = 0;
    figure->id = 0;
    
    while((chaine[i] != CMUCAM_FIN) && j<5){
        if(chaine[i] == ' '){
            j++;
            i++;
            temp=0;
            while(chaine[i] >= '0' && chaine[i] <= '9'){
                temp = temp * 10;
                temp = (chaine[i] - '0') + temp;
                i++;
            }
            switch(j){
            case 1:figure->x0 = temp; break;
            case 2:figure->y0 = temp; break;
            case 3:figure->x1 = temp; break;
            case 4:figure->y1 = temp; break;
            case 5:figure->id = (unsigned char)temp; break;
            }
        }else{
            i++;
        }
    }
    return figure->id;
}

char rec_cmucam(char *chaine){
    // Afin de ne pas recevoir le début de la trame n et la fin de la trame n+1
    // Ou de ne pas relire inutilement une trame déjà lue.
    if(CMUcam_RX_libre == NON || _nouvelle_reception == NON ){
        // Il s'agit d'une reception
        return 0;
    }
    memcpy( (void*) chaine, (void*) CMUcam_in, NB_DATA_IN );
    _nouvelle_reception = NON; // On se note que nous avons lu ces données. 
    return 1;
}



char rec_cmucam_cours(){
    if (CMUcam_RX_libre == OUI){
        return 0;
    }else{
        return 1;
    }
}

char env_cmucam(void){
    if (CMUcam_TX_libre == OUI){
        unsigned char i=0;
        while( (CMUcam_out[i] != CMUCAM_FIN) && (i < NB_DATA_OUT) ){
            i++;
        }
        if(i == NB_DATA_OUT){
            return 0;
        }
        if(i > 0){
            CMUcam_TX_libre = NON;
        }
        CMUcam_out_index = 0;
       	PIE1bits.TXIE = 1; // Activation des interruptions du port com en emission
        
        
        return 1;
    }
    return 0;
}
char set_tampon_env(char *chaine){
    unsigned char i=0;
    while( (chaine[i] != CMUCAM_FIN) && (i < NB_DATA_OUT) ){
        i++;
    }
    if(i == NB_DATA_OUT){
        return 0;
    }
    memcpy((void *)CMUcam_out,(void *)chaine,i);
    return 1;
    
}
char TX_libre(){
    if(CMUcam_TX_libre == NON){
        return 0;
    }else{
        return 1;
    }
}


// Partie en interruption
// Interruption du port com
// Créer une grosse fonction CMUcam_Int ou objet_proche
void CMUcam_int(void){
    // Reception
	if( PIR1bits.RCIF){
		// On réarme l'interruption
		PIR1bits.RCIF = 0;
		
		// S'il y a une erreur, 
		if(RCSTAbits.OERR == 1 || RCSTAbits.FERR == 1){
			_erreur_RC=1;
			RCSTAbits.CREN = 0;
			RCSTAbits.CREN = 1;

			CMUcam_in_index = 0;
			
		}
		
		// On ajoute la donnée reçue au tampon
		if(CMUcam_in_index == 0){
			CMUcam_RX_libre = NON;
			_nouvelle_reception = NON;
		}
		CMUcam_in[CMUcam_in_index] = RCREG;
		if(CMUcam_in[CMUcam_in_index] == 'g' || CMUcam_in[CMUcam_in_index] == 't' || (CMUcam_in[CMUcam_in_index] >= '0' && CMUcam_in[CMUcam_in_index] <= '9') || CMUcam_in[CMUcam_in_index] == ' ' || CMUcam_in[CMUcam_in_index] == 0x0D || CMUcam_in[CMUcam_in_index] == 0x0A){
			if(CMUcam_in[CMUcam_in_index] == CMUCAM_FIN || CMUcam_in_index == NB_DATA_IN || CMUcam_in[CMUcam_in_index] == 0 ){
				// Fin de la reception
				CMUcam_RX_libre = OUI;
				_nouvelle_reception = OUI;
				CMUcam_in_index = 0;
			}else{
				CMUcam_in_index++;
			}
		}
	}
	
	// Emission
	if( PIR1bits.TXIF){
		// On ne réarme pas l'interruption !

        // Si on a encore des choses à emettre
		if(CMUcam_TX_libre == NON){
		    // Envoi des données

		    TXREG = CMUcam_out[CMUcam_out_index];
		
		    // Si on vient d'envoyer le dernier caractère
		    if(CMUcam_out[CMUcam_out_index] == CMUCAM_FIN || CMUcam_out_index == NB_DATA_OUT){
		        // On dit qu'on n'a plus besoin d'émettre
		        // Meme si il faudrait attendre que l'octet soit envoyé pour ça
		        CMUcam_TX_libre = OUI;
		        CMUcam_out_index=0;
		        // On n'a plus besoin de l'interruption
            	PIE1bits.TXIE = 0;
		    }else{
       		    CMUcam_out_index++;
       		}
		}
	}
	
}

// Fin de la fonction CMUcam_Int
