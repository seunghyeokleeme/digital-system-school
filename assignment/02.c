#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#define MACRO_DELAY 20

void mesc_delay(int n);

unsigned char key, LED_Array[4] = {0x10, 0x20, 0x40, 0x80};

int main(void)
{
    DDRB = 0xf0;
    PORTB = 0x00;
    while (1)
    {
        while (!(~PINB & 0x0f));
        mesc_delay(MACRO_DELAY);

        key = ~PINB & 0x0f;
        if (key == 1) {
            key = 0;
        } else if (key == 2) {
            key = 1;
        } else if (key == 4) {
            key = 2;
        } else {
            key = 3;
        }
        PORTB = LED_Array[key];

        while (~PINB & 0x0f);
        mesc_delay(MACRO_DELAY);
    }
}
void mesc_delay(int n)
{
    for (; n > 0; n--)
    _delay_ms(1);
}
