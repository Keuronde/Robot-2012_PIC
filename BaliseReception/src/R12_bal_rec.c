#include <p18cxxx.h>
#include <timers.h>
#include <pwm.h>
#include "../include/i2c_s.h"

/** D E F I N E D ********************************************************/
// Identifiant balise
#define ID_BALISE_1 0x44
#define ID_BALISE_2 0x25
// Emission IR
#define NB_MESSAGES 16
#define NB_MSG_TOTAL 48
// Durée de vie (10 secondes) // 1807
#define T_MAX ((unsigned int) 1807)
// Clignotement LED
#define F_1HZ 90
#define F_5HZ 18
/** V A R I A B L E S ********************************************************/
#pragma udata
volatile unsigned char timer_led;
volatile unsigned char timer_init;
volatile unsigned char id_recepteur;
volatile unsigned char tab_reception[NB_MSG_TOTAL];
volatile unsigned char tab_traitement[NB_MESSAGES + NB_MESSAGES/2];
volatile unsigned char active_calcul;
volatile unsigned int  timer_desynchro;
volatile unsigned char synchro;
volatile unsigned char recu;
unsigned char nb_rec;
/** P R I V A T E  P R O T O T Y P E S ***************************************/
void MyInterrupt(void);
void MyInterrupt_L(void);
unsigned char estSynchro(void);
void Init(void);
void Set_recepteur(unsigned char);

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
    MyInterrupt_L();
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
	

	
	if(INTCONbits.TMR0IF == 1){
		unsigned char data;

		INTCONbits.TMR0IF = 0;
		//WriteTimer0(0xffff - 2074); // avec un préscaler de 128,
									// on est à 180,7 Hz
		TMR0H=0xF7;
		TMR0L=0xE5;
		data =0;
		if(timer_desynchro > 0){
			timer_desynchro--;
		}
		
		// incrément des compteurs
		timer_led++;
		
		if(synchro == 1){
			
			// T1 : Si on recoit une valeur, vérifier que la valeur est fiable
			tab_reception[id_recepteur] = 0;
			
			if(recu!=0){ // Si on a reçu quelquechose			
				// T12 : Vérifier que la valeur correspond à un identifiant balise
				if(recu == ID_BALISE_1 || recu == ID_BALISE_2){
					// Si on a un identifiant fiable, on le stocke.
					// T2 : Enregistrement de la valeur reçu
					tab_reception[id_recepteur] = recu;
				}
				recu=0;
			}
			
			// T3 : Changer le recepteur écouté
			id_recepteur++;
			if(id_recepteur == NB_MSG_TOTAL)
				id_recepteur = 0;
			//Set_recepteur(id_recepteur);
			{
				unsigned char _recepteur;
				_recepteur = id_recepteur ;
				while(_recepteur > NB_MESSAGES){
    				_recepteur = _recepteur - NB_MESSAGES;
				}
				// Ordre des récepteurs (TSOP)
				// TSOP 13, 9, 5, 1
				// TSOP 12, 8, 4, 0
				// TSOP 15, 11, 7, 3
				// TSOP 14, 10, 6, 2
				// Correspondance entre le numéro du récepteur et sa position géographique
				switch (_recepteur){
					case 0  : _recepteur =13;break;
					case 1  : _recepteur =9 ;break;
					case 2  : _recepteur =5 ;break;
					case 3  : _recepteur =1 ;break;
					case 4  : _recepteur =12;break;
					case 5  : _recepteur =8 ;break;
					case 6  : _recepteur =4 ;break;
					case 7  : _recepteur =0 ;break;
					case 8  : _recepteur =15;break;
					case 9  : _recepteur =11;break;
					case 10 : _recepteur =7 ;break;
					case 11 : _recepteur =3 ;break;
					case 12 : _recepteur =14;break;
					case 13 : _recepteur =10;break;
					case 14 : _recepteur =6 ;break;
					case 15 : _recepteur =2 ;break;
				}
				// Choix Mux
				if( _recepteur == 2 || _recepteur == 3 ||
					_recepteur == 6 || _recepteur == 7 ||
					_recepteur == 10 || _recepteur == 11 ||
					_recepteur == 14 || _recepteur == 15 ){
					
					LATAbits.LATA3 = 0;
				}else{
					LATAbits.LATA3 = 1;
				}
				// In0
				if( _recepteur == 11 || _recepteur == 9){
					LATAbits.LATA2 = 0;
					LATAbits.LATA1 = 0;
					LATAbits.LATA0 = 0;
					
					LATBbits.LATB7 = 0;
					LATBbits.LATB6 = 0;
					LATBbits.LATB5 = 0;
				}
				// In1
				if( _recepteur == 7 || _recepteur == 5){
					LATAbits.LATA2 = 0;
					LATAbits.LATA1 = 0;
					LATAbits.LATA0 = 1;
					
					LATBbits.LATB7 = 0;
					LATBbits.LATB6 = 0;
					LATBbits.LATB5 = 1;
				}
				// In2
				if( _recepteur == 3 || _recepteur == 0){
					LATAbits.LATA2 = 0;
					LATAbits.LATA1 = 1;
					LATAbits.LATA0 = 0;
					
					LATBbits.LATB7 = 0;
					LATBbits.LATB6 = 1;
					LATBbits.LATB5 = 0;
				}
				// In3
				if( _recepteur == 2 || _recepteur == 13){
					LATAbits.LATA2 = 0;
					LATAbits.LATA1 = 1;
					LATAbits.LATA0 = 1;
					
					LATBbits.LATB7 = 0;
					LATBbits.LATB6 = 1;
					LATBbits.LATB5 = 1;
				}
				// In4
				if( _recepteur == 14 || _recepteur == 12){
					LATAbits.LATA2 = 1;
					LATAbits.LATA1 = 0;
					LATAbits.LATA0 = 0;
					
					LATBbits.LATB7 = 1;
					LATBbits.LATB6 = 0;
					LATBbits.LATB5 = 0;
				}
				// In5
				if( _recepteur == 15 || _recepteur == 1){
					LATAbits.LATA2 = 1;
					LATAbits.LATA1 = 0;
					LATAbits.LATA0 = 1;
					
					LATBbits.LATB7 = 1;
					LATBbits.LATB6 = 0;
					LATBbits.LATB5 = 1;
				}
				// In6
				if( _recepteur == 10 || _recepteur == 8){
					LATAbits.LATA2 = 1;
					LATAbits.LATA1 = 1;
					LATAbits.LATA0 = 0;
					
					LATBbits.LATB7 = 1;
					LATBbits.LATB6 = 1;
					LATBbits.LATB5 = 0;
				}
				// In7
				if( _recepteur == 6 || _recepteur == 4){
					LATAbits.LATA2 = 1;
					LATAbits.LATA1 = 1;
					LATAbits.LATA0 = 1;
					
					LATBbits.LATB7 = 1;
					LATBbits.LATB6 = 1;
					LATBbits.LATB5 = 1;
				}
			}
			
			// T4 : Emettre un top pour lancer le calcul
			if(id_recepteur == 0 || id_recepteur == NB_MESSAGES || id_recepteur == 2 * NB_MESSAGES){
				active_calcul = 1;
			}
			
			
		}else{
			timer_init++;
		}
		
		
	}
	if(PIR1bits.RCIF == 1){
		recu = RCREG;
		// T111 : Framing Error
		if(RCSTAbits.FERR){ // Gestion des erreur de trame série.
			recu = RCREG; // Effacement de l'erreur
			recu = 0;
		}
		// T112 : Overun Error
		if(RCSTAbits.OERR){
			recu=0; // La donnée n'est pas pertinente
			RCSTAbits.CREN = 0; // Réinitialisation du module
			RCSTAbits.CREN = 1; // de réception série
		}
		
		// Syncronisation permanente
		if ( (recu == ID_BALISE_1) ){
//			WriteTimer0(0xffff - 518); 
			TMR0H=0xFD;
			TMR0L=0xF9;
			timer_desynchro = T_MAX;
		}
	}


	PRODL = sauv1;
	PRODH = sauv2;		

}

#pragma interrupt MyInterrupt_L
void MyInterrupt_L(void){
	// Communication I2C
	com_i2c();
}


#pragma code




void main(void){
	unsigned char id_balise;
	unsigned char i;
	char envoi[32];

	
	// P1 : Initialisation
    Init();
    // P2 Traitement des données.
	while(1){
		unsigned char amas_taille=0;
		unsigned char amas_taille_old;
		unsigned char amas_pos;
		unsigned char amas_balise;
		unsigned char amas_balise_old;
		unsigned char mot_balise;
		
		if ( (timer_desynchro == 0) /*&& (synchro == 1)*/ ){
			synchro = 0;
			active_calcul = 0;
			for (i=0;i<6;i++){
				envoi[i]=88;
			}
			envoi_i2c(envoi);
		}
		
		if (synchro == 0){
			// Cas déespéré ! 
			Set_recepteur(0); // Celui en face du détrompeur du PIC
			// Désactivation des interruptions pour la liaison série
			PIE1bits.RCIE = 0; // Interruption inactive
			while (synchro == 0){
				synchro = estSynchro();
			}
			timer_desynchro = T_MAX;
			PIE1bits.RCIE = 1; // Interruption active
		}
		
		
		// P21 : Attendre qu'une balise ait fini d'émettre sa trame.
		if(active_calcul == 1){
			active_calcul=0;
			// P22 : Recopier les données dans l'espace de travail
			// On trouve la balise qui vient d'être lu :
			id_balise = id_recepteur / NB_MESSAGES;
			if(id_balise == 0){
				id_balise = 3;
			}
			id_balise--; // On a un identifiant balise compris entre 0 et 2;
			for(i=0;i<NB_MESSAGES;i++){
				tab_traitement[i]=tab_reception[i+id_balise*NB_MESSAGES];
				if(i<NB_MESSAGES/2){
					tab_traitement[i+NB_MESSAGES]=tab_traitement[i];
				}
			}
			// Pré-traitement
			// Si un récepteur un récepteur ne reçoit pas de signal alors que les deux encadrant reçoivent,
			// C'est une anomalie qu'il faut corriger.
			for(i=0;i<(NB_MESSAGES + NB_MESSAGES/2) ;i++){
				unsigned char prec, suiv;
				if(i == 0){
					prec = NB_MESSAGES-1;
				}else{
					prec = i - 1;
				}
				if (i == (NB_MESSAGES + NB_MESSAGES/2)){
					suiv = NB_MESSAGES/2 +1;
				}else{
					suiv = i+1;
				}
				if  (tab_traitement[prec] == tab_traitement[suiv]){
					tab_traitement[i] = tab_traitement[prec];
				}
			}
			// P23 : Touver l'amas le plus gros
			amas_pos=0;
			amas_taille=0;
			amas_taille_old=0;
			amas_balise=0;
			for(i=0;i<(NB_MESSAGES + NB_MESSAGES/2) ;i++){
				// On parcourt le tableau
				if(tab_traitement[i] != 0){
					// Si on a reçu une valeur valide
					if(amas_taille == 0){
						// Si aucun amas n'était commencé
						// On commence un nouvel amas
						amas_taille++;
						amas_balise = tab_traitement[i];
					}else{
						// Si un amas était commencé
						if(amas_balise == tab_traitement[i]){
							// Si on reçoit la même balise
							// On grossit l'amas
							amas_taille++;
						}else{
							// Sinon, l'amas est terminé
							// Comparaison avec l'amas précédent
							// On garde le plus gros
							if(amas_taille > amas_taille_old){
								amas_taille_old = amas_taille;
								amas_balise_old = amas_balise;
								amas_pos = 2*i - amas_taille_old;
							}
							// On commence un nouvel amas
							amas_taille = 1;
							amas_balise = tab_traitement[i];
						}
					}
				}else if(amas_taille != 0){
					// Si on n'a rien reçu de valide, l'amas est terminé
					// Comparaison avec l'amas précédent
					// On garde le plus gros
					if(amas_taille > amas_taille_old){
						amas_taille_old = amas_taille;
						amas_balise_old = amas_balise;
						amas_pos = 2*i - amas_taille_old;
					}
					// On n'a plus d'amas
					amas_taille = 0;
					amas_balise = 0;
				}
			}
			
			if(amas_taille > amas_taille_old){
				amas_taille_old = amas_taille;
				amas_balise_old = amas_balise;
				amas_pos = 2*i - amas_taille_old;
			}
			amas_pos -= 1;
			// P24 : déduction de l'angle
			// On a fait notre recherche sur 1 tour et demi,
			// On veut un angle sur 1 tour
			// On a un angle qui varie entre 0 et 31,
			// 1 bit = 11,25 degrés.
			// Une donnée sur 5 bits.
			if( amas_pos >= 2*NB_MESSAGES){
				amas_pos -= 2*NB_MESSAGES;
			}
			if(amas_pos > 2*NB_MESSAGES){
				amas_pos=0;			
			}
			
			// P25 : construction du message concernant la balise
			/*mot_balise = 0;
			mot_balise = (amas_pos & 0x1F) | ((amas_taille_old & 0x0F)<<3);
			envoi[0 + id_balise *2] = amas_taille_old;
			envoi[1 + id_balise *2] = amas_pos;*/
			if (id_balise == 0){
				for (i=0;i<16;i++){
					if (tab_traitement[i] == ID_BALISE_1){
						envoi[i] = 1;
					}else{
						envoi[i] = 0;
					}
				}
			}else if(id_balise == 1){
				for (i=16;i<32;i++){
					if (tab_traitement[i-16] == ID_BALISE_2){
						envoi[i] = 2;
					}else{
						envoi[i] = 0;
					}
				}
			}
			envoi_i2c(envoi);
			
		}

    }

}

void Init(){
	// P11 Initialisaiton des modules
	unsigned char data;
	char envoi[6];
	unsigned char i;
	nb_rec=0;
  


	// Activation des interruptions
   	INTCONbits.GIEH = 1; // Activation interruptions hautes
  	INTCONbits.GIEL = 1; // Activation interruptions basses
  	RCONbits.IPEN=1; // Activation des niveau d'interruptions


 	// Initialisation de l'i2c
	init_i2c(0x41);
	for (i=0;i<6;i++){
		envoi[i]=0;
	}

	while(!envoi_i2c(envoi));
	
  	// Configuration des entrées-sorties
  	TRISAbits.TRISA0=0;
  	TRISAbits.TRISA1=0;
  	TRISAbits.TRISA2=0;
  	TRISAbits.TRISA3=0;
  	TRISBbits.TRISB7=0;
  	TRISBbits.TRISB6=0;
  	TRISBbits.TRISB5=0;
  	
  	
  	// Initialisation de l'UART
  	TRISCbits.TRISC6 = 1;
	TRISCbits.TRISC7 = 1;
	RCSTAbits.CREN  = 1; // Activation de la réception
	TXSTAbits.BRGH = 0; // Gestion de la base de temps
	BAUDCONbits.BRG16 = 1; // Gestion de la base de temps
	SPBRGH = 3; // Gestion de la base de temps => 0x0361 = 829
	SPBRG = 61; // Gestion de la base de temps
	RCSTAbits.SPEN=1;
  	
    // Initialisation du Timer 0 pour la base de temps
	synchro = 0;
	OpenTimer0(	TIMER_INT_ON &  // interruption ON
				T0_16BIT &		// Timer 0 en 16 bits
				T0_SOURCE_INT & // Source interne (Quartz + PLL)
				T0_PS_1_32);		// 128 cycle, 1 incrémentation
	WriteTimer0(0xffff - 2074);
	timer_init=0;
	

	data=0;
	
	// P12 : Synchroniser la balise
	
	// P121 : Choix du recepteur à écouter
  	Set_recepteur(0); // Celui en face du détrompeur du PIC
	
	while(synchro == 0){
		synchro = estSynchro();
	}
	timer_desynchro = T_MAX;
	// Activation des interruption pour la liaison série
	IPR1bits.RCIP = 1; // Interruption haute
	PIE1bits.RCIE = 1; // Interruption active
	
	timer_led = 0;

	
}

void Set_recepteur(unsigned char _recepteur){
	// Ordre des récepteurs (TSOP)
	// TSOP 13, 9, 5, 1
	// TSOP 12, 8, 4, 0
	// TSOP 15, 11, 7, 3
	// TSOP 14, 10, 6, 2
	switch (_recepteur){
		case 0  : _recepteur =13;break;
		case 1  : _recepteur =9 ;break;
		case 2  : _recepteur =5 ;break;
		case 3  : _recepteur =1 ;break;
		case 4  : _recepteur =12;break;
		case 5  : _recepteur =8 ;break;
		case 6  : _recepteur =4 ;break;
		case 7  : _recepteur =0 ;break;
		case 8  : _recepteur =15;break;
		case 9  : _recepteur =11;break;
		case 10 : _recepteur =7 ;break;
		case 11 : _recepteur =3 ;break;
		case 12 : _recepteur =14;break;
		case 13 : _recepteur =10;break;
		case 14 : _recepteur =6 ;break;
		case 15 : _recepteur =2 ;break;
	}
	// Choix Mux
	if( _recepteur == 2 || _recepteur == 3 ||
		_recepteur == 6 || _recepteur == 7 ||
		_recepteur == 10 || _recepteur == 11 ||
		_recepteur == 14 || _recepteur == 15 ){
		
		LATAbits.LATA3 = 0;
	}else{
		LATAbits.LATA3 = 1;
	}
	// In0
	if( _recepteur == 11 || _recepteur == 9){
		LATAbits.LATA0 = 0;
		LATAbits.LATA1 = 0;
		LATAbits.LATA2 = 0;
		
		LATBbits.LATB7 = 0;
		LATBbits.LATB6 = 0;
		LATBbits.LATB5 = 0;
	}
	// In1
	if( _recepteur == 7 || _recepteur == 5){
		LATAbits.LATA0 = 0;
		LATAbits.LATA1 = 0;
		LATAbits.LATA2 = 1;
		
		LATBbits.LATB7 = 0;
		LATBbits.LATB6 = 0;
		LATBbits.LATB5 = 1;
	}
	// In2
	if( _recepteur == 3 || _recepteur == 0){
		LATAbits.LATA0 = 0;
		LATAbits.LATA1 = 1;
		LATAbits.LATA2 = 0;
		
		LATBbits.LATB7 = 0;
		LATBbits.LATB6 = 1;
		LATBbits.LATB5 = 0;
	}
	// In3
	if( _recepteur == 2 || _recepteur == 13){
		LATAbits.LATA0 = 0;
		LATAbits.LATA1 = 1;
		LATAbits.LATA2 = 1;
		
		LATBbits.LATB7 = 0;
		LATBbits.LATB6 = 1;
		LATBbits.LATB5 = 1;
	}
	// In4
	if( _recepteur == 14 || _recepteur == 12){
		LATAbits.LATA0 = 1;
		LATAbits.LATA1 = 0;
		LATAbits.LATA2 = 0;
		
		LATBbits.LATB7 = 1;
		LATBbits.LATB6 = 0;
		LATBbits.LATB5 = 0;
	}
	// In5
	if( _recepteur == 15 || _recepteur == 1){
		LATAbits.LATA0 = 1;
		LATAbits.LATA1 = 0;
		LATAbits.LATA2 = 1;
		
		LATBbits.LATB7 = 1;
		LATBbits.LATB6 = 0;
		LATBbits.LATB5 = 1;
	}
	// In6
	if( _recepteur == 10 || _recepteur == 8){
		LATAbits.LATA0 = 1;
		LATAbits.LATA1 = 1;
		LATAbits.LATA2 = 0;
		
		LATBbits.LATB7 = 1;
		LATBbits.LATB6 = 1;
		LATBbits.LATB5 = 0;
	}
	// In7
	if( _recepteur == 6 || _recepteur == 4){
		LATAbits.LATA0 = 1;
		LATAbits.LATA1 = 1;
		LATAbits.LATA2 = 1;
		
		LATBbits.LATB7 = 1;
		LATBbits.LATB6 = 1;
		LATBbits.LATB5 = 1;
	}
}

unsigned char estSynchro(void){
    unsigned char data;
		// P122 : Recevoir une lecture valide sur le port série
		if(RCSTAbits.FERR){ // Gestion des erreur de trame série.
			data = RCREG; // Effacement de l'erreur
			data = 0;
		}
		// Overun Error
		if(RCSTAbits.OERR){
				data=0; // La donnée n'est pas pertinente
				RCSTAbits.CREN = 0; // Réinitialisation du module
				RCSTAbits.CREN = 1; // de réception série
		}
		if(PIR1bits.RCIF){ // Si on a reçu quelquechose			
			data = RCREG;  // On lit la donnée
			// Vérifier que la valeur correspond à un identifiant balise
			if(data == ID_BALISE_1){
				nb_rec++;
				timer_init=0;
			}
		}
		// P124 : Fin du minuteur
		if(timer_init > 2){
			nb_rec=0;
			timer_init=0;
		}
		
		// P125 : On est bon
		if(nb_rec == NB_MESSAGES){
			WriteTimer0(0xffff - 1036); // On attend 1/2 période pour se mettre au début du message suivant
			id_recepteur = NB_MESSAGES - 1; // On est encore sur le dernier récepteur de la 1ere balise !
			                          // On veut recevoir la deuxième balise dès la prochaine interruption
      nb_rec = 0;              
			return 1;
		}
		return 0;
}










