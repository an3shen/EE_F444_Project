//#include <msp430.h>

void main()
{
    // Button
    P2DIR &= ~BIT6; //selects s1 as a button
    P2IES |= BIT6; //selects interrupt edge 
    P2IE |= BIT6; //enable pin interrupt
    P2REN |= BIT6; // pullup 
    P2OUT |= BIT6; // pullup
    P2IFG &= ~BIT6; // enable flag

    eint();
    lpm0;
}

    static unsigned int t1; 
    static unsigned int t2; 
    static unsigned int state = 0; 
    static unsigned int delta;
void button_interrupt(void)__interrupt[PORT2_VECTOR]
{ 
    if (P2IV == 0x0e)
    {
        if (state == 0)// this flips to go back and forth to measure
        {
            P10OUT ^= BIT0;
            t1 = TA0R;
            state = 1;
            P8OUT |= BIT6;
        }
        else 
        {
            P10OUT &=~ BIT0;
            t2 = TA0R;
            delta = t2 - t1;
            state = 0; 
        }
    }
}
