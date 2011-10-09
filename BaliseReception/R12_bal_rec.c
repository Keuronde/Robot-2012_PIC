#include <p18cxxx.h>
#include <timers.h>
#include <pwm.h>

/** D E F I N E D ********************************************************/
// Identifiant balise
#define ID_BALISE_1 0x44
#define ID_BALISE_2 0x25
// Emission IR
#define NB_MESSAGES 16
#define NB_MSG_TOTAL 48
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
volatile unsigned char synchro;
volatile unsigned char recu;
/** P R I V A T E  P R O T O T Y P E S ***************************************/
void MyInterrupt(void);
void MyInterrupt_L(void);
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
				// Choix Mux
				if( _recepteur == 2 || _recepteur == 3 ||
					_recepteur == 6 || _recepteur == 7 ||
					_recepteur == 10 || _recepteur == 11 ||
					_recepteur == 14 || _recepteur == 15 ){
					
					PORTAbits.RA3 = 0;
				}else{
					PORTAbits.RA3 = 1;
				}
				// In0
				if( _recepteur == 11 || _recepteur == 9){
					PORTAbits.RA0 = 0;
					PORTAbits.RA1 = 0;
					PORTAbits.RA2 = 0;
					
					PORTBbits.RB7 = 0;
					PORTBbits.RB6 = 0;
					PORTBbits.RB5 = 0;
				}
				// In1
				if( _recepteur == 7 || _recepteur == 5){
					PORTAbits.RA0 = 0;
					PORTAbits.RA1 = 0;
					PORTAbits.RA2 = 1;
					
					PORTBbits.RB7 = 0;
					PORTBbits.RB6 = 0;
					PORTBbits.RB5 = 1;
				}
				// In2
				if( _recepteur == 3 || _recepteur == 0){
					PORTAbits.RA0 = 0;
					PORTAbits.RA1 = 1;
					PORTAbits.RA2 = 0;
					
					PORTBbits.RB7 = 0;
					PORTBbits.RB6 = 1;
					PORTBbits.RB5 = 0;
				}
				// In3
				if( _recepteur == 1 || _recepteur == 13){
					PORTAbits.RA0 = 0;
					PORTAbits.RA1 = 1;
					PORTAbits.RA2 = 1;
					
					PORTBbits.RB7 = 0;
					PORTBbits.RB6 = 1;
					PORTBbits.RB5 = 1;
				}
				// In4
				if( _recepteur == 14 || _recepteur == 12){
					PORTAbits.RA0 = 1;
					PORTAbits.RA1 = 0;
					PORTAbits.RA2 = 0;
					
					PORTBbits.RB7 = 0;
					PORTBbits.RB6 = 0;
					PORTBbits.RB5 = 0;
				}
				// In5
				if( _recepteur == 15 || _recepteur == 1){
					PORTAbits.RA0 = 1;
					PORTAbits.RA1 = 0;
					PORTAbits.RA2 = 1;
					
					PORTBbits.RB7 = 1;
					PORTBbits.RB6 = 0;
					PORTBbits.RB5 = 1;
				}
				// In6
				if( _recepteur == 10 || _recepteur == 8){
					PORTAbits.RA0 = 1;
					PORTAbits.RA1 = 1;
					PORTAbits.RA2 = 0;
					
					PORTBbits.RB7 = 0;
					PORTBbits.RB6 = 1;
					PORTBbits.RB5 = 1;
				}
				// In7
				if( _recepteur == 6 || _recepteur == 4){
					PORTAbits.RA0 = 1;
					PORTAbits.RA1 = 1;
					PORTAbits.RA2 = 1;
					
					PORTBbits.RB7 = 1;
					PORTBbits.RB6 = 1;
					PORTBbits.RB5 = 1;
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
		if(recu == ID_BALISE_1){
//			WriteTimer0(0xffff - 518); 
			TMR0H=0xFD;
			TMR0L=0xF9;
		}
	}


	PRODL = sauv1;
	PRODH = sauv2;		

}

#pragma interrupt MyInterrupt_L
void MyInterrupt_L(void){

}


#pragma code




void main(void){
	unsigned char t_diode;
	unsigned char id_balise;
	unsigned char i;
	
	// P1 : Initialisation
    Init();
    t_diode = F_1HZ;
    TRISBbits.TRISB0 = 0; //Sortie
    
    // P2 Traitement des données.
    while(1){
		unsigned char amas_taille=0;
		unsigned char amas_taille_old=0;
		unsigned char amas_pos;
		unsigned char amas_balise;
		unsigned char amas_balise_old;
		unsigned char mot_balise;
		// Clognottement LED
		if(timer_led > t_diode){
			PORTCbits.RC1 = !PORTCbits.RC1;
			timer_led = 0;
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
					tab_traitement[i+NB_MESSAGES]=tab_reception[i+id_balise*NB_MESSAGES];
				}
			}
			// P23 : Touver l'amas le plus gros
			amas_pos=0;
			amas_taille=0;
			amas_balise=0;
			for(i=0;i<(NB_MESSAGES + NB_MESSAGES/2) ;i++){
				if(tab_traitement[i] != 0){
					if(amas_taille == 0){
						amas_taille++;
						amas_balise = tab_traitement[i];
					}else{
						if(amas_balise == tab_traitement[i-1]){
							amas_taille++;
						}else{
							// Comparaison avec l'amas précédent
							// On garde le plus gros
							if(amas_taille > amas_taille_old){
								amas_taille_old = amas_taille;
								amas_balise_old = amas_balise;
								amas_pos = 2*i - amas_taille_old;
							}
							amas_taille = 1;
							amas_balise = tab_traitement[i];
						}
					}
				}else if(amas_taille != 0){
					if(amas_taille > amas_taille_old){
						amas_taille_old = amas_taille;
						amas_balise_old = amas_balise;
						
					}
					amas_taille = 0;
					amas_balise = 0;
				}
			}
			if(amas_taille > amas_taille_old){
				amas_taille_old = amas_taille;
				amas_balise_old = amas_balise;
				amas_pos = 2*i - amas_taille_old;
			}
			amas_pos -= 2;
			
			// P24 : déduction de l'angle
			// On a fait notre recherche sur 1 tour et demi,
			// On veut un angle sur 1 tour
			// On a un angle qui varie entre 0 et 31,
			// 1 bit = 11,25 degrés.
			// Une donnée sur 5 bits.
			if( amas_pos >= 2*NB_MESSAGES){
				amas_pos -= 2*NB_MESSAGES;
			}
			
			// P25 : construction du message concernant la balise
			mot_balise = 0;
			mot_balise = (amas_pos & 0x1F) | ((amas_taille_old & 0x0F)<<3);

			// Si on est sur la balise 1 et qu'on a rien reçu, on passe en mode pannique
			if(id_balise == 0){
				if(amas_taille_old != 24){
					t_diode = F_5HZ;
				}else{
					t_diode = F_1HZ;
				}
			}
		
			
		}

    }

}

void Init(){
	// P11 Initialisaiton des modules
	char nb_rec=0;
	unsigned char data;
	// Activation des interruptions
   	INTCONbits.GIEH = 1; // Activation interruptions hautes
  	INTCONbits.GIEL = 1; // Activation interruptions basses
  	RCONbits.IPEN=1; // Activation des niveau d'interruptions
  	
  	
  	// Configuration des entrées-sorties
  	TRISAbits.TRISA0=0;
  	TRISAbits.TRISA1=0;
  	TRISAbits.TRISA2=0;
  	TRISAbits.TRISA3=0;
  	TRISBbits.TRISB0=0;
  	TRISBbits.TRISB1=0;
  	TRISBbits.TRISB2=0;
  	
  	
  	
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
	
	// P12 : Synchroniser la balise
	
	// P121 : Choix du recepteur à écouter
  	Set_recepteur(13); // Celui en face du détrompeur du PIC
	
	while(synchro == 0){
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
			WriteTimer0(0xffff - 518); // On attend 1/4 de période pour se mettre au milieu du creux entre deux messages
			id_recepteur = 15; // On est encore sur le dernier récepteur de la 1ere balise !
			                   // C'est à l'interuption suivante qu'on commencera la lecture de 1er récepteur de la balise 2
			synchro=1;
		}
		
	}
	
	// Activation des interruption pour la liaison série
	IPR1bits.RCIP = 1; // Interruption haute
	PIE1bits.RCIE = 1; // Interruption active
	
	timer_led = 0;
	
	
	

	
	
	
	
	// On allume la LED
	TRISCbits.TRISC1 = 0;
	PORTCbits.RC1 = 1;
	
}

void Set_recepteur(unsigned char _recepteur){
	// Ordre des récepteurs (TSOP)
	// TSOP 13, 9, 5, 1
	// TSOP 12, 8, 4, 0
	// TSOP 15, 11, 7, 3
	// TSOP 14, 10, 6, 2
	// Choix Mux
	if( _recepteur == 2 || _recepteur == 3 ||
		_recepteur == 6 || _recepteur == 7 ||
		_recepteur == 10 || _recepteur == 11 ||
		_recepteur == 14 || _recepteur == 15 ){
		
		PORTAbits.RA3 = 0;
	}else{
		PORTAbits.RA3 = 1;
	}
	// In0
	if( _recepteur == 11 || _recepteur == 9){
		PORTAbits.RA0 = 0;
		PORTAbits.RA1 = 0;
		PORTAbits.RA2 = 0;
		
		PORTBbits.RB7 = 0;
		PORTBbits.RB6 = 0;
		PORTBbits.RB5 = 0;
	}
	// In1
	if( _recepteur == 7 || _recepteur == 5){
		PORTAbits.RA0 = 0;
		PORTAbits.RA1 = 0;
		PORTAbits.RA2 = 1;
		
		PORTBbits.RB7 = 0;
		PORTBbits.RB6 = 0;
		PORTBbits.RB5 = 1;
	}
	// In2
	if( _recepteur == 3 || _recepteur == 0){
		PORTAbits.RA0 = 0;
		PORTAbits.RA1 = 1;
		PORTAbits.RA2 = 0;
		
		PORTBbits.RB7 = 0;
		PORTBbits.RB6 = 1;
		PORTBbits.RB5 = 0;
	}
	// In3
	if( _recepteur == 1 || _recepteur == 13){
		PORTAbits.RA0 = 0;
		PORTAbits.RA1 = 1;
		PORTAbits.RA2 = 1;
		
		PORTBbits.RB7 = 0;
		PORTBbits.RB6 = 1;
		PORTBbits.RB5 = 1;
	}
	// In4
	if( _recepteur == 14 || _recepteur == 12){
		PORTAbits.RA0 = 1;
		PORTAbits.RA1 = 0;
		PORTAbits.RA2 = 0;
		
		PORTBbits.RB7 = 0;
		PORTBbits.RB6 = 0;
		PORTBbits.RB5 = 0;
	}
	// In5
	if( _recepteur == 15 || _recepteur == 1){
		PORTAbits.RA0 = 1;
		PORTAbits.RA1 = 0;
		PORTAbits.RA2 = 1;
		
		PORTBbits.RB7 = 1;
		PORTBbits.RB6 = 0;
		PORTBbits.RB5 = 1;
	}
	// In6
	if( _recepteur == 10 || _recepteur == 8){
		PORTAbits.RA0 = 1;
		PORTAbits.RA1 = 1;
		PORTAbits.RA2 = 0;
		
		PORTBbits.RB7 = 0;
		PORTBbits.RB6 = 1;
		PORTBbits.RB5 = 1;
	}
	// In7
	if( _recepteur == 6 || _recepteur == 4){
		PORTAbits.RA0 = 1;
		PORTAbits.RA1 = 1;
		PORTAbits.RA2 = 1;
		
		PORTBbits.RB7 = 1;
		PORTBbits.RB6 = 1;
		PORTBbits.RB5 = 1;
	}
}











