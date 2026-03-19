#include <stdio.h>

unsigned long int next = 1;

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
    srand(5282); // seed the generator
    int x = rand()+3;
    printf("%d\n", x);
    return 0;
}
