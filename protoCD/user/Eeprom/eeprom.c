/************************************************
* Ce fichier fournit les foncitons pour lire la *
* m�moire EEPROM du microcontroleur. Ces 		*
* fonctions ont �t� sommairement test�es et 	*
* semblent marcher. 							*
************************************************/

/** I N L U D E S ******************************/
#include "eeprom.h"
#include <p18f2550.h>



void EE_w(unsigned char _Adr ,unsigned  char _data) {
	EECON1bits.EEPGD = 0;
	EECON1bits.CFGS = 0;
	EEADR = _Adr;
	EEDATA = _data;
	EECON1bits.WREN = 1;
	EECON2 = 0x55;
	EECON2 = 0x0AA;
	EECON1bits.WR = 1;
	while(EECON1bits.WR){ _asm nop _endasm;}
}
unsigned char EE_r(unsigned char _Adr) {
	EECON1bits.EEPGD = 0;
	EECON1bits.CFGS = 0;
	EEADR = _Adr;
	EECON1bits.RD = 1;
 	return EEDATA;
}