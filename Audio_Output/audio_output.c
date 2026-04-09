void main(void) 
{
    //Speaker 
    P4DIR |= BIT1;
    P4SEL |= BIT1;
}
// In Timer_A CCR1 ISR
P1OUT |= BIT0;
t_led = TA0R;
state = 2;
play_tone(1000); 