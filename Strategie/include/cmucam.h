
#define LIM_GAUCHE 88
#define LIM_DROITE 264
#define LIM_MARGE 10

#define CMUCAM_FIN 0x0a
#define NB_DATA_IN 25
#define NB_DATA_OUT 25

// *********

enum booleen {
    NON=0,
    OUI
};

enum etat_cmucam_t {
    INIT=0,
    RECEPTION_FORME,
    NOUVELLE_RECHERCHE,
    TEST_FORME,
    ENVOI_ID,
    TRACKING_ATTENTE,
    TRACKING,
    TRACKING_PROCHE,
    CMUCAM_RESET,
    CMUCAM_RESETING,
    CMUCAM_PRETE,
    PERDU
};

typedef struct  {
  unsigned int x0,x1,y0,y1;
  unsigned char id;
} figure_t;


/** P U B L I C  P R O T O T Y P E S ***************************************/
void CMUcam_Init(void);

void cherche_lingot(void);
void cherche_CD_ile(void);
void cherche_CD_droit(void);
void cherche_CD_gauche(void);
char cmucam_ile_proche();
char cherche_case_rouge(void);
char cherche_case_bleue(void);
char nouvelle_recherche(void);
void CMUcam_int(void);
void CMUcam_reset(void);
void setCouleur(char _c);
char cherche_couleur(void);

void CMUcam_active(void);
void CMUcam_desactive(void);
void CMUcam_desactive_asser(void);
enum etat_cmucam_t CMUcam_get_Etat(void);
void setCouleur(char _c);
void CMUcam_gestion(long * consigne_angle,long *angle);
char cmucam_ile_proche(void);

