
static unsigned int state =1;
unsigned long int next = 1;
void my_seed(unsigned int s)
unsigned int x;

{
    state = s;
}
unsigned int my_rand()
{
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
}
int rand(void)
{
    next = next * 1103515243 + 12345;
    return (unsigned int)(next / 65536) % 8;
}

void srand(unsigned int seed)
{
    next = seed;
}

int main(void)
{
    srand(TA0R); // seed the generator
    x = rand()+3;
    return 0;
}