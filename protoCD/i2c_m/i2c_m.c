/*****************************************************************************
*  Ce fichier avec i2c_m.h doit permettre à un PIC maitre I2C de d'envoyer   *
*  et de recevoir des informations à un périphérique esclave.                *
*  Hypothèses :                                                              *
*  La communication est toujours structurée de la même manière :             *
*    Le maitre envoi l'adresse de l'esclave pour une lecture                 *
*    Le maitre envoi un nombre d'octets prédéfini                            *
*    Le maitre envoi l'adresse de l'esclave pour une ecriture                *
*    Le maitre reçoit un nombre d'octets prédéfini                           *
*                                                                            *
******************************************************************************/



#define NB_MAX

char data_out[NB_MAX];
char data_in[NB_MAX];
char i2c_slave_adresse;
char data_index;
char nb_rec;
char nb_env;
i2c_m_flag etat_i2c = I2C_LIBRE; // Etat de notre i2c


/* 
La fonction à qui on donne :
* L'adresse de l'esclave
* Le nombre d'octet à envoyer
* Le nombre d'octet à recevoir
* L'adresse de la chaine à envoyer
* L'adresse de la chaine recevant les données
Elle renvoie :
* 0 si l'i2c est occupé
* 1 si la transaction à bien pu s'initialiser
*/

char transmission_i2c(char _adresse, char _nb_env, char _nb_rec, char *s_envoi){
// Faire les tests pour vérifier que la ligne est libre.
    if(_i2c_flag != I2C_LIBRE){
        return 0;
    }
    _i2c_flag = I2C_START;

// On prépare l'envoi
    nb_env = _nb_env;
    nb_rec = _nb_rec;
    i2c_slave_adresse = _adresse;
    memcpy( (void*) data_out, (void*) s_envoi, nb_env );

    // Envoyer un bit de début
    SSPCON2bits.SEN = 1; // 0b 0000 0001
    
    // Il faudra envoyer l'adresse
    _i2c_flag = I2C_ENV_ADRESSE;
    return 1;
}

/*
La fonction en interruption
Celle qui fait tout, sauf envoyer le bit de début
*/
void gestion_i2c(){
    static nacq = 1;
    // Si c'est bien une interruption I2C
    if(PIR1bits.SSPIF == 1){
        
        // On réinitialise le drapeau.
        PIR1bits.SSPIF == 0;
        
        // Si on attendait un NACK, on vérifie qu'il est bon
        if(nacq == 1){
            nacq = 0;
            if(SSPCON2bits.ACKSTAT){
                // En cas de problème, on arrête l'I2C.
                etat_i2c = I2C_STOP;
            }
        }
        
        switch(etat_i2c){
          case I2C_START:     
          case I2C_ENV_ADRESSE:
            SSPBUF = (i2c_slave_adresse << 1 | 0);
            data_index = 0;
            etat_i2c = I2C_ENV_DONNEE;
            nacq = 1;
            break;
            
          case I2C_ENV_DONNEE:
            SSPBUF = data_out[data_index];
            data_index++;
            nacq = 1;
            // Si on a tout envoyé
            if(data_index == nb_env){
                etat_i2c = I2C_ENV_DONNEE;                
            }
            break;
                        
          case I2C_RESTART:
            SSPCON2bits.RSEN=1;
            break;
                        
          case I2C_REC_ADRESSE:
            SSPBUF = (i2c_slave_adresse << 1 | 1);
            data_index++;
            nacq = 1;
            break;
                        
          case I2C_REC_DONNEE:
            if(data_index == nb_rec){
                etat_i2c = I2C_STOP;
                break;
            }
            SSPCON2bits.RCEN=1;
            etat_i2c = I2C_REC_ACK;
            break;            
            
          case I2C_REC_ACK:
            // Envoi d'un accusé réception
            SSPCON2bits.ACKDT=0;
            data_in[data_index]=SSPBUF;
            data_index++;
            etat_i2c = I2C_REC_ADRESSE;
          
          case I2C_STOP:
            //Arrêt de l'I2C
            SSPCON2bits.PEN = 1;
        }
    }
}










