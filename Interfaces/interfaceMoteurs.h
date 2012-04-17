#define NB_MOTEUR_2_STRATEGIE 1
#define NB_STRATEGIE_2_MOTEUR 3

struct c_Moteur_t {
  unsigned CT_AV_G:1;
  unsigned CT_AV_D:1;
  unsigned _CT3:1;
  unsigned _CT4:1;
  unsigned SONIC_PROCHE_G:1;
  unsigned SONIC_PROCHE_D:1;
  unsigned SONIC_LOIN_G:1;
  unsigned SONIC_LOIN_D:1;
};
