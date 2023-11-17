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
	ms++;
	if (!mode)
	{ // 일반 모드일 때만 초, 분, 시 업데이트
		if (ms >= 1000)
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
	displayTimeOnLCD();
}

// 스위치 입력 상태를 업데이트하고 필요한 액션을 수행합니다.
void checkSwitches()
{
	// 현재 스위치 상태를 읽습니다.
	char currentSwitchState = ~PINB & 0x0F;

	// 스위치 상태가 변경되었는지 확인하고, 변경되었을 때만 디바운싱 로직을 적용합니다.
	if (currentSwitchState != lastSwitchState && ms - lastInputTime >= 200)
	{
		lastInputTime = ms; // 마지막 입력 시간을 업데이트합니다.

		// 모드 전환을 검사합니다.
		if (currentSwitchState & (1 << 0))
		{ // S5 - 모드 전환
			mode = !mode;
			cursorPosition = 0; // 설정 모드 종료시 커서 초기화
			if (!mode)
			{ // 설정 모드를 빠져나오면 시간을 다시 표시합니다.
				displayTimeOnLCD();
			}
		}
		else if (mode)
		{ // 설정 모드인 경우 커서 이동과 시간 조정을 처리합니다.
			if (currentSwitchState & (1 << 1))
			{ // S4 - 커서 이동
				cursorPosition = (cursorPosition + 1) % 4;
			}
			else if (currentSwitchState & (1 << 2))
			{ // S3 - 시간 증가
				updateTime(1);
			}
			else if (currentSwitchState & (1 << 3))
			{ // S2 - 시간 감소
				updateTime(-1);
			}
		}
		// 마지막 스위치 상태를 현재 상태로 업데이트합니다.
		lastSwitchState = currentSwitchState;
	}
}

void displayTimeOnLCD()
{
	char timeString[18];
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
		checkSwitches(); // 스위치 입력 처리
		if (!mode)		 // 설정 모드가 아닐 때만 시간을 표시합니다.
		{
			displayTimeOnLCD(); // 실제 시간을 표시합니다.
		}
		displayMode(); // 설정 모드 표시
	}
}
