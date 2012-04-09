
#define ASSER_TOURNE 0
#define ASSER_AVANCE 1
#define ASSER_RECULE 2


enum etat_asser_t {
	AVANCE_DROIT_INIT=0,
	DROIT_TEMPO,
	AVANCE_DROIT=2,
	AVANCE_DROIT_TEMPO,
	TOURNE_INIT,
	TOURNE_TEMPO,
	TOURNE,
	FIN_TOURNE,
	TOURNE_VERS_AVANCE,
	TOURNE_VERS_AVANCE_2,
	AVANCE_VERS_TOURNE,
	FIN_ASSER
};

extern char asser_actif;

void Asser_gestion(long * consigne_angle,long * angle);
void active_asser_lent(char avance_droit, long _angle,long * consigne_angle);
void active_asser(char avance_droit, long _angle, long * consigne_angle);
void desactive_asser(void);
char fin_asser(void);
char get_etat_asser(void);

