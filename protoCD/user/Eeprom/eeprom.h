/************************************************
* Ce fichier fournit la d�claration des 		*
* fonctions pour lire la m�moire EEPROM du		*
* microcontroleur. Ces fonctions ont �t� 		*
* sommairement test�es et semblent marcher. 	*
************************************************/


/** P U B L I C   P R O T O T Y P E S ***********/
void EE_w(unsigned char _Adr,unsigned char _data);
unsigned char EE_r(unsigned char);
