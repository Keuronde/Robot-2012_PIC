/*****************************************************************************
*  Ce fichier avec i2c_s.c doit permettre à un PIC escalve I2C de d'envoyer  *
*  et de recevoir des informations de la part du pic maitre.                 *
*  Hypothèses :                                                              *
*  La communication est toujours structurée de la même manière :             *
*    Le maitre envoi l'adresse                                               *
*    Suivant le bit R/W,                                                     *
*      Le maitre envoi un nombre d'octets défini à la compilation            *
*      Le maitre reçoit un nombre d'octets définit à la conpilation          *
*  Le nombre d'octets reçus et envoyés doit pouvoir être facilement          *
*  parametrable.                                                             *
*                                                                            *
******************************************************************************/

/** P U B L I C   P R O T O T Y P E S ***/
void init_i2c();
char rec_i2c(char *);
char envoi_i2c(char *);
void com_i2c(void);

enum i2c_flag {
    I2C_INACTIF=0,
    I2C_ACTIF
};
enum data_flag {
    PERIME=0,
    ACTUALISE
};
