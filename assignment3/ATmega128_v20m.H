/* -------------------------------------------------------------------------- */
/*  ATmega128_v20m.H                                                                	 */
/*  I/O Address Definition & User Function for ATmega128 V20 Target Board  */
/* -------------------------------------------------------------------------- */
#define LCD_DATABUS		*((volatile unsigned char *)0x2000) 	// LCD data output address
#define LCD_CONTROL		*((volatile unsigned char *)0x2100) 	// LCD control signal output address

#define RTC_SECOND		*((unsigned char *)0x2200) 	// DS12C887 RTC
#define RTC_SEC_ALM		*((unsigned char *)0x2201)
#define RTC_MINUTE		*((unsigned char *)0x2202)
#define RTC_MIN_ALM		*((unsigned char *)0x2203)
#define RTC_HOUR		*((unsigned char *)0x2204)
#define RTC_HOUR_ALM		*((unsigned char *)0x2205)
#define RTC_WEEKDAY		*((unsigned char *)0x2206)
#define RTC_DATE		*((unsigned char *)0x2207)
#define RTC_MONTH		*((unsigned char *)0x2208)
#define RTC_YEAR		*((unsigned char *)0x2209)
#define RTC_A 			*((unsigned char *)0x220A)
#define RTC_B			*((unsigned char *)0x220B)
#define RTC_C			*((unsigned char *)0x220C)
#define RTC_D			*((unsigned char *)0x220D)
#define RTC_FLAG		*((unsigned char *)0x220E)   
#define RTC_Value		*((unsigned int  *)0x220F)   

#define PPI_PORTA		*((unsigned char *)0x2300) 	// 8255-1 address  
#define PPI_PORTB		*((unsigned char *)0x2301)
#define PPI_PORTC		*((unsigned char *)0x2302)
#define PPI_CW			*((unsigned char *)0x2303)

void MCU_initialize(void)	/* initialize ATmege128 MCU */
{ 
     MCUCR = 0x80; 			// Enable external memory and I/O, Disable SLEEP mode
     XMCRA = 0x44; 			// 0x1100 - 0x7FFF (1 wait), 0x8000 - 0xFFFF (0 wait)
     XMCRB = 0x80; 			// Enable bus keeper. Use PC0-PC7 as high byte of address bus

     DDRB  = 0xF0;  		// PORTB7..4 = output, PORTB3..0 = Input
     PORTB = 0x00;

//     DDRD  = 0x00;  		// PORTD = Input
     DDRD  = 0xFD;  		// PORTD7..2 =Output, PORTD1..0 = Input
     PORTD = 0x00;

//     DDRE  = 0x02;  		// PORTE1 = output, All other PORTs are Input 
     DDRE  = 0x0A;  		// PORTE1, 3 = output, All other PORTs are Input 

//     PORTE = 0x00;
     PORTE = 0xE0;			// Pull up the PORTE7..5

     DDRF  = 0x00;  		// PORTF = Input
     PORTF = 0x00;             
     
     DDRG  = 0x1F;

     LCD_CONTROL = 0x00;
     LCD_DATABUS = 0x00;
}

void Delay_us(unsigned char time_us)		/* time delay for us */
{ 
     register unsigned char i;

     for(i = 0; i < time_us; i++) {     // 4 cycle +      
        asm (" PUSH  R0 ");     		// 2 cycle +       
        asm (" POP   R0 ");       		// 2 cycle +
        asm (" PUSH  R0 ");       		// 2 cycle +
        asm (" POP   R0 ");       		// 2 cycle +
        asm (" PUSH  R0 ");       		// 2 cycle +
        asm (" POP   R0 ");	     	// 2 cycle = 16 cycle = 1 us for 16MHz
     }
}

void Delay_ms(unsigned int time_ms)    	/* time delay for ms */
{ 
     register unsigned int i;

     for(i = 0; i < time_ms; i++) {
        Delay_us(250);
        Delay_us(250);
        Delay_us(250);
        Delay_us(250);
     }
}    

void LCD_command(unsigned char command)	/* write a command(instruction) to text LCD */
{
     LCD_CONTROL = 0x00;			// E = 0, Rs = 0
     LCD_CONTROL = 0x01;			// E = 1
     LCD_DATABUS = command;		// output command
     asm (" PUSH  R0 ");			// delay for about 250 ns
     asm (" POP   R0 ");
     LCD_CONTROL = 0x00;			// E = 0
     Delay_us(50);
}

void LCD_data(unsigned char data)		/* display a character on text LCD */
{
     LCD_CONTROL = 0x02;			// E = 0, Rs = 1
     LCD_CONTROL = 0x03;			// E = 1
     LCD_DATABUS = data;			// output data
     asm (" PUSH  R0 ");			// delay for about 250 ns
     asm (" POP   R0 ");
     LCD_CONTROL = 0x02;			// E = 0
     Delay_us(50);
}

void LCD_string(char command, char *string) /* display a string on LCD */
{
     LCD_command(command);			// start position of string
     while(*string != '\0') {			// display string
        LCD_data(*string);
        string++;
     }
}

void LCD_initialize(void)			/* initialize text LCD module */
{
     LCD_CONTROL = 0x03;				// E = 1, Rs = 1 (dummy write)
     LCD_CONTROL = 0x02;				// E = 0, Rs = 1
     Delay_ms(2);

     LCD_command(0x38);				// function set(8 bit, 2 line, 5x7 dot)
     LCD_command(0x0C);				// display control(display ON, cursor OFF)
     LCD_command(0x06);				// entry mode set(increment, not shift)
     LCD_command(0x01);				// clear display
     Delay_ms(2);
}

void LCD_1d1(float number)
{
     unsigned int i, j;

     j=(int)(number*10. +0.5);
     i = j / 10;
     LCD_data(i + '0');
     LCD_data('.');
     i = j % 10;
     LCD_data(i + '0');
} 

void LCD_2d(unsigned int number)
{
     unsigned int i;
     i = number / 10;
     if(i == 0) LCD_data(' ');
     else LCD_data(i + '0');
      
     i = number % 10;
     LCD_data(i + '0');
} 

void LCD_2hex(unsigned int number)
{
     unsigned int i;
     
	i = (number >> 4) & 0x0F;
     if(i <= 9) LCD_data(i + '0');
     else       LCD_data(i - 10 + 'A');
     
     i = number & 0x0F;
     if(i <= 9) LCD_data(i + '0');
     else       LCD_data(i - 10 + 'A');
} 

void LCD_2d1(float number)			/* display XX.X number on LCD */
{ 
     int i;

 //    number = fabs(number) + 0.05;
     i = (int)(number/10.);
     LCD_data(i + '0');

     i = (int)(number - i * 10);
     LCD_data(i + '0');

     LCD_data('.');

     i = (int)((number - (int)number) * 10.);
     LCD_data(i + '0');
}   

void LCD_1d2(float number)			/* display X.XX number on LCD */
{
	int i, j;
	j = (int)(number * 100. + 0.5);
	
	i = j / 100;
	LCD_data(i + '0');
	
	LCD_data('.');
	
	i = (j % 100) / 10;
	LCD_data(i + '0');
	
	i = j % 10;
	LCD_data(i + '0');
}

void LCD_3d(unsigned int number)
{
     unsigned int i,flag;
     flag = 0;
     i = number / 100;
     if(i == 0) LCD_data(' ');
     else {   LCD_data(i + '0');
              flag = 1;
          }
      
     number = number % 100;
     i = number / 10; 
     if((i == 0) && (flag == 0)) LCD_data(' ');
     else {   LCD_data(i + '0');
              flag = 1;
          }
     
     i = number % 10;
     LCD_data(i + '0');
}  

void LCD_4d(unsigned int number)
{
     unsigned int i,flag;
     flag = 0;
     i = number / 1000;
     if(i == 0) LCD_data(' ');
     else {   LCD_data(i + '0');
              flag = 1;
          }
      
     number = number % 1000;
     i = number / 100; 
     if((i == 0) && (flag == 0)) LCD_data(' ');
     else {   LCD_data(i + '0');
              flag = 1;
          }
     
     number = number % 100;
     i = number / 10;
     if((i == 0) && (flag == 0)) LCD_data(' ');
     else LCD_data(i + '0');
     
     i = number % 10;
     LCD_data(i + '0');
}    

void LCD_s4d(int number)
{
     unsigned int i, flag;
     flag = 0;      
     
     if(number < 0) {
     	number = number * (-1);
     	LCD_data('-');
     } else {
     	if(number == 0) LCD_data(' ');
     	else            LCD_data('+');
     }
     
     i = number / 1000;
     if(i == 0) LCD_data(' ');
     else {   LCD_data(i + '0');
              flag = 1;
          }
      
     number = number % 1000;
     i = number / 100; 
     if((i == 0) && (flag == 0)) LCD_data(' ');
     else {   LCD_data(i + '0');
              flag = 1;
          }
     
     number = number % 100;
     i = number / 10;
     if((i == 0) && (flag == 0)) LCD_data(' ');
     else LCD_data(i + '0');
     
     i = number % 10;
     LCD_data(i + '0');  
}  
   

void LCD_4hex(unsigned int number)
{
     unsigned int i;
     
     i = number >>12;
     if(i <= 9) LCD_data(i + '0');
     else       LCD_data(i - 10 + 'A');
     
     i = (number >> 8) & 0x000F;
     if(i <= 9) LCD_data(i + '0');
     else       LCD_data(i - 10 + 'A');
     
     i = (number >> 4) & 0x000F;
     if(i <= 9) LCD_data(i + '0');
     else       LCD_data(i - 10 + 'A');
     
     i = number & 0x000F;
     if(i <= 9) LCD_data(i + '0');
     else       LCD_data(i - 10 + 'A');
}  

void LCD_4d1(float number) 	/* xxxx.x */
{
     unsigned int i,j,flag;  
     flag = 0;
     
     j = (int)(number*10. + 0.5);
     i = j/10000; 
     if(i == 0) LCD_data(' ');					/* 10^3 */
     else {   LCD_data(i + '0');     
              flag = 1;
          }
     
     j = j % 10000;				
     i = j / 1000;                       			/* 10^2 */
     if((i == 0) && (flag == 0)) LCD_data(' ');
     else {   LCD_data(i + '0');
              flag = 1;
          }       
     
     j = j % 1000;				
     i = j / 100;                       			/* 10^1 */
     if((i == 0) && (flag == 0)) LCD_data(' ');
     else {   LCD_data(i + '0');
              flag = 1;
          }
     
     j = j % 100;				
     i = j / 10;                       				/* 10^0 */
     LCD_data(i + '0');

     LCD_data('.');     
     
     i = j % 10;								/* 10^-1 */
     LCD_data(i + '0');
}   

void LCD_6d(unsigned long number)
{
     unsigned int i,flag;  
     
     flag = 0; 
     i = number / 100000;                              /* 10^5 */
     if(i == 0) LCD_data(' ');
     else {   LCD_data(i + '0');
              flag = 1;
          } 
                     
     number = number % 100000;                		/* 10^4 */
     i = number / 10000;
     if((i == 0) && (flag == 0)) LCD_data(' ');
     else {   LCD_data(i + '0');
              flag = 1;
          }      
     
     number = number % 10000;                		/* 10^3 */
     i = number / 1000;
     if((i == 0) && (flag == 0)) LCD_data(' ');
     else {   LCD_data(i + '0');
              flag = 1;
          }   
          
     number = number % 1000;                			/* 10^2 */
     i = number / 100;
     if((i == 0) && (flag == 0)) LCD_data(' ');
     else {   LCD_data(i + '0');
              flag = 1;
          }   
          
     number = number % 100;                			/* 10^1 */
     i = number / 10;
     if((i == 0) && (flag == 0)) LCD_data(' ');
     else {   LCD_data(i + '0');
              flag = 1;
          }        
          
     i = number % 10;     						/* 10^0 */
     LCD_data(i + '0');   
}   

void Beep(void)					/* beep for 50 ms */
{ 
  PORTG |= _BV(3);				// buzzer on
  Delay_ms(50);					// delay 50 ms
  PORTG &= ~_BV(3);				// buzzer off
}



unsigned char Key_input(void)			/* input key SW1 - SW4 */
{ register unsigned char key;

  key = PINB & 0x0F;				// any key pressed ?
  if(key == 0x0F) return key;				// if no key, check key off
  while((PINB & 0x0F) != 0x0F) {
  Delay_ms(1);
  }
  Delay_ms(50);
  return key;
  } 

void LCD_display_onoff(int D, int C, int B) {
	unsigned cmd = 0b00001000;
	if (D == 1) cmd = cmd | 0b00000100;
	if (C == 1) cmd = cmd | 0b00000010;
	if (B == 1) cmd = cmd | 0b00000001;
	LCD_command(cmd);
}

void LCD_display_position(unsigned char row, unsigned char col) {
	unsigned char cmd = 0b10000000;
	cmd = cmd | ((row - 1) * 0x40 + (col - 1));
	LCD_command(cmd);
}