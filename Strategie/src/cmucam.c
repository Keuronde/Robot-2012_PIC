#include "../include/carte_strategie.h"
#include "../include/asservissement.h"
#include "../include/CMUcam.h"
#include <p18f2550.h>
#include <string.h>


#define CMUCAM_MILIEU_X (int) 176
// 5600 : un peu violent
// 2800 : un peu lent
// 4000 : un peu violent avec une batterie neuve
#define FACTEUR_CMUCAM_ANGLE_DROIT (int) 5600
#define FACTEUR_CMUCAM_ANGLE_TOURNE (int) 2800
#define ID_INVALIDE (unsigned char) 0XFF
#define CONSIGNE_MAX 176




// FONCTIONS PRIVEES
unsigned char test_figure(unsigned char id_forme, unsigned int * critere_figure, volatile figure_t * mFigure);
int erreur_angle(volatile figure_t * _mFigure);
char ask_figure(void);
char select_figure(unsigned char);
char chaine_to_figure(char *chaine,volatile figure_t *figure);
char cmucam_envoi_reset(void);
char get_erreur_RC(void);
char chaine_to_id(char*);
char TX_libre(void);
char env_cmucam(void);
char rec_cmucam(char *chaine);
char rec_cmucam_cours(void);
char set_tampon_env(char *chaine);



enum repere_t {
    R_MILIEU=0,
    R_DROIT,
    R_GAUCHE,
    R_AUCUN
};

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
char tempo_cmucam=0;
enum etat_cmucam_t etat_cmucam=INIT;
char chaine[NB_DATA_IN]; // Reception CMUcam
volatile figure_t mFigure;
unsigned char id_forme;
enum repere_t mRepere;
char cmucam_perdu=0;
unsigned int critere_figure;
int cmucam_cible;
char ile_proche;

// Gestion CMUcam
void CMUcam_gestion(long * consigne_angle,long * angle){
	static char tempo_tracking=0;
	if(cmucam_active){
        	switch(etat_cmucam){
            case INIT:
                if(cherche_couleur()){
                    etat_cmucam=RECEPTION_FORME;
                    id_forme = ID_INVALIDE;
                    tempo_cmucam=200;
                }
                break;
             case NOUVELLE_RECHERCHE:
            	if(nouvelle_recherche()){
            		etat_cmucam=RECEPTION_FORME;
            		id_forme = ID_INVALIDE;
            		tempo_cmucam=200;
            	}
            	break;
            case RECEPTION_FORME:
            	tempo_cmucam--;
                if(rec_cmucam(chaine)){
                	tempo_cmucam=200;
                	if(get_erreur_RC()){
                		etat_cmucam=NOUVELLE_RECHERCHE;
		        		break;
		        	}else{
			            if(chaine[0]=='g'){
					        chaine_to_figure(chaine,&mFigure);
					        etat_cmucam=TEST_FORME;
		                }
                    }
                }
                if(tempo_cmucam==0){
                	etat_cmucam=NOUVELLE_RECHERCHE;
                }
                break;
            case TEST_FORME:
            	if(mFigure.y1==0 && mFigure.x1==0){
					etat_cmucam=ENVOI_ID;
            	}else{
					id_forme = test_figure(id_forme, &critere_figure, &mFigure);
					if(ask_figure()){
						etat_cmucam = RECEPTION_FORME;
					}
            	}
                break;
            case ENVOI_ID:
				if (id_forme == ID_INVALIDE){
				    etat_cmucam=NOUVELLE_RECHERCHE;
				}else{
					if(TX_libre()){
						if(select_figure(id_forme)){
							tempo_tracking=0;
							ile_proche = 0;
							etat_cmucam=TRACKING;
						}
					}
				}
            	break;
            case TRACKING_ATTENTE:
            case TRACKING:
            case TRACKING_PROCHE:
            	get_erreur_RC();
	            if(rec_cmucam(chaine)){
					tempo_tracking=0;
		            if(chaine[0]=='t'){
		            	chaine_to_figure(chaine,&mFigure);
				    	if(mFigure.x1!=0 && mFigure.y1!=0){
				    		if(mFigure.y0 >= 200){
								etat_cmucam = TRACKING_PROCHE;
							}
							if (etat_cmucam == TRACKING_ATTENTE){
								etat_cmucam = TRACKING;
							}
							if(mFigure.y1 >= 200){
								ile_proche=1;
							}
				    		
				    		LED_CMUCAM =1;
				    		if (mRepere != R_AUCUN){
								if(get_etat_asser()){
									*consigne_angle = (long)*angle + (long) ((long)( erreur_angle(&mFigure) ) * (long)FACTEUR_CMUCAM_ANGLE_DROIT);
								}else{
									*consigne_angle = (long)*angle + (long) ((long)( erreur_angle(&mFigure) ) * (long)FACTEUR_CMUCAM_ANGLE_TOURNE);
								}
							}
					        
					        cmucam_perdu=0;
				        }else{
				        	cmucam_perdu++;
				        	if(cmucam_perdu>4){
				        		etat_cmucam=NOUVELLE_RECHERCHE;
				        		LED_BLEUE = 0;
				        	}
				            LED_CMUCAM =0;
				        }
                	}
               	}else{
					tempo_tracking++;
					if (tempo_tracking > 80){
						tempo_tracking=0;
						LED_BLEUE = !LED_BLEUE;
					}
				}
				
               	break;
           case CMUCAM_RESET:
		    	if(TX_libre()){
		    		if(cmucam_envoi_reset()){
			    		cmucam_active=0;
			    		etat_cmucam=CMUCAM_PRETE;
		    		}
		    	}
		    	break;
        	}
        }
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
    
    mRepere = R_DROIT;
    cmucam_cible = CMUCAM_MILIEU_X;
    cmucam_cible = (int)104; // Nominal : 112

}

unsigned char test_figure(unsigned char id_forme, unsigned int * critere_figure, volatile figure_t * mFigure){
	switch (couleur_cmucam){
		case 'W':
			if( mFigure->y0 < 225){
				if( (id_forme == ID_INVALIDE) || ((unsigned int) (mFigure->y1) > (unsigned int)(*critere_figure) ) ) {
					*critere_figure = mFigure->y1;
					id_forme = mFigure->id;
				}
			}
			break;
		case 'P':
			if( mFigure->y0 < 225){
				if( (id_forme == ID_INVALIDE) || ((unsigned int)(mFigure->y1) < (unsigned int)(*critere_figure) ) ) {
					*critere_figure = mFigure->y1;
					id_forme = mFigure->id;
				}
			}
			break;
		default:
			if(id_forme == ID_INVALIDE){
				id_forme = mFigure->id;
			}
	}
	return id_forme;
}

int erreur_angle(volatile figure_t * _mFigure){
	int iRepere,consigne;
	switch (mRepere){
		case R_DROIT:
			iRepere = _mFigure->x1;
			break;
		case R_GAUCHE:
			iRepere = _mFigure->x0;
			break;
		case R_MILIEU:
		default:
			iRepere = _mFigure->x1/2 + _mFigure->x0/2;
			break;
	}
	consigne = iRepere - cmucam_cible; 
	
	return consigne;
}

void CMUcam_active(){
	cmucam_active=1;
	etat_cmucam=INIT;
}
void CMUcam_desactive(){
	cmucam_active=0;
}
void CMUcam_desactive_asser(){
	mRepere = R_AUCUN;
}
void CMUcam_reset(){
	etat_cmucam = CMUCAM_RESET;
}

char cmucam_ile_proche(void){
	return ile_proche;
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

void cherche_lingot(void){	
	couleur_cmucam='P';
    mRepere = R_MILIEU;
    cmucam_cible = CMUCAM_MILIEU_X;
}
void cherche_CD_droit(void){	
	couleur_cmucam='W';
    mRepere = R_GAUCHE;
    cmucam_cible = (int)264;
}
void cherche_CD_gauche(void){	
	couleur_cmucam='W';
    mRepere = R_DROIT;
    cmucam_cible = (int)104;
}
void cherche_CD_ile(void){	
	couleur_cmucam='P';
    mRepere = R_DROIT;
    cmucam_cible = 80;
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

char cmucam_envoi_reset(void){
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

char chaine_to_figure(char *chaine,volatile figure_t *figure){
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
