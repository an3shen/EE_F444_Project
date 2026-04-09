#include <msp430.h>
#include "hal_board.h"
#include "hal_lcd.h"

static unsigned int t_led; 
static unsigned int t_react; 
static unsigned int state = 0; 
static unsigned int delta;
unsigned int r;
unsigned int delay_counts;
#define ONE_SEC 4096
volatile unsigned long g_ms = 0;
unsigned int sound_detected = 0;
unsigned int sound_time = 0;
#define SOUND_THRESHOLD 500 //claping is around 80-90 dB
void buzzer_init(void);
void play_tone(unsigned int freq);
void stop_tone(void);


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

    halLcdClearScreen();
    halLcdPrintLine(buf, 0, OVERWRITE_TEXT);
}


//NOTE 
void main(void)
{

    //Unified Clock Setup
    UCSCTL2 = 1;
    UCSCTL3 = SELREF_0;
    UCSCTL4 = SELA__XT1CLK;

    // Button
    P2DIR &= ~BIT6; //selects s1 as a button
    P2IES |= BIT6; //selects interrupt edge 
    P2IE |= BIT6; //enable pin interrupt
    P2REN |= BIT6; // pullup 
    P2OUT |= BIT6; // pullup
    P2IFG &= ~BIT6; // enable flag

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


    //Configure ADC12 for A5 (audio input)
    ADC12CTL0 = ADC12SHT0_2 | ADC12ON; // Need to change sample time: scurrently sampling 16 ADC12CLK cycles
    ADC12CTL1 = ADC12SHP | ADC12SSEL_1 |ADC12CONSEQ_2; //SAMPSON sourced from sample timer, sequence of reapeat signal channel, set to SMCLK
    ADC12MCTL0 = ADC12INCH_5; // Selects Channel A5
    ADC12IE = ADC12IE0; // this bit enables the interrupt rquest for ADC12IFG0
    ADC12CTL0 |= ADC12ENC;              // enable ADC
   

    //Timer A

    //owen or amelia timer a channel 1
    TA0CTL = TASSEL__ACLK + MC__CONTINUOUS + TACLR + ID_3;  //32 kHz for timer
    TA0CCTL1 = CCIE; // Enable CCR1 interrupt

    //sam timera channel 0
        TA0CCR0  = 33;                      // about 1 ms 32768cylespersecond/1000milliseconds = 33cyclespermillisecond
   TA0CCTL0 = CCIE;                    // enable CCR0 interrupt

    //LCD

    
   
    while (1)
    {
      showTime(g_ms);
      __delay_cycles(500);   // slows screen refresh a little
    }

    //NOTE LCD Board Initialization
    halBoardInit();         // board init
    halLcdInit();           // LCD init
    halLcdClearScreen();    // clear LCD
    halLcdBackLightInit();
    halLcdSetBackLight(10);
    halLcdSetContrast(100);

    //NOTE this is the timer setup using a 32kHz crystal for ACLK as the source

    //TA0CTL   = TASSEL__ACLK | MC__UP | TACLR;

  _EINT();

    while (1)
    {
        showTime(g_ms);
       // __delay_cycles(500);   // slows screen refresh a little but find what works for you stukk
    }


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


//NOTE butt

void button_interrupt(void)__interrupt[PORT2_VECTOR] //Replace button interrupt
{
    if (P2IV == 0x0e)
    {
        if (state == 0)
        {
            TA0CTL |= TACLR;

            delay_counts = 2 * ONE_SEC + (10 % (4 * ONE_SEC));

            TA0CCR1 = TA0R + delay_counts;
            state = 1;
        }

        else if (state == 2)
        {
            t_react = TA0R;
            delta = t_react - t_led;

            P1OUT &= ~BIT0; // remove led
            stop_tone();      // <-- AUDIO OFF
            state = 0;
        }
    }
}

void ADC_Interrupt(void)__interrupt[ADC12_VECTOR]
{
    unsigned int sample = ADC12MEM0;

    if (!sound_detected && sample > SOUND_THRESHOLD)
    {
        sound_time = TA0R;
        sound_detected = 1;
        ADC12CTL0 &= ~ADC12ENC;
    }
}

unsigned int startTime = 0;
void timerA_ISR(void)__interrupt[TIMER0_A1_VECTOR]
{
  P1OUT |= BIT0;  
  t_led = TA0R;    //THIS IS THE START OF REACTION TIME
  state = 2;
ADC12CTL0 |= ADC12SC;
  play_tone(100);   
 
    if (TA0IV == TA0IV_TACCR1)
    {
        TA0CCTL1 &= ~CCIE;  
        P10OUT ^= BIT0; // toggles waveforms
        P1OUT |= BIT0;
        t_led = TA0R;
        state = 2;
    }
}