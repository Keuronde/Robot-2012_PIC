/** I N C L U D E S **********************************************************/
#include <p18cxxx.h>
#include <string.h>
#include <stdio.h>
#include <delays.h>
#include <timers.h>
#include "system\typedefs.h"
#include "Servo\servo.h"
#include "system\usb\usb.h"

#include "user\user.h"



#define F_PRECISION 2
#define SERVO_INIT 0x460
/** V A R I A B L E S ********************************************************/
#pragma udata

char input_buffer[64];
char output_buffer[64];
char molette1;
char molette2;
char loop_active;



char sel = 0;
unsigned int OUVERT = 0x6FE;
unsigned int FERMEE = 0x4EF;
/** P R I V A T E  P R O T O T Y P E S ***************************************/
void InitializeUSART(void);
void nunchuck_read(unsigned char * input_buffer);

/** D E C L A R A T I O N S **************************************************/
#pragma code
void UserInit(void)
{
    LATA = 0;		// clear data latches
    TRISA = 0;		// direction: output pins
	TRISCbits.TRISC2 = 0;
	TRISAbits.TRISA1 = 0;
    InitializeUSART();   
	molette1 = 0;
	molette2 = 0;
	TRISBbits.TRISB7 = 1;
	TRISBbits.TRISB6 = 1;
	TRISCbits.TRISC2 = 0; // Output	
	PORTCbits.RC2 = 1;

/*
	// I2C INIT
	// Initialisation de l'I2C
	// Les pattes concernée avec TRIS à 1
	TRISB = TRISB | 0x03; // TRISB0 et TRISB1 à 1
	// I2C en mode MASTER
	SSPCON1 = 0x08; // 0b 0000 1000
	// Pour la fréquence, on utilise les 7 dernier bits de SSPADD
	// La formule semble être la suivante Fosc/2/(SSPADD +1)
	// à 48 MHz, 59 nous donne 400kHz
	SSPADD = 59;
	// Activation du SSP, module dont dépend l'I2C
	SSPCON1bits.SSPEN = 1;
	// On peut probablement faire ça d'un coups :
	// SSPCON1 = 0x28; // 0b 0010 1000


	// Initialisation du Timer 0
	OpenTimer0(	TIMER_INT_ON &  // interruption ON
				T0_16BIT &		// Timer 0 en 16 bits
				T0_SOURCE_INT & // Source interne (Quartz + PLL)
				T0_PS_1_1);		// 1 cycle, 1 incrémentation

	// On active toutes les interruptions
	INTCONbits.GIE = 1;
	INTCONbits.PEIE = 1;

	dt = 3;
	WriteTimer0(65535 - 36000); // dt * 1ms
	timer =0;
	loop_active=0;
	calcul = 0;



// numero des messages
	nb_msg =0;
*/

  Servo_Init();
}//end UserInit

void InitializeUSART(void)
{
    TRISCbits.TRISC7=1; // RX
    TRISCbits.TRISC6=0; // TX
    SPBRG = 0x71;
    SPBRGH = 0x02;      // 0x0271 for 48MHz -> 19200 baud
    TXSTA = 0x24;       // TX enable BRGH=1
    RCSTA = 0x90;       // continuous RX
    BAUDCON = 0x08;     // BRG16 = 1
}//end InitializeUSART

void ProcessIO(void)
{   
	static unsigned int cnt = 0;
	//char str[32]="\rC = X, Z = X \r\n";
	static int iteration;
	static char temps;
	char ram str[40]="\r"; //X, Z = X \r\n";
	char ram milieu[3]=" ";
	char ram fin[5]="\r\n";
	unsigned char nunchuk[6];
	char i,j;
	char input_buffer[32];
	static int blink = 1;


	if( (usb_device_state < CONFIGURED_STATE) || (UCONbits.SUSPND==1) ) return;


  if(getsUSBUSART(input_buffer,1)) // if a byte has been received from the computer
  {
    if(input_buffer[0] == 'y') { // On augmente de 10 la valeur du servo.
			if(sel == 1){
				OUVERT = OUVERT + 0x10;
				Servo_Set(OUVERT,0);
				sprintf(str,"OUVERT +10 : %x\r\n",(unsigned int)OUVERT );
				if(mUSBUSARTIsTxTrfReady())	putsUSBUSART(str);
			}else{
				FERMEE = FERMEE + 0x10;
				Servo_Set(FERMEE ,0);
				sprintf(str,"FERME +10 : %x\r\n",FERMEE );
				if(mUSBUSARTIsTxTrfReady())	putsUSBUSART(str);

			}
			return;
		}
        if(input_buffer[0] == 'u') { // On augmente de 10 la valeur du servo.
			if(sel == 1){
				OUVERT = OUVERT + 0x1;
				Servo_Set(OUVERT,0);
				sprintf(str,"OUVERT +1 : %x\r\n",OUVERT );
				if(mUSBUSARTIsTxTrfReady())	putsUSBUSART(str);
			}else{
				FERMEE = FERMEE + 0x1;
				Servo_Set(FERMEE,0);
				sprintf(str,"FERMEE +1 : %x\r\n",FERMEE );
				if(mUSBUSARTIsTxTrfReady())	putsUSBUSART(str);
			}
			return;
		}
		if(input_buffer[0] == 'i') { // On diminue de 10 la valeur du servo.
			if(sel == 1){
				OUVERT = OUVERT - 0x1;
				Servo_Set(OUVERT,0);
				sprintf(str,"OUVERT -1 : %x\r\n",OUVERT );
				if(mUSBUSARTIsTxTrfReady())	putsUSBUSART(str);

			}else{
				FERMEE = FERMEE - 0x1;
				Servo_Set(FERMEE,0);
				sprintf(str,"FERMEE -1 : %x\r\n",FERMEE );
				if(mUSBUSARTIsTxTrfReady())	putsUSBUSART(str);

			}
			return;
		}
		if(input_buffer[0] == 'o') { // On diminue de 10 la valeur du servo.
			if(sel == 1){
				OUVERT = OUVERT - 0x10;
				Servo_Set(OUVERT,0);
				sprintf(str,"OUVERT -10 : %x\r\n",OUVERT );
				if(mUSBUSARTIsTxTrfReady())	putsUSBUSART(str);

			}else{
				FERMEE = FERMEE - 0x10;
				Servo_Set(FERMEE,0);
				sprintf(str,"FERMEE -10 : %x\r\n",FERMEE );
				if(mUSBUSARTIsTxTrfReady())	putsUSBUSART(str);

			}
			return;
		}

		if(input_buffer[0] == 'p') { // On diminue de 10 la valeur du servo.
			if(sel == 1){
				OUVERT = OUVERT - 0x100;
				Servo_Set(OUVERT,0);
				sprintf(str,"OUVERT -100 : %x\r\n",OUVERT );
				if(mUSBUSARTIsTxTrfReady())	putsUSBUSART(str);

			}else{
				FERMEE = FERMEE - 0x100;
				Servo_Set(FERMEE,0);
				sprintf(str,"FERMEE -100 : %x\r\n",FERMEE );
				if(mUSBUSARTIsTxTrfReady())	putsUSBUSART(str);

			}
			return;
		}

		if(input_buffer[0] == 't') { // On diminue de 10 la valeur du servo.
			if(sel == 1){
				OUVERT = OUVERT + 0x100;
				Servo_Set(OUVERT,0);
				sprintf(str,"OUVERT +100 : %x\r\n",OUVERT );
				if(mUSBUSARTIsTxTrfReady())	putsUSBUSART(str);

			}else{
				FERMEE = FERMEE + 0x100;
				Servo_Set(FERMEE,0);
				sprintf(str,"FERMEE +100 : %x\r\n",FERMEE );
				if(mUSBUSARTIsTxTrfReady())	putsUSBUSART(str);

			}
			return;
		}

        if(input_buffer[0] == 'a') { // r (red) -> light the LED on RA0
			Servo_Set(FERMEE,0);
			sel =0;
			if(mUSBUSARTIsTxTrfReady())	putrsUSBUSART("\rFERMEE\r\n");
			return;
		}
		if(input_buffer[0] == 'z') { // r (red) -> light the LED on RA0
			Servo_Set(OUVERT,0);
			sel =1;
			if(mUSBUSARTIsTxTrfReady())	putrsUSBUSART("\rOUVERT\r\n");
			return;
		}
		if(input_buffer[0] == 'e') { // r (red) -> light the LED on RA0
			Servo_Set(0x51F,0);
			sel =1;
			if(mUSBUSARTIsTxTrfReady())	putrsUSBUSART("\rENTRE OUVERT\r\n");
			return;
		}

  }
	

}//end ProcessIO

/** EOF user.c ***************************************************************/
