#include <msp430.h>
#include "HAL_MSP-EXP430F5438.h"
#include "hal_lcd.h"

static unsigned int t_led; 
static unsigned int t_react; 
static unsigned int state = 0; 
static unsigned int delta;
unsigned int r;
unsigned int delay_counts;
#define ONE_SEC 4096
volatile unsigned long g_ms = 0;
volatile unsigned char sound_detected = 0;
volatile unsigned int  sound_time = 0;
volatile unsigned char playing   = 0;
#define SOUND_THRESHOLD 600 //claping is around 80-90 dB; 150-300 is load clap;300-600 is very load clap, etc,
void buzzer_init(void);
void play_tone(unsigned int freq);
void stop_tone(void);
#define SAMPLE_RATE_HZ    8000          // Approx. 8 kHz
#define BUF_SIZE 200
volatile unsigned int audio_buffer[BUF_SIZE]; // 4050 is clapping range and 4000 is noise

volatile unsigned int write_index = 0;
volatile unsigned int play_index  = 0;

volatile unsigned char recording      = 0;


/* Convert milliseconds to string "X.XXX" */
void formatTime(unsigned long ms, char *buf)
{
    unsigned int whole;
    unsigned int frac;

    if (ms > 99999UL)
    {
        ms = 99999UL;
    }

    whole = (unsigned int)(ms / 1000UL);   // 0 to 9
    frac  = (unsigned int)(ms % 1000UL);   // 000 to 999
    
    buf[0] = '0' + whole / 10;
    buf[1] = '0' + whole % 10;
    buf[2] = '.';
    buf[3] = '0' + (frac / 100);
    buf[4] = '0' + ((frac / 10) % 10);
    buf[5] = '0' + (frac % 10);
    buf[6] = '\0';
}

/* Show the time string on line 0 */
void showTime(unsigned long ms)
{
    char buf[7];

    formatTime(ms, buf);

//    halLcdClearScreen();
 //   halLcdPrintLine(buf, 0, OVERWRITE_TEXT);
}

//NOTE 
void main(void)
{

    //Unified Clock Setup
    UCSCTL2 = 1;
    UCSCTL3 = SELREF_0;
    UCSCTL4 = SELA__XT1CLK;

    // Reaction Button
    P2DIR &= ~BIT6; //selects s1 as a button
    P2IES |= BIT6; //selects interrupt edge 
    P2IE |= BIT6; //enable pin interrupt
    P2REN |= BIT6; // pullup 
    P2OUT |= BIT6; // pullup
    P2IFG &= ~BIT6; // enable flag

    // Start/Reset Button
    P2DIR &= ~BIT7; //selects s2 as a button
    P2IES |= BIT7; //selects interrupt edge 
    P2IE |= BIT7; //enable pin interrupt
    P2REN |= BIT7; // pullup 
    P2OUT |= BIT7; // pullup
    P2IFG &= ~BIT7; // enable flag

    P1DIR |= BIT4; // Some pin for measureing the clock
    P1OUT &= ~BIT4;

    P10DIR |= BIT0;

    P1DIR |= BIT0; // LED
    P1OUT &= ~BIT0;

    // Power microphone + op-amp
    P6DIR |= BIT4;      // P6.4 output
    P6OUT |= BIT4;      // turn ON mic + op-amp

    //Speaker 
    P4DIR |= BIT4;
    P4SEL |= BIT4;

    P6SEL |= BIT5;

    //Configure ADC12 for A5 (audio input)
    ADC12CTL0 = ADC12SHT0_8 | ADC12ON; //128-cycle sample time
    ADC12CTL1 = ADC12SHP | ADC12SSEL_3 |ADC12CONSEQ_2; // SMCLK, repeat single channel
    ADC12MCTL0 = ADC12INCH_5; // Selects Channel A5
    ADC12IE = ADC12IE0; // this bit enables the interrupt rquest for ADC12IFG0
    ADC12CTL0 |= ADC12ENC | ADC12SC;              // enable ADC and start conversions
   
    //Timer A
    //owen or amelia timer a channel 1
    TA0CTL = TASSEL__ACLK + MC__CONTINUOUS + TACLR + ID_3;  //32 kHz for timer
    TA0CCTL1 = CCIE; // Enable CCR1 interrupt

    //sam timera channel 0
        TA0CCR0  = 33;                      // about 1 ms 32768cylespersecond/1000milliseconds = 33cyclespermillisecond
   TA0CCTL0 = CCIE;                    // enable CCR0 interrupt

    //NOTE LCD Board Initialization
  //  halBoardInit();         // board init
    //halLcdInit();           // LCD init
    //halLcdClearScreen();    // clear LCD
    //halLcdBackLightInit();
    //halLcdSetBackLight(10);
    //halLcdSetContrast(100);

    //NOTE this is the timer setup using a 32kHz crystal for ACLK as the source

    //TA0CTL   = TASSEL__ACLK | MC__UP | TACLR;

    _EINT();
    /*while (1)
    {
        showTime(g_ms);
       // __delay_cycles(500);   // slows screen refresh a little but find what works for you stukk
    }*/

    LPM0;
}

/* Timer interrupt */
#pragma vector = TIMER0_A0_VECTOR
__interrupt void timer0A0ISR(void)
{
    g_ms++;

    if (state == 2 && sound_detected)
    {
        t_react = sound_time;
        delta = t_react - t_led;

        P1OUT &= ~BIT0;
        stop_tone();
        sound_detected = 0;
        state = 0;
    }
}

//button interrupt
void button_interrupt(void)__interrupt[PORT2_VECTOR]
{ 
    switch (P2IV) 
    {
        case 0x10: //Start/reset button
            if (state == 0)
            {
                TA0CTL |= TACLR;
                sound_detected = 0;
                P1OUT &= ~BIT0;
                stop_tone();

                r = 10;
                delay_counts = 2 * ONE_SEC + (r % (4 * ONE_SEC));

                TA0CCR1 = TA0R + delay_counts;
                TA0CCTL1 |= CCIE;

                state = 1;
            }
            else
            {
                state = 0;
                sound_detected = 0;

                P1OUT &= ~BIT0;
                stop_tone();

                TA0CCTL1 &= ~CCIE;
                TA0CCR0 = 0;
            }
            break;

        case 0x0e: //reaction
            if (state == 2)
            {
                t_react = TA0R;
                delta = t_react - t_led;

                P1OUT &= ~BIT0; //LED off
                stop_tone(); // Buzzer off

                state = 0; // idle
            }
            break;
    }
}


void ADC_Interrupt(void)__interrupt[ADC12_VECTOR]
{
    unsigned int sample = ADC12MEM0;

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

}

void timerA_ISR(void)__interrupt[TIMER0_A1_VECTOR]
{
    if (TA0IV == TA0IV_TACCR1)
    {
        TA0CCTL1 &= ~CCIE;

        P1OUT |= BIT0;       // LED ON
        t_led = TA0R;        // Start time
        state = 2;

        sound_detected = 0;  // <-- IMPORTANT RESET
        play_tone(100);
 
    }
}
