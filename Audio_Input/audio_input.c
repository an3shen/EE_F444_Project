unsigned int sound_detected = 0;
unsigned int sound_time = 0;
#define SOUND_THRESHOLD 2800 //claping is around 80-90 dB

void main(void)
{
    // Power microphone + op-amp
    P6DIR |= BIT4;      // P6.4 output
    P6OUT |= BIT4;      // turn ON mic + op-amp

    //Configure ADC12 for A5 (audio input)
    ADC12CTL0 = ADC12SHT0_8 | ADC12ON; // 128-cycle sample time
    ADC12CTL1 = ADC12SHP | ADC12SSEL_3 |ADC12CONSEQ_2; //SAMPSON sourced from sample timer, sequence of reapeat signal channel, set to SMCLK
    ADC12MCTL0 = ADC12INCH_5; // Selects Channel A5
    ADC12IE = ADC12IE0; // this bit enables the interrupt rquest for ADC12IFG0
    ADC12CTL0 |= ADC12ENC | ADC12SC;              // enable ADC

}