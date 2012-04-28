#define HAUT 1
#define BAS  0

#define ADRESSE_SERVO 0x33

char transmission_servo(void);

void SetServoPArG(char haut);
void SetServoPArD(char haut);
void SetServoPAv(char haut);
void SetServoCMUcam(char haut);
void SetCremaillere(char haut);
void GetDonneesServo(void);
char GetEnvoiServo(void);

void CDBrasDroit(void);
void CDBrasGauche(void);

char get_IS_Droit(void);
char get_IS_Gauche(void);
char get_Etat_Droit(void);
char get_Etat_Gauche(void);
char get_Presence_Pion(void);


