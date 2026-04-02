void timerA_ISR(void)__interrupt[TIMER0_A1_VECTOR]
{
    if (TA0IV == TA0IV_TACCR1)
    {
        P10OUT ^= BIT0; // toggles waveforms
        P1OUT |= BIT0;
        t_led = TA0R;
        state = 2;
    }
}