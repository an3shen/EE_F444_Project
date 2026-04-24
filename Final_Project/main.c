#include <msp430.h>
#include "HAL_MSP-EXP430F5438.h"
#include "hal_lcd.h"

static unsigned int t_led; 
static unsigned int t_react; 
static unsigned int state = 0; 
static unsigned int delta;
unsigned int r;
unsigned int delay_counts;
#define ONE_SEC 50000 //986000 //for 1 mHz it's 1000000 nut really 986000 for our thing
volatile unsigned char sound_detected = 0;
volatile unsigned int  sound_time = 0;
volatile unsigned char playing   = 0;
unsigned int sample = 0;


unsigned int g_ms = 0;
unsigned int tref  = 0;
unsigned int soundMagnitude = 0;


#define SOUND_THRESHOLD 4000 //claping is around 80-90 dB; 150-300 is load clap;300-600 is very load clap, etc,
void buzzer_init(void);
void play_tone(unsigned int freq);
void stop_tone(void);

 // stored value 4050 is clapping range and 4000 is noise

volatile unsigned int write_index = 0;
volatile unsigned int play_index  = 0; 
volatile unsigned char recording = 0;

/* Convert milliseconds to string "XX.XXX" */
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

  //  halLcdClearScreen();
   halLcdPrintLine(buf, 0, OVERWRITE_TEXT);
}

//NOTE 
void main(void)
{

    //Unified Clock System Setup
    UCSCTL1 = DCORSEL_1; 
    UCSCTL2 = 30;//  1Mhz/32768 = 30.5 -->31 - 1 = 30 is the frequency multiplier for DCO 
    UCSCTL3 = SELREF_0;
    UCSCTL4 = SELS__DCOCLK + SELA__XT1CLK;  //NOTE is XT1 working?



    // Reaction Button
    P2DIR &= ~BIT6; //selects s1 as a button
    P2IES |= BIT6; //selects interrupt edge 
    P2IE |= BIT6; //enable pin interrupt
    P2REN |= BIT6; // pullup  
    P2IFG &= ~BIT6; // enable flag

    // Start/Reset Button
    P2DIR &= ~BIT7; //selects s2 as a button redundantly
    P2IES |= BIT7; //selects interrupt edge 
    P2IE |= BIT7; //enable pin interrupt
    P2REN |= BIT7; // pullup 
    P2OUT |= BIT7; // pullup
    P2IFG &= ~BIT7; // enable flag

    P1DIR |= BIT4; // Some pin for measuring the clock
    P1OUT &= ~BIT4;

    P10DIR |= BIT0;

    P1DIR |= BIT0; // LED
    P1OUT &= ~BIT0;

    // Power microphone + op-amp
    P6DIR |= BIT4;      // P6.4 output
    P6OUT |= BIT4;      // turn ON mic + op-amp
    P6SEL |= BIT5;


    //Speaker 
    P4DIR |= BIT4;
    P4SEL |= BIT4;

    
   
    //Timer A
    //owen or amelia timer a channel 1
    TA1CTL = TASSEL__SMCLK + MC__CONTINUOUS + TACLR;  // 1 mHz for timer
    TA0CTL = TASSEL__ACLK + MC__CONTINUOUS + TACLR; //
         

   

       //Configure ADC12 for A5 (audio input)
    ADC12CTL0 |= ADC12SHT0_5 + ADC12ON ; //256-cycle sample time?ADC12MSC is for continuous sampling and not needed
    ADC12CTL1 |= ADC12SHP + ADC12SSEL_3 ; // SMCLK, repeat single channel
   
   ADC12MCTL0 |= ADC12INCH_5; // Selects Channel A5
    ADC12IE |= ADC12IE0; // this bit enables the interrupt rquest for ADC12IFG0
    ADC12CTL0 |= ADC12ENC;              // enable ADC and start conversions

    //NOTE LCD Board Initialization
  // halBoardInit();         // board init
    halLcdInit();           // LCD init
    halLcdClearScreen();    // clear LCD
    halLcdBackLightInit();
    halLcdSetBackLight(5);
    halLcdSetContrast(90);


    _EINT();
    /*while (1)
    {
        showTime(TA1R);
       // __delay_cycles(500);   // slows screen refresh a little but find what works for you stukk
    }
*/
    LPM0;
}


//button interrupt
void button_interrupt(void)__interrupt[PORT2_VECTOR]
{ 
    switch (P2IV) 
    {
        case 0x10: //Start/reset button
            if (state == 0)
            {
                TA1CTL |= TACLR;     //clear timers from previous time
                sound_detected = 0;  //clear from last time
                P1OUT &= ~BIT0;         //turn LED off
                stop_tone();            //redundancy
  g_ms = 0;
               delay_counts = 30000;   // test value first
TA1CCR1 = TA1R + delay_counts;
                
             
TA1CCTL1 |= CCIE;
              
                state = 1;                  
            }
            else
            {
                state = 0;
                sound_detected = 0;
              

                P1OUT &= ~BIT0;
                stop_tone();

                TA1CCTL1 &= ~CCIE;
                TA1CCR1 = 0;
                //TA1CCR2 = TA1R+1000;
            }
            break;

        case 0x0e: //reaction
            /*if (state == 2)
            {
                t_react = g_ms;
                delta = t_react - t_led;

                P1OUT &= ~BIT0; //LED off
                stop_tone(); // Buzzer off
                sound_detected =1;
                state = 0; // idle
            }*/
            break;
    }
}

void ADC_Interrupt(void)__interrupt[ADC12_VECTOR]
{
      sample = ADC12MEM0;
    if( sample >= SOUND_THRESHOLD){
        t_react = g_ms;
               
                stop_tone();
        sound_detected = 1;  
        ADC12CTL0 &= ~ADC12ENC; 
  delta = t_react - t_led;
  soundMagnitude = sample;
        P1OUT &= ~BIT0;
        state = 0;

}
}


void timerA1_ISR(void)__interrupt[TIMER1_A1_VECTOR]
{
    switch (TA1IV)
    {
        case TA1IV_TA1CCR1:
        
            TA1CCTL1 &= ~CCIE;
            P1OUT |= BIT0;
            t_led = g_ms;
            
            state = 2;
            sound_detected = 0;
tref = TA1R;

while ((TA1R - tref) < 50000){ play_tone(600);};   // 50 ms
tref = TA1R;
while ((TA1R - tref) < 50000){ play_tone(600);};   // 50 ms
tref = TA1R;
while ((TA1R - tref) < 50000){ play_tone(600);};   // 50 ms
tref = TA1R;
while ((TA1R - tref) < 50000){ play_tone(600);};   // 50 ms
g_ms += 200;
            stop_tone();
             TA0CCTL1 |= CCIE;  //time incrementation
                    TA0CCR1 = TA0R + 33;            //enable  capture compare interrupt
         

      
        //showTime(delta)

            break;
      case TA1IV_TA1CCR2:
     // TA1CCR2+=1000;
    //  if(TA1R%10 == 9){
//g_ms++;
//   if(!sound_detected){
//            ADC12CTL0 |= ADC12ENC + ADC12SC;   // start continuous conversion
//                        }
//                       else if (sound_detected){ showTime(delta);
//                       TA1CCTL2 &= ~CCIE;
//                       }
//}
break;

    }
    }



void timerA0_ISR(void)__interrupt[TIMER0_A1_VECTOR]
{
 switch (TA0IV)
    {

case TA0IV_TACCR1:
TA0CCR1 += 33;
g_ms++;
   if(!sound_detected){
           // ADC12CTL0 |= ADC12ENC + ADC12SC;   // start continuous conversion
                        }
                       else if (sound_detected){ showTime(delta);
                       TA0CCTL1 &= ~CCIE;
                       }
break;

    }
}

