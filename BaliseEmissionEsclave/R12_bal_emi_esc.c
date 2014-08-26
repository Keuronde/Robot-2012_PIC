#include <p18cxxx.h>
#include <timers.h>
#include <pwm.h>

/** D E F I N E D ********************************************************/
// Identifiant balise
#define ID_BALISE_1 0x44
#define ID_BALISE 0x25
// Emission IR
#define NB_MESSAGES 16
#define NB_MSG_TOTAL 48
// Durée de vie (10 secondes)
#define T_MAX ((unsigned int) 1807)
// Clignotement LED
#define F_1HZ 90
#define F_5HZ 18
// LED
#define TRIS_LED TRISBbits.TRISB7
#define LED LATBbits.LATB7
/** V A R I A B L E S ********************************************************/
#pragma udata
volatile unsigned char timer_led;
volatile unsigned char timer_emi;
volatile unsigned char timer_init;
volatile unsigned int  timer_desynchro;
volatile unsigned char synchro;
unsigned char nb_rec;
/** P R I V A T E  P R O T O T Y P E S ***************************************/
void MyInterrupt(void);
void MyInterrupt_L(void);
unsigned char estSynchro(void);
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
	// Adapté et modifié par Keuronde
	unsigned char sauv1;
	unsigned char sauv2;

	sauv1 = PRODL;
	sauv2 = PRODH;
	

	
	if(INTCONbits.TMR0IF == 1){
		INTCONbits.TMR0IF = 0;
		//WriteTimer0(0xffff - 2074); // avec un préscaler de 128,
									// on est à 180,7 Hz
		TMR0H=0xF7;
		TMR0L=0xE5;
		timer_led++;
		timer_init++;
    timer_emi++;
    if(timer_desynchro > 0){
      timer_desynchro--;
    }
		if(synchro == 1){
		  // Si on est synchronisé
//		  timer_emi++;
		  if(timer_emi == NB_MSG_TOTAL)
			  timer_emi=0;
		
		  // Emission UART
		  if(timer_emi < NB_MESSAGES){
			  TXREG = ID_BALISE;
		  }
		}else{
//		  timer_init++;
		}
		
		
	}
	
	if(PIR1bits.RCIF == 1){
	  char recu;
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

}


#pragma code




void main(void){
	char t_diode;
    Init();
    
    t_diode = F_1HZ;
    while(1){
      if (timer_desynchro == 0){
        synchro = 0;
        LED=0;
      }
      if (synchro == 0){
        //Désactivation de l'interruption RCIE
      	PIE1bits.RCIE = 0; // Interruption active
        while(synchro == 0){
		      synchro = estSynchro();
	      }
	    	timer_desynchro = T_MAX;
	    	PIE1bits.RCIE = 1; // Interruption active
	    }

      // Clignotement DEL
		  if( (timer_led > t_diode) && (synchro == 1) ){
			  LED = !LED;
			  timer_led = 0;
		  }

    }

}

void Init(){
  unsigned char data;
  nb_rec=0;
  // Initialisation des variables
  synchro = 0;
  timer_init=0;
  
  // Activation des interruptions
 	INTCONbits.GIEH = 1; // Activation interruptions hautes
  INTCONbits.GIEL = 1; // Activation interruptions basses
  RCONbits.IPEN=1; // Activation des niveau d'interruptions
  	
  // Initialisation du Timer 0 pour la base de temps
	OpenTimer0(	TIMER_INT_ON &  // interruption ON
				T0_16BIT &		// Timer 0 en 16 bits
				T0_SOURCE_INT & // Source interne (Quartz + PLL)
				T0_PS_1_32);		// 128 cycle, 1 incrémentation
	WriteTimer0(0xffff - 2074);
	timer_emi = 0;
	timer_led = 0;
	
	// Initialisation de l'UART
  TRISCbits.TRISC6 = 1;
  TRISCbits.TRISC7 = 1;
  TXSTAbits.TX9  = 0; // Mode 8 bits
  TXSTAbits.TXEN = 1; // Activation du module UART de transmission
  RCSTAbits.CREN = 1; // Activation de la réception
  TXSTAbits.BRGH = 0; // Gestion de la base de temps
  BAUDCONbits.BRG16 = 1; // Gestion de la base de temps
  BAUDCONbits.TXCKP = 1; // Inversion de la sortie série
  SPBRGH = 3; // Gestion de la base de temps => 0x0361 = 829
  SPBRG = 61; // Gestion de la base de temps
  RCSTAbits.SPEN=1; // Activation UART

	
	
	
	while(synchro == 0){
		synchro = estSynchro();
	}
	timer_desynchro = T_MAX;
	
	// Configuration de la MLI
	OpenTimer2( TIMER_INT_OFF & T2_PS_1_4 & T2_POST_1_1 );
	TRISBbits.TRISB3=0;
	OpenPWM2(82);
	SetDCPWM2((unsigned int)164);
	
	// Activation des interruption pour la liaison série
	IPR1bits.RCIP = 1; // Interruption haute
	PIE1bits.RCIE = 1; // Interruption active
	
	// On allume la LED
	TRIS_LED = 0;
	LED = 1;
	
	
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
			timer_emi = NB_MSG_TOTAL - 1; // On est encore sur le dernier récepteur de la 1ere balise !
			                          // On veut émettre notre premier message à la prochaine interruption
			                
			return 1;
		}
		return 0;
}









