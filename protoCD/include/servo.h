


/** D E F I N E **************************************************************/
// Constantes
#define SERVO_MIN 0x220
#define SERVO_MAX 0x850
#define NB_SERVO 1

// Servo 1
#define TRIS_SERVO1 TRISAbits.TRISA0
#define SERVO1 LATAbits.LATA0
// Servo 2
#define TRIS_SERVO2 TRISAbits.TRISA1
#define SERVO2 LATAbits.LATA1

// Positions
// Pince
#define PINCE_OUVERT 0x6FE,0
#define PINCE_FERMEE 0x4EF,0
#define PINCE_LACHE  0x54F,0
// Bras
#define BRAS_HAUT    0x6AE,1
#define BRAS_BAS     0x393,1
#define BRAS_BASCULE 0x7FE,1


// Variables globales
extern volatile char timer_servo; // Compteur qui permet d'avoir une periode de 10 ms.
extern volatile char servo_courant; // Compteur qui sauvegarde le servo courant.
extern volatile int pos_servo[10]; 		// Tableau qui contient les valeurs 

/** P R I V A T E  P R O T O T Y P E S ***************************************/
void Servo_Set(unsigned int,char);
void Servo_Init(void);
void Servo_Int(void);
int Servo_Get(char);
