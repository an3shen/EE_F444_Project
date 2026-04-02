
unsigned int sound_detected = 0;
unsigned int sound_time = 0;
#define SOUND_THRESHOLD 2800

void main(void)
{
    // 1. Power microphone + op-amp
    P6DIR |= BIT4;      // P6.4 output
    P6OUT |= BIT4;      // turn ON mic + op-amp

    // 2. Configure ADC12 for A5 (audio input)
    ADC12CTL0 = ADC12SHT0_2 | ADC12ON;
    ADC12CTL1 = ADC12SHP | ADC12CONSEQ_2;
    ADC12MCTL0 = ADC12INCH_5;
    ADC12CTL0 |= ADC12ENC | ADC12SC;              // enable ADC

}