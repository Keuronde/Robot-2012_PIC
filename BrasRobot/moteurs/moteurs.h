#include <p18f2550.h>
#include "config.h"


/** D E F I N E *******************************************************/
#define V_MOTEUR 0x035F

/** P R O T O T Y P E S   P U B L I C *********************************/
void Moteurs_Init(void); // A appeler après Servo_Init();
void M1_Avance(void);
void M1_Recule(void);
void M1_Stop(void);
void M2_Avance(void);
void M2_Recule(void);
void M2_Stop(void);
