void main(void)
{
    // 1. Power microphone + op-amp
    P6DIR |= BIT4;      // P6.4 output
    P6OUT |= BIT4;      // turn ON mic + op-amp

    // 2. Configure ADC12 for A5 (audio input)
    ADC12CTL0 = ADC12SHT0_2 | ADC12ON;   // sample time + ADC on
    ADC12CTL1 = ADC12SHP;                // use sampling timer
    ADC12MCTL0 = ADC12INCH_5;            // channel A5
    ADC12CTL0 |= ADC12ENC;               // enable ADC

    while (1)
    {
        ADC12CTL0 |= ADC12SC;            // start conversion
        while (ADC12CTL1 & ADC12BUSY);   // wait
        unsigned int sample = ADC12MEM0; // audio sample

        __no_operation();                // breakpoint here to inspect sample
    }
}