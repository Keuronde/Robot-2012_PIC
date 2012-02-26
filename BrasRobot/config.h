// Contacteur
// TRIS
#define TRIS_CT1 TRISAbits.TRISA0
#define TRIS_CT2 TRISAbits.TRISA1
#define TRIS_CT3 TRISAbits.TRISA2
#define TRIS_CT4 TRISAbits.TRISA3
#define TRIS_CT7 TRISCbits.TRISC0
#define TRIS_CT8 TRISBbits.TRISB7
#define TRIS_CT9 TRISBbits.TRISB6
#define TRIS_CT10 TRISBbits.TRISB5

#define TRIS_CT_M1_AV TRISAbits.TRISA4
#define TRIS_CT_M1_AR TRISAbits.TRISA5

// PORT
#define CT1 PORTAbits.RA0
#define CT2 PORTAbits.RA1
#define CT3 PORTAbits.RA2
#define CT4 PORTAbits.RA3
#define CT7 PORTCbits.RC0
#define CT8 PORTBbits.RB7
#define CT9 PORTBbits.RB6
#define CT10 PORTBbits.RB5

#define CT_M1_AV PORTAbits.RA4
#define CT_M1_AR PORTAbits.RA5


// Moteur
// TRIS
#define TRIS_M1_ENABLE TRISCbits.TRISC2
#define TRIS_M2_ENABLE TRISCbits.TRISC1
#define TRIS_M1_SENS TRISBbits.TRISB3
#define TRIS_M2_SENS TRISBbits.TRISB2

//LAT
#define M1_ENABLE LATCbits.LATC2
#define M2_ENABLE LATCbits.LATC1
#define M1_SENS LATBbits.LATB2
#define M2_SENS LATBbits.LATB3

// Bouton RB4
#define TRIB_BOUTON TRISBbits.TRISB4
#define BOUTON PORTBbits.RB4

// Servomoteurs
#define TRIS_SERVO1 TRISCbits.TRISC7
#define TRIS_SERVO2 TRISCbits.TRISC6

#define SERVO1 LATCbits.LATC7
#define SERVO2 LATCbits.LATC6
