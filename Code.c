#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator: High-speed crystal/resonator on RA4/OSC2/CLKOUT and RA5/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select bit (MCLR pin function is MCLR)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = ON       // Brown-out Reset Selection bits (BOR enabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal External Switchover mode is disabled)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is enabled)

#include <xc.h>
#include "configurations bits.h"
#define _XTAL_FREQ 8 000 000
#define Baud_rate 9600
#define di_debit 0.095
// prototypes
void config_blue();
void load_char(char val);
void load_string(char* string);
void load_float();
char get_char(char character);
void interruption();
void time();
void config_timer2();
void config_timer0();
//void aff_volume(); 

//variaables
float debit = 0;
unsigned long pre_volume = 0;
unsigned long vol = 0;
char cnt;

void __interrupt() traitement() {

    if (T0IF) {
        T0IF = 0;
    }
    if (TMR2IF) // Timer flag has been triggered due to timer overflow
    {
        cnt = TMR0;
        TMR0 = 0;
        TMR2IF = 0; // Clear timer interrupt flag
        pre_volume += cnt;
        debit = cnt * di_debit;
        vol = (pre_volume / 330);
        eeprom_write(0, pre_volume);
        eeprom_write(0x55, vol);
    }
    if (RCIF) {
        RCIF = 0;
        if (RCREG == 'A') {
            load_char(13);
            load_string("chambre 1");
            load_char(13);
            load_string("volume = ");
            load_float();
            load_char('L');
            load_char(13);
        }
    }
}

void main(void) {
    ANSEL = 0; //Configurer tout les ports en digital
    ANSELH = 0;
    interruption();
    config_timer0();
    config_timer2();
    config_blue();
    load_string("Bienvenue");
    load_char(13);
    while (1);
}

// configurations to enable all required interruptions

void interruption() {
    GIE = 1; //active l'interruption globale
    PEIE = 1; //active l'interruption sur les périphériques
    RCIE = 1; //active l'interruptuion sur la réception
    T0IE = 1;
}


//timer2

void config_timer2() {
    TMR2IE = 1; //active l'interruption sur le timer2
    TMR2ON = 1; //le TIMER2 est activé
    T2CKPS0 = 1; // le prescaler prend la valeur 16
    T2CKPS1 = 1;
    TOUTPS0 = 1; //le postscaler prend la valeur 16
    TOUTPS1 = 1; //le postscaler prend la valeur 16
    TOUTPS2 = 1; //le postscaler prend la valeur 16
    TOUTPS3 = 1; //le postscaler prend la valeur 16
    PR2 = 249;
}

//timer0

void config_timer0() {
    T0CS = 1; //sélectionne le pin T0CKI pour le comptage des impulsionbs
    PSA = 1; //le prescaler est sur le timer2
    T0SE = 0; //l'incrémentation du timer se fait sur chaque front montant

}
//uart configurations for bluetooth

void config_blue() {
    //Set the baud rate using the look up table in datasheet(pg114)//
    BRGH = 0; //Always use high speed baud rate with  else it wont work
    BRG16 = 0;
    //Turn on*. Serial Port//
    SYNC = 0;
    SPEN = 1;
    //Enable transmission and reception//
    TXEN = 1;
    CREN = 1;
    SPBRG = 12;
    //enable 8bits transmission and receive 9 nits
    TX9 = 0;
    RX9 = 0;
}

//char sending

void load_char(char val) {
    while (!TXIF);
    TXREG = val;
}
//string sending

void load_string(char* string) {
    while (*string)
        load_char(*string++);
}

//load float 

void load_float() {
    int nul = 10, c = 0, c1 = 0, q = 0, q1 = 0;
    if (vol > 0) {
        if (vol < 10) {
            load_char(0 + '0');
        } else if (vol >= 10 && vol < 100) {
            q = vol / nul;
            c = vol % nul;
            load_char(q + '0');
            load_char(c + '0');
        } else if (vol >= 100 && vol < 1000) {
            q = vol / nul;
            q1 = q / nul;
            c = vol % nul;
            c1 = q % nul;
            load_char(q1 + '0');
            load_char(c1 + '0');
            load_char(c + '0');
        }
    }
}

// char getting

char get_char(char character) {
    if (OERR) // check for over run error
    {
        CREN = 0;
        CREN = 1; //Reset CREN
    }
    if (RCIF == 1) //if the user has sent a char return the char (ASCII value)
    {
        while (!RCIF);
        return RCREG;
    } else //if user has sent no message return 0
        return 0;
}
