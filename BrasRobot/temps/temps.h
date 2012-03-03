extern volatile unsigned int centisecondes;

void Temps_Init(void);
unsigned int getTemps(void);
unsigned  int getTemps_s(void);
unsigned int getTemps_cs(void);
unsigned int getTemps_micro_s(void);
void Temps_Int(void);
#define TIMER_INIT 50536
#define TIMER_L    0x68
#define TIMER_H    0xC5
