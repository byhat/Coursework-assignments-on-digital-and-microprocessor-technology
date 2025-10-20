/*
 * File:   main.c
 * 
 */


#include <xc.h>

#define _XTAL_FREQ 4000000UL  // microcontroller freq 4MHz

void setupUsart(void) {
    // set RC6 as output (TX)
    TRISCbits.TRISC6 = 0;

    TXSTAbits.BRGH = 1;   // high speed (16-bit baud rate generator)
    TXSTAbits.TXEN = 1;   // enable transmit pin
    RCSTAbits.SPEN = 1;   // enable serial port pins
    TXSTAbits.SYNC = 0;   // async mode

    // Serial Port Baud Rate Generator (set usart freq)
    // SPBRG = (Fosc / (16 * Baud)) - 1 = (4000000 / (16 * 9600)) - 1 ? 25.04 ? 25
    SPBRG = 25;
}

void UsartWriteChar(char c) {
    while (!TXSTAbits.TRMT);  // Transmit Shift Register Status
    TXREG = c;
}

void UsartWriteString(const char *str) {
    while (*str) {
        UsartWriteChar(*str);
        str++;
    }
}

volatile unsigned char led_state = 0; // led state (0 = off, 1 = on)

void setupTimer0(void) {
    T0CONbits.TMR0ON = 0;   // turn off timer
    T0CONbits.T08BIT = 0;   // set to 16-bit mode
    T0CONbits.T0CS = 0;     // use internal clock
    T0CONbits.PSA = 0;      // dont touch because we use internal clock
    T0CONbits.T0PS2 = 1;    // set prescaler to 1:256 (all bits to 1)
    T0CONbits.T0PS1 = 1;
    T0CONbits.T0PS0 = 1;
    
    INTCONbits.TMR0IE = 1;  // enable overflow interrupt for TMR0
    INTCONbits.TMR0IF = 0;  // turn off initial overflow interrupt
    INTCONbits.GIE = 1;     // global allow interrupts
    
    // set first timer value (for 500 ms)
    TMR0H = 0xF8;
    TMR0L = 0x5F;
}
// interrupt handler
void __interrupt() ISR(void) {
    if (INTCONbits.TMR0IF) {
        
        // set first timer value (for 500 ms)
        TMR0H = 0xF8;
        TMR0L = 0x5F;
        
        INTCONbits.TMR0IF = 0; // turn off overflow interrupt

        led_state = !led_state;     // switch led state
        LATBbits.LATB0 = led_state; // write led state to pin

        // send message to USART
        if (led_state) {
            UsartWriteString("Led on\r\n");
        } else {
            UsartWriteString("Led off\r\n");
        }
    }
}

void main(void) {
    ADCON1 = 0x0F; // all pins to ADC mode

    TRISBbits.TRISB0 = 0; // set RB0 as output
    LATBbits.LATB0 = 0;   // set latch register to 0 voltage (0 output)

    setupUsart();
    setupTimer0();

    T0CONbits.TMR0ON = 1; // start timer

    UsartWriteString("Start blinking\r\n"); // send test message

    while (1) {
        // all logic in interrupt
    }
}
