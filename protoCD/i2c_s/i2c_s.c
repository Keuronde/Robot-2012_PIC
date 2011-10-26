/*****************************************************************************
*  Ce fichier avec i2c_s.h doit permettre à un PIC escalve I2C de d'envoyer  *
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


/** I N C L U D E S **********************************************************/
#include "i2c_s/i2c_s.h"

/* P A R A M E T R E S *******************************************************/
#define NB_REC 1
#define NB_ENV 3


/** VARIABLES GLOBALES ***/
char data_out[NB_ENV];
char data_in[NB_REC];
char data_index;

data_flag _reception = PERIME;
i2c_flag _i2c_flag = I2C_INACTIF;

/*
Fonction rec_i2c

Fonction permettant de récupérer le tampon de réception i2c.
Cette fonction prend en argument l'adresse de la chaine de caractere de destination
Cette fonction renvoi 0 si une reception est en cours, 1 si tout s'est bien passé.

*/
char rec_i2c(char *chaine){
    // Afin de ne pas recevoir le début de la trame n et la fin de la trame n+1
    if(_i2c_flag == I2C_ACTIF){
        // s'il s'agit d'une reception
        if(SSPSTATbits.RW == 1){
            // On ne fait rien
            return 0;
        }
    }
    memcpy( (void*) chaine, (void*) data_in, NB_REC );
    _reception = PERIME; // On se note que nous avons lu ces données. 
    return 1;
}


/*
Fonction envoi_i2c

Fonction permettant de mettre dans le tampon d'envoi i2c une chaine de caractère.
Cette fonction prend en argument l'adresse de la chaine de caractere à envoyer
Cette fonction renvoi 0 si une émission est en cours, 1 si tout s'est bien passé.

*/

char envoi_i2c(char *chaine){
    // Afin de ne pas envoyer le début de la trame n et la fin de la trame n+1
    // Si une transaction i2c est en cours
    if(_i2c_flag == I2C_ACTIF){
        // s'il s'agit d'un envoi
        if(SSPSTATbits.RW == 0){
            // On ne fait rien
            return 0;
        }
    }
    memcpy( (void*) data_out, (void*) chaine, NB_ENV );
    return 1;
}

// Fonction en interruption
void com_i2c(){
    if(PIR1bits.SSPIF == 1){
        // Réinitialisation de l'interruption
        PIR1bits.SSPIF = 0;
        
        // On vérifie qu'il n'y a pas d'erreur :
        // SSPOV = OVERFLOW => On vient de recevoir quelque chose, mais le tampon précédent n'a pas été lu.
        if(SSPCONbits.SSPOV == 1){
            char temp = SSPBUF; // Efface BF
            SSPCONbits.SSPOV = 0;
            return; // La fonction s'arrête la, 
        }
        
        //Adresse ou données ?
        if(SSPSTATbits.DA == 1){
            // Données
            
            // Reception ou émission ?
            if(SSPSTATbits.RW == 1){
                // Réception
                data[data_index] = SSPBUF;
                data_index++;
                // Dernier octet ? 
                if(data_index == NB_REC){
                    // Fin de transaction
                    _i2c_flag = I2C_INACTIF;
                    _reception = ACTUALISE;
                }
            }else{
                //Émission
                SSPBUF = data[data_index];
                data_index++;
                if(data_index == NB_REC){
                    // Fin de transaction
                    _i2c_flag = I2C_INACTIF;
                }
            }

        }else{
            // Adresse
            char temp = SSPBUF; // Pour eviter l'overflow, efface BF
            data_index = 0;     // On prépare la réception, ou l'émision
            // On se note qu'une transaction est en cours
            _i2c_flag = I2C_ACTIF;
        }
    }
}


