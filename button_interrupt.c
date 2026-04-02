void button_interrupt(void)__interrupt[PORT2_VECTOR]
{ 
    if (P2IV == 0x0e)
    {
        if (state == 0)
        {
            TA0CTL |= TACLR;
            TA0CCR1 = x * 1000;
            state = 1;
        }
        else if (state == 2)
        {
            t_react = TA0R;
            delta = t_react - t_led;

            P1OUT &= ~BIT0;
            state = 0;
        }
    }
}

