/************************************************
* Ce fichier fournit la déclaration des 		*
* fonctions pour lire la mémoire EEPROM du		*
* microcontroleur. Ces fonctions ont été 		*
* sommairement testées et semblent marcher. 	*
************************************************/


/** P U B L I C   P R O T O T Y P E S ***********/
void EE_w(unsigned char _Adr,unsigned char _data);
unsigned char EE_r(unsigned char);
