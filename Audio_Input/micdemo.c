#include <msp430.h>
#include "audio.h"

extern int IncrementVcore(void);

#define SOUND_THRESHOLD   1000         // Clap threshold (tune experimentally)
#define SAMPLE_RATE_HZ    8000          // Approx. 8 kHz
#define BUF_SIZE 2000
volatile unsigned int audio_buffer[BUF_SIZE]; // 4050 is clapping range and 4000 is noise

volatile unsigned int write_index = 0;
volatile unsigned int play_index  = 0;

volatile unsigned char sound_detected = 0;
volatile unsigned char recording      = 0;
volatile unsigned char playing        = 0;

void init_clock(void)
{
  IncrementVcore();
  IncrementVcore();
  IncrementVcore();
    UCSCTL2 = 1;
    UCSCTL3 = SELREF_0;
    UCSCTL4 = SELS_0;// Assume default DCO ~1 MHz (good enough for demo).
    // If you have a different clock setup, adjust SAMPLE_RATE_HZ math below.
}

void init_led(void)
{
    P1DIR |= BIT0;   // P1.0 as output (LED)
}


void init_mic_and_adc(void)
{
    // Power microphone + op-amp on P6.4
    P6DIR |= BIT4;
    P6OUT |= BIT4;
    P5SEL |= BIT5;

    // Mic input on A5 (P6.5)
    P6SEL |= BIT5;

    // ADC12 configuration
    ADC12CTL0 = ADC12SHT0_8 | ADC12ON;   // 128-cycle sample time, ADC on
    ADC12CTL1 = ADC12SHP;                // Use sampling timer, single-channel, single-conversion
    ADC12MCTL0 = ADC12INCH_5;            // Channel A5
    ADC12IE   = ADC12IE0;                // Enable interrupt for MEM0
    ADC12CTL0 |= ADC12ENC; 
  
}

void init_speaker_pwm(void)
{
    // P4.1 as TA0.1 output (speaker)
    P4DIR |= BIT1;
    P4SEL |= BIT1;

    // Timer0_A for PWM (8-bit-ish range)
    TA0CCR0  = 255;                      // PWM period
    TA0CCTL1 = OUTMOD_7;                 // Reset/Set mode
    TA0CCR1  = 0;                        // Start with 0 duty
    TA0CTL   = TASSEL_2 | MC_1 | TACLR;  // SMCLK, up mode
}

void init_sample_timer(void)
{
    // Timer1_A0: generates interrupts at SAMPLE_RATE_HZ
    // Assuming SMCLK ~1 MHz
    // CCR0 = 1 MHz / 8000 = 125
    TA1CCR0  = (1000000 / SAMPLE_RATE_HZ) - 1;
    TA1CCTL0 = CCIE;                     // Enable CCR0 interrupt
    TA1CTL   = TASSEL_2 | MC_1 | TACLR;  // SMCLK, up mode
    TA0CCR0 = 4095;

}

int main(void)
{
    init_clock();
    init_mic_and_adc();
    init_led();
    init_speaker_pwm();
    init_sample_timer();

    _EINT();

        // Simple state machine in main (optional)
        // Everything important happens in ISRs.
    LPM0;
}

// Timer1_A0 ISR: triggers ADC sampling at fixed rate

void timerA_ISR(void)__interrupt[TIMER1_A0_VECTOR]
{
    // Only sample when recording or waiting for clap
    // (You can always sample if you prefer.)
    ADC12CTL0 |= ADC12SC;    // Start conversion


}

// ADC12 ISR: clap detection, recording, and playback

void ADC_Interrupt(void)__interrupt[ADC12_VECTOR]
{
    unsigned int sample = ADC12MEM0;

    P1OUT ^= BIT0;
    // --- Clap detection when idle ---
    if (!recording && !playing)
    {

        if (!sound_detected && sample > SOUND_THRESHOLD)
        {
            sound_detected = 1;
            recording      = 1;
            write_index    = 0;

        }
    }

    // --- Recording mode ---
    if (recording)
    {
        audio_buffer[write_index++] = sample;

        if (write_index >= BUF_SIZE)
        {
            recording   = 0;
            playing     = 1;
            play_index  = 0;
            sound_detected = 0;
        }
    }
    // --- Playback mode ---
    else if (playing)
    {
        // Scale 12-bit ADC sample (0–4095) to 8-bit PWM (0–255)
        unsigned int out = audio_buffer[play_index++] >> 4;
        if (out > 255) out = 255;

        TA0CCR1 = out;   // Set PWM duty for speaker

        if (play_index >= BUF_SIZE)
        {
            playing = 0;
            TA0CCR1 = 0; // Silence
        }
    }
}
