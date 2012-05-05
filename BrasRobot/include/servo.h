#include "../include/config.h"
/** D E F I N E **************************************************************/
// Constantes
#define SERVO_MIN 0x02D0 
#define SERVO_MAX 0x07C0
#define NB_SERVO 2


// SERVO 1 
#define DOIGT_G_OUVERT 0x2D0,0
//4D0
#define DOIGT_G_TIRE   0x460,0
#define DOIGT_G_RABAT  0x5D0,0
#define DOIGT_G_FERME  0x710,0
// SERVO 2 (marqué servo 1 sur la carte réelle)
#define DOIGT_D_OUVERT 0x710,1
//590
#define DOIGT_D_TIRE   0x5F0,1
#define DOIGT_D_RABAT  0x410,1
#define DOIGT_D_FERME  0x30F,1

/** P R I V A T E  P R O T O T Y P E S ***************************************/
void Servo_Set(unsigned int,char);
void Servo_Init(void);
void Servo_Int(void);
int Servo_Get(char);
