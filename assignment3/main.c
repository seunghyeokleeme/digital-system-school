#include <avr/io.h>
#include <util/delay.h>

#define BUTTON_POWER PB0
#define BUTTON_INPUT PB1
#define BUTTON_STOP PB2
#define BUTTON_POINTS PD3

// 전역 변수
int count = 0; // 투입된 물품의 수

void updateSegmentDisplay(int number)
{
    // 7세그먼트 디스플레이에 숫자 표시
    unsigned char SegmentData[10] = {0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90};
    PORTE = SegmentData[number];
}

void setup()
{
    // 핀 모드 설정
    DDRB = 0xf0; // LED 포트를 출력으로 설정
    DDRE = 0xff; // 7세그먼트를 출력으로 설정

    // 초기 상태 설정
    PORTB = 0x00; // 모든 LED 끄기
    PORTE = 0xff; // 모든 7segment 끄기
    MCU_initialize();
    LCD_initialize();
}

void debounce()
{
    _delay_ms(50); // 디바운싱 지연 시간
}

void loop()
{
    // 전원 버튼이 눌렸을 때의 동작
    if (!(PINB & (1 << BUTTON_POWER)))
    {
        debounce();
        if (!(PINB & (1 << BUTTON_POWER))) // 상태 재확인
        {
            // 전원 버튼 로직...
            PORTB = 0xF0; // 모든 LED 켜기
            LCD_string(0x80, "수퍼빈,패트, 캔 최대 15개");
        }
    }

    if (!(PINB & (1 << BUTTON_INPUT)))
    {
        debounce();
        if (!(PINB & (1 << BUTTON_INPUT))) // 상태 재확인
        {
            // 투입 버튼이 눌렸을 때의 동작
            while (1)
            {
                for (int i = 0; i < 4; i++)
                {
                    PORTB = (1 << (i + 4)); // LED 순차적으로 켜기
                    _delay_ms(500);
                    if (count < 9)
                    {
                        count++;
                    }

                    char countStr[16];
                    sprintf(countStr, "투입 중: %d", count);
                    LCD_string(0x80, countStr);
                    updateSegmentDisplay(count);

                    if (PINB & (1 << BUTTON_STOP))
                    {
                        debounce();
                        if (PINB & (1 << BUTTON_STOP))
                        {
                            break; // 중지 버튼이 눌리면 루프 탈출
                        }
                    }
                }

                if (PINB & (1 << BUTTON_STOP))
                {
                    debounce();
                    if (PINB & (1 << BUTTON_STOP)) // 상태 재확인
                    {
                        break; // 중지 버튼 바깥 루프 탈출
                    }
                }
            }
        }

        if (PINB & (1 << BUTTON_STOP))
        {
            debounce();
            if (PINB & (1 << BUTTON_STOP)) // 상태 재확인
            {
                // 중지 버튼이 눌렸을 때의 동작
                PORTB = 0x00; // 모든 LED 끄기
                char countStr[16];
                sprintf(countStr, "그만 넣기: %d", count);
                LCD_string(0x80, countStr);
            }
        }

        if (PINB & (1 << BUTTON_POINTS))
        {
            debounce();
            if (PINB & (1 << BUTTON_POINTS)) // 상태 재확인
            {
                // 포인트 적립 버튼이 눌렸을 때의 동작
                PORTB = 0x0F; // 모든 LED 깜빡임
                _delay_ms(500);
                PORTB = 0x00;
                _delay_ms(500);
                char pointsStr[16];
                sprintf(pointsStr, "포인트: %d", count * 10);
                LCD_string(0x80, pointsStr);
            }
        }
        _delay_ms(100); // 디바운싱 지연
    }

    int main(void)
    {
        setup();
        while (1)
        {
            loop();
        }
        return 0;
    }
