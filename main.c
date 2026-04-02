#include <msp430.h>

static unsigned int t_led; 
static unsigned int t_react; 
static unsigned int state = 0; 
static unsigned int delta;

void main()
{

    //Unified Clock Setup
    UCSCTL2 = 1;
    UCSCTL3 = SELREF_0;
    UCSCTL4 = SELS_0;

    // Button
    P2DIR &= ~BIT6; //selects s1 as a button
    P2IES |= BIT6; //selects interrupt edge 
    P2IE |= BIT6; //enable pin interrupt
    P2REN |= BIT6; // pullup 
    P2OUT |= BIT6; // pullup
    P2IFG &= ~BIT6; // enable flag

    P10DIR |= BIT0; // Some pin for measureing the clock

    P1DIR |= BIT0; // LED
    P1OUT $= ~BIT0

    //Timer A
    TA0CTL = TASSEL_2 + MC__CONTINOUS + TACLR;
    TA0CCTL1 = CCIE // Enable CCR1 interrupt

    EINT();
    LPM0;
}
