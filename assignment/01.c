#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

unsigned char LED_Array[4] = {0x10, 0x20, 0x40, 0x80};

int main(void)
{
    short int i = 3;
    DDRB = 0xf0;
    PORTB = LED_Array[i];
    while (1)
    {
        if (--i == -1) i = 3;
        PORTB = LED_Array[i];
        _delay_ms(500);
    }
}
