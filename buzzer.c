#include <msp430.h>

// Initialize the buzzer pin and timer
void buzzer_init(void) {
    P4DIR |= BIT4;             // Set P4.4 to output direction
    P4SEL |= BIT4;             // Select Timer B0.4 output function
}

// Start playing a tone at a specific frequency
void play_tone(unsigned int clockFreq, unsigned int freq) {
    // Frequency calculation: 32768Hz / freq
    // For a 1kHz tone, CCR0 would be ~33
    TB0CCR0 = clockFreq / freq;    
    TB0CCTL4 = OUTMOD_7;       // Reset/Set mode for PWM
    TB0CCR4 = TB0CCR0 >> 1;    // 50% duty cycle
    TB0CTL = TBSSEL_2 + MC_1;  // ACLK, up mode
}

// Stop the tone immediately
void stop_tone(void) {
    TB0CTL = MC_0;             // Stop the timer
    P4OUT &= ~BIT4;            // Ensure pin is low
}

