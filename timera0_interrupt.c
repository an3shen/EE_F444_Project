void timerA_ISR(void)__interrupt[TIMER0_A0_VECTOR]
{
    if (TA0IV == TA0IV_TACCR1)
    {
        P1OUT ^= BIT4;   // toggle P1.4 → square wave on scope
        P1OUT |= BIT0;
        t_led = TA0R;
        state = 2;
    }
}