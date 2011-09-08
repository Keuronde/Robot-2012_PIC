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
volatile unsigned char tab_traitement[NB_MESSAGES * 1.5];
volatile unsigned char active_calcul;
volatile unsigned char synchro;
/** P R I V A T E  P R O T O T Y P E S ***************************************/
void MyInterrupt(void);
void MyInterrupt_L(void);
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
		unsigned char data=0;

		INTCONbits.TMR0IF = 0;
		WriteTimer0(0xffff - 2074); // avec un préscaler de 128,
									// on est à 180,7 Hz
		// incrément des compteurs
		timer_led++;
		
		
		if(synchro == 1){
			
			// T1 : Si on recoit une valeur, vérifier que la valeur est fiable
			tab_reception[id_recepteur] = 0;
			// T111 : Framing Error
			if(RCSTAbits.FERR){ // Gestion des erreur de trame série.
				data = RCREG; // Effacement de l'erreur
				data = 0;
			}
			// T112 : Overun Error
			if(RCSTAbits.OERR){
					data=0; // La donnée n'est pas pertinente
					RCSTAbits.CREN = 0; // Réinitialisation du module
					RCSTAbits.CREN = 1; // de réception série
			}
			if(PIR1bits.RCIF){ // Si on a reçu quelquechose			
				data = RCREG;  // On lit la donnée
				// T12 : Vérifier que la valeur correspond à un identifiant balise
				if(data == ID_BALISE_1 || data == ID_BALISE_2){
					// Si on a un identifiant fiable, on le stocke.
					// T2 : Enregistrement de la valeur reçu
					tab_reception[id_recepteur] = data;
				}
			}
			
			// T3 : Changer le recepteur écouté
			id_recepteur++;
			if(id_recepteur > NB_MSG_TOTAL)
				id_recepteur = 0;
			Set_recepteur(id_recepteur);
			
			// T4 : Emettre un top pour lancer le calcul
			if(id_reception == 0 || id_reception == NB_MESSAGES || id_reception == 2 * NB_MESSAGES){
				active_calcul = 1;
			}
			
			
		}else{
			timer_init++;
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
	char t_diode;
	
	// P1 : Initialisation
    Init();
    t_diode = F_1HZ;
    
    // P2 Traitement des données.
    while(1){
		if(timer_led > t_diode){
			PORTCbits.RC1 = !PORTCbits.RC1;
			timer_led = 0;
		}

    }

}

void Init(){
	// P11 Initialisaiton des modules
	char nb_rec=0;
	// Activation des interruptions
   	INTCONbits.GIEH = 1; // Activation interruptions hautes
  	INTCONbits.GIEL = 1; // Activation interruptions basses
  	RCONbits.IPEN=1; // Activation des niveau d'interruptions
  	
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
	OpenTimer0(	TIMER_INT_ON &  // interruption ON
				T0_16BIT &		// Timer 0 en 16 bits
				T0_SOURCE_INT & // Source interne (Quartz + PLL)
				T0_PS_1_32);		// 128 cycle, 1 incrémentation
	WriteTimer0(0xffff - 2074);
	synchro = 0;
	timer_init=0;
	
	// P12 : Synchroniser la balise
	while(synchro == 0){
		// P121 : Recevoir une lecture valide sur le port série
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
		// P123 : Fin du minuteur
		if(timer_init > 2){
			nb_rec=0;
			timer_init=0;
		}
		
		// P124 : On est bon
		if(nb_rec == NB_MESSAGES){
			WriteTimer0(0xffff - 518); // On attend 1/4 de période pour se mettre au milieu du creux entre deux messages
			id_recepteur = 16;
		}
		
	}
	
	timer_led = 0;
	
	
	

	
	
	
	
	// On allume la LED
	TRISCbits.TRISC1 = 0;
	PORTCbits.RC1 = 1;
	
}













