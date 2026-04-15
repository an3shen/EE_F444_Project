// RESET BUTTON
// in main
    P2DIR &= ~BIT7; //selects s2 as a button
    P2IES |= BIT7; //selects interrupt edge 
    P2IE |= BIT7; //enable pin interrupt
    P2REN |= BIT7; // pullup 
    P2OUT |= BIT7; // pullup
    P2IFG &= ~BIT7; // enable flag


//button interrupt
case 0x10: Start/reset button
    if (state == 0) //Starts the shot timer
    {
        TA0CTL |= TACLR; // clears timer
        sound_detected = 0;
        P1OUT &= ~BIT0; // LED off
        stop_tone(); //buzzer off

        delay_counts = 2 * ONE_SEC + (10 % (4 * ONE_SEC));

        TA0CCR1 =TA0R + delay_counts;
        TA0CCTL1 |= CCIE; // enables CCR1 interrupt

        state = 1; // waits for LED/buzzer
    }
    else
    { // Reset
        state = 0;
        sound_detected = 0;

        P1OUT &= ~BIT0;
        stop_tone();

        TA0CCTL1 |= CCIE; // disables CCR1 interrupt
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