#define NB_BRAS_2_STRATEGIE 1
#define NB_STRATEGIE_2_BRAS 1


#define CDE_BRAS_OUVERT 1
#define CDE_BRAS_ATTRAPE 2
#define CDE_BRAS_DEPOSE 3


union message_bras_t {
	unsigned char MESSAGE;
	
	struct {
	  unsigned COMMANDE_BRAS:2;
	  unsigned :6;
	};
};

enum etat_bras_t {
/* Attraper le lingo */
    REPLIE=0,
    OUVRE_DOIGT,
    AVANCE_BRAS,
    FERME_DOIGT,
    RECULE_BRAS,
    RENTRE_LINGOT,
/* Deposer le lingo */
	ROUVRE_DOIGT,
	POUSSE_LINGOT,
	RENTRE_BRAS
};
