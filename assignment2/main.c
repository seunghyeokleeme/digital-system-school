/*
 * LCD_Timer_counter
 * Timer Watch : 기능 표구 요구(예: 시간, AM/PM 표시, 현재 시간 설정 등)
 * Created : 작성 날짜
 * Author : 분반 이름 및 개발자 이름 모두
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "ATmega128_v20m.H"

unsigned int ms = 0;
int sec = 0, min = 0, hour = 12;
int isPM = 0;			// 오전은 0, 오후는 1
int mode = 0;			// 일반 모드는 0, 설정 모드는 1
int cursorPosition = 0; // 0: 시, 1: 분, 2: 초, 3: AM/PM
static unsigned char lastSwitchState = 0xFF;
unsigned long lastInputTime = 0;

ISR(TIMER0_OVF_vect)
{
	TCNT0 = 6;
	if (++ms >= 1000)
	{
		ms = 0;
		if (++sec >= 60)
		{
			sec = 0;
			if (++min >= 60)
			{
				min = 0;
				hour = (hour % 12) + 1;
				if (hour == 12)
				{
					isPM = !isPM;
				}
			}
		}
	}
}

// 시간 증가 및 감소 함수
void updateTime(int increment)
{
	switch (cursorPosition)
	{
	case 0: // 시
		hour = (hour + increment) % 12;
		hour = hour ? hour : 12;
		break;
	case 1: // 분
		min = (min + increment) % 60;
		min = min >= 0 ? min : 59;
		break;
	case 2: // 초
		sec = (sec + increment) % 60;
		sec = sec >= 0 ? sec : 59;
		break;
	case 3: // AM/PM 전환
		if (increment != 0)
			isPM = !isPM;
		break;
	}
}

// 스위치 입력 처리 함수
void checkSwitches()
{
	if (ms - lastInputTime < 200)
	{ // 200ms 동안 새 입력 무시 (디바운싱)
		return;
	}

	char switchState = ~PINB & 0x0F; // 스위치 상태 읽기

	// 스위치 상태가 변경되었는지 확인
	if (switchState != lastSwitchState)
	{
		lastInputTime = ms;

		if (switchState & (1 << 0))
		{ // S5 - 모드 전환/종료
			mode = !mode;
			cursorPosition = 0; // 설정 모드 종료시 커서 초기화
		}

		if (mode)
		{
			if (switchState & (1 << 1))
			{ // S4 - 커서 이동
				cursorPosition = (cursorPosition + 1) % 4;
			}
			if (switchState & (1 << 2))
			{ // S3 - 증가
				updateTime(1);
			}
			if (switchState & (1 << 3))
			{ // S2 - 감소
				updateTime(-1);
			}
		}

		lastSwitchState = switchState; // 스위치 상태 업데이트
	}
}

void displayTimeOnLCD()
{
	char timeString[16];
	sprintf(timeString, "%02d:%02d:%02d %s", hour, min, sec, isPM ? "PM" : "AM");
	LCD_string(0xC2, timeString);
}

// 설정 모드 표시
void displayMode()
{
	if (mode)
	{
		LCD_string(0x80, "Set Mode"); // 설정 모드일 때 LCD에 표시
	}
	else
	{
		LCD_string(0x80, "         "); // 일반 모드일 때 공백 표시
	}
}

int main(void)
{
	MCU_initialize();
	LCD_initialize();

	// 타이머 인터럽트 레지스터
	TIMSK = 0x01; // R/W 선택 TIMER 0 사용
	TCCR0 = 0x04; // 분주비 64
	TCNT0 = 6;	  // 6에서 시작 255가되어 256이 되면 OVF가 되어 인터럽트 구문을 실행한다.
	sei();		  // 전역 인터럽트

	while (1)
	{
		checkSwitches();	// 스위치 입력 처리
		displayTimeOnLCD(); // LCD에 시간 표시
		displayMode();		// 설정 모드 표시
	}
}
