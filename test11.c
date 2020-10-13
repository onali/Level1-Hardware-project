#define D0 eS_PORTD0
#define D1 eS_PORTD1
#define D2 eS_PORTD2
#define D3 eS_PORTD3
#define D4 eS_PORTD4
#define D5 eS_PORTD5
#define D6 eS_PORTD6
#define D7 eS_PORTD7
#define RS eS_PORTC6
#define EN eS_PORTC7
#define get_bit(reg,bitnum) ((reg & (1<<bitnum))>>bitnum)

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>														
#include <stdlib.h>
#include "lcd.h"  
#define DHT11_PIN 1 
uint8_t c=0,I_RH,D_RH,I_Temp,D_Temp,CheckSum;     

void buzzer(int i)
{
	if(i)
	{
		PORTB |= 1<<PINB0;
		_delay_ms(500);
		PORTB &= ~(1<<PINB0);
		_delay_ms(100);
	}
	else
		PORTB &= ~(1<<PINB0);	
}

void InitADC()
{
	ADMUX=(1<<REFS0);                         // For Aref=AVcc;
	ADCSRA=(1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0); //Rrescalar div factor =128
}

uint16_t ReadADC(uint8_t ch)
{
	//Select ADC Channel ch must be 0-7
	ch=ch&0b00000111;
	ADMUX|=ch;

	//Start Single conversion
	ADCSRA|=(1<<ADSC);

	//Wait for conversion to complete
	while(!(ADCSRA & (1<<ADIF)));

	//Clear ADIF by writing one to it
	//Note you may be wondering why we have write one to clear it
	//This is standard way of clearing bits in io as said in datasheets.
	//The code writes '1' but it result in setting bit to '0' !!!

	ADCSRA|=(1<<ADIF);

	return(ADC);
}

void Wait()
{
	uint8_t i;
	for(i=0;i<20;i++)
	_delay_loop_2(0);
}

void Request()				
{
	DDRA |= (1<<DHT11_PIN);
	PORTA &= ~(1<<DHT11_PIN);
	_delay_ms(20);			
	PORTA |= (1<<DHT11_PIN);
}

void Response()				
{
	DDRA &= ~(1<<DHT11_PIN);
	while(PINA & (1<<DHT11_PIN));
	while((PINA & (1<<DHT11_PIN))==0);
	while(PINA & (1<<DHT11_PIN));
}

uint8_t Receive_data()		
{
	for (int q=0; q<8; q++)
	{
	
		while((PINA & (1<<DHT11_PIN)) == 0);
		_delay_us(30);
		
		if(PINA & (1<<DHT11_PIN))
			c = (c<<1)|(0x01);
		else			
			c = (c<<1);
		
		while(PINA & (1<<DHT11_PIN));
	
	}
	return c;
}

int main(void)
{
	int apparentTemp[15][21] = {{52, 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 }, 
							    {49, 53, 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
							    {47, 50, 55, 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
						 	    {44, 47, 51, 55, 61, 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
							    {42, 44, 47, 51, 55, 59, 64, 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
							    {39, 42, 44, 46, 49, 53, 57, 62, 66, 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
							    {37, 39, 41, 42, 44, 47, 51, 54, 58, 62, 66, 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
							    {35, 36, 38, 39, 41, 43, 45, 48, 51, 54, 57, 61, 65, 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
							    {33, 34, 35, 36, 37, 38, 40, 42, 43, 46, 49, 52, 56, 59, 62, 0 , 0 , 0 , 0 , 0 , 0 },
							    {31, 32, 32, 33, 34, 34, 36, 37, 38, 40, 42, 43, 46, 48, 51, 54, 58, 0 , 0 , 0 , 0 },
							    {28, 29, 29, 30, 31, 31, 32, 33, 34, 35, 36, 37, 38, 39, 41, 43, 45, 47, 50, 0 , 0 },
							    {26, 26, 27, 27, 28, 28, 29, 29, 30, 31, 31, 32, 32, 33, 34, 35, 36, 37, 39, 41, 42},
							    {23, 23, 24, 24, 25, 25, 26, 26, 26, 27, 27, 27, 28, 28, 29, 30, 30, 31, 31, 32, 33},
							    {21, 21, 21, 22, 22, 22, 23, 23, 23, 23, 24, 24, 24, 24, 25, 25, 26, 26, 26, 26, 27},
							    {18, 18, 18, 18, 19, 19, 19, 19, 20, 20, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22}}; 
								
	DDRD = 0xFF;
	DDRC = 0xFF;
	DDRB |= 1<<DDB0;
	DDRA |= 1<<DDA7;
	DDRA &= ~(1 << DDA2);
	
	Lcd8_Init();
	
	char data[5];
	int adc_result, p = 1;
	InitADC();
	
	
	
	
	while(1)
	{
		Request();		
		Response();		
		I_RH=Receive_data();	
		D_RH=Receive_data();	
		I_Temp=Receive_data();	
		D_Temp=Receive_data();	
		CheckSum=Receive_data();
		
		adc_result=ReadADC(0);
		adc_result= (int)((adc_result * 7.94) - 3127.46);   
		
		if (PINA & (1<<PA2) && I_Temp < 40 && adc_result < 800)
		{
			PORTC &= ~(1<<PC0) & ~(1<<PC1);
			PORTA &= ~(1<<PA7);
			if(p==1)
			{
				
				if ((I_RH + D_RH + I_Temp + D_Temp) != CheckSum)
				{
					Lcd8_Set_Cursor(1,0);
					Lcd8_Write_String("Error");
				}
				else
				{
					Lcd8_Clear();
					Lcd8_Set_Cursor(1,0);
					Lcd8_Write_String("Humidity =");
					Lcd8_Set_Cursor(2,0);
					Lcd8_Write_String("Temp = ");
					
					itoa(I_RH,data,10);
					Lcd8_Set_Cursor(1,11);
					Lcd8_Write_String(data);
					Lcd8_Write_String(".");
					
					itoa(D_RH,data,10);
					Lcd8_Write_String(data);
					Lcd8_Write_String("%");
					itoa(I_Temp,data,10);
					
					
					Lcd8_Set_Cursor(2,7);
					Lcd8_Write_String(data);
					Lcd8_Write_String(".");
					
					itoa(D_Temp,data,10);
					Lcd8_Write_String(data);
					Lcd8_Write_String(" C");
					
				}
				p = 2;
			}
			else if(p==2)
			{
				
				int hum = I_RH + D_RH/10.0;
				int tem = (I_Temp + D_Temp/10.0)*1.8 +32;
				itoa(apparentTemp[14 - (tem - 70) / 5][hum / 5], data, 10);
				Lcd8_Clear();
				Lcd8_Write_String("Apparent Temp");
				Lcd8_Set_Cursor(2,0);
				Lcd8_Write_String(data);
				Lcd8_Write_String(" C");
				p = 0;
			}
			else
			{
				
				Lcd8_Clear();
				Lcd8_Write_String("CO2 = ");
				sprintf(data, "%d", adc_result);
				Lcd8_Write_String(data);
				Lcd8_Write_String(" ppm");
				p = 1;
			}
		}
		else
		{
			if(I_Temp >= 40 )
			{	
				Lcd8_Clear();
				Lcd8_Write_String("Temperature ");
				Lcd8_Set_Cursor(2,0);
				Lcd8_Write_String("Warning!");
				PORTC |= 1<<PC1;
				buzzer(1);
			}
			else
				PORTC &= ~(1<<PC1);
			
			if(!(PINA & (1<<PA2)))
			{
				Lcd8_Clear();
				Lcd8_Write_String("DETECTED");
				Lcd8_Set_Cursor(2,0);
				Lcd8_Write_String("LP GAS");
				PORTC |= 1<<PC0;
				buzzer(1);
			}
			else
				PORTC &= ~(1<<PC0);
			
			if(adc_result >= 800)
			{
				Lcd8_Clear();
				Lcd8_Write_String("CO2");
				Lcd8_Set_Cursor(2,0);
				Lcd8_Write_String("Warning!");
				PORTA |= 1<<PA7;
				buzzer(1);
			}
			else
				PORTA &= ~(1<<PA7);
		}
		_delay_ms(1500);
		
	}

}
