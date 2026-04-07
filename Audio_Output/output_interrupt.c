void button_interrupt(void)__interrupt[PORT2_VECTOR] //Replace button interrupt
{
    if (P2IV == 0x0e)
    {
        if (state == 0)
        {
            TA0CTL |= TACLR;

            r = 10;
            delay_counts = 2 * ONE_SEC + (r % (4 * ONE_SEC));

            TA0CCR1 = TA0R + delay_counts;
            state = 1;
        }

        else if (state == 2)
        {
            t_react = TA0R;
            delta = t_react - t_led;

            P1OUT &= ~BIT0;
            stop_tone();      // <‑‑ AUDIO OFF
            state = 0;
        }
    }
}