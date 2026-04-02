void ADC_Interrupt(void)__interrupt[ADC12_VECTOR]
{
    unsigned int sample = ADC12MEM0;

    if (!sound_detected && sample > SOUND_THRESHOLD)
    {
        sound_time = TA0R;
        sound_detected = 1;
    }
}