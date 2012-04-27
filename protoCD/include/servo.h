


/** D E F I N E **************************************************************/
// Constantes
#define SERVO_MIN 0x220
#define SERVO_MAX 0x850
#define NB_SERVO 1

// Positions
// GAUCHE
// Pince
//#define PINCE_OUVERT (unsigned int)0x4EF,(char)3
#define PINCE_OUVERT (unsigned int)0x550,(char)3
#define PINCE_FERMEE (unsigned int)0x6FE,(char)3
#define PINCE_LACHE  (unsigned int)0x6A0,(char)3
// Bras
#define BRAS_HAUT    (unsigned int)0x6AE,(char)4
#define BRAS_BAS     (unsigned int)0x3B0,(char)4
#define BRAS_BASCULE (unsigned int)0x7FE,(char)4

// DROIT
// Pince
#define PINCE_OUVERT_D (unsigned int)0x550,(char)1
#define PINCE_FERMEE_D (unsigned int)0x3B0,(char)1
#define PINCE_LACHE_D  (unsigned int)0x400,(char)1
// Bras
#define BRAS_HAUT_D    (unsigned int)0x380,(char)2
#define BRAS_BAS_D     (unsigned int)0x680,(char)2
#define BRAS_BASCULE_D (unsigned int)0x280,(char)2


// Variables globales
extern volatile char timer_servo; // Compteur qui permet d'avoir une periode de 10 ms.
extern volatile char servo_courant; // Compteur qui sauvegarde le servo courant.
extern volatile int pos_servo[10]; 		// Tableau qui contient les valeurs 

/** P R I V A T E  P R O T O T Y P E S ***************************************/
void Servo_Set(unsigned int,char);
void Servo_Init(void);
void Servo_Int(void);
int Servo_Get(char);
