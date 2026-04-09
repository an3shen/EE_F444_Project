// RESET BUTTON
// in main
    P2DIR &= ~BIT7; //selects s2 as a button
    P2IES |= BIT7; //selects interrupt edge 
    P2IE |= BIT7; //enable pin interrupt
    P2REN |= BIT7; // pullup 
    P2OUT |= BIT7; // pullup
    P2IFG &= ~BIT7; // enable flag


//button interrupt

switch (P2IV) 
{
    case 0x0e:
    if (state == 0)
        {
          TA0CTL |= TACLR;

          r = 10;   // whatever function the TI RNG file provides   //CHANGE THIS
          delay_counts = 2 * ONE_SEC + (r % (4 * ONE_SEC));

          TA0CCR1 = TA0R + delay_counts;
          state = 1;
        }

    else if (state == 2)
    {
        t_react = TA0R;
        delta = t_react - t_led;

        P1OUT &= ~BIT0;
        state = 0;
    }
    case 0x10:
        state = 0;
        sound_detected = 0;

        P1OUT &= ~BIT0;
        stop_tone();

        TA0CCTL1 &= ~CCIE;  // disable pending CCR1 event
        break;
}
