extern volatile unsigned int centisecondes;

void Temps_Init(void);
unsigned int getTemps(void);
unsigned  int getTemps_s(void);
unsigned int getTemps_cs(void);
unsigned int getTemps_micro_s(void);
void Temps_Int(void);
