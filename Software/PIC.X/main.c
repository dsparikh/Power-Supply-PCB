
// PIC16F876A Configuration Bit Settings

// 'C' source line config statements

// CONFIG
#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = ON        // Watchdog Timer Enable bit (WDT enabled)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdlib.h>
#include <stdio.h>

#define _XTAL_FREQ 4000000 //4 Mhz

#define RS RB1
#define EN RB2
#define D4 RB4
#define D5 RB5
#define D6 RB6
#define D7 RB7

void Lcd_SetBit(char data_bit) //Based on the Hex value Set the Bits of the Data Lines
{
    if(data_bit& 1) 
        D4 = 1;
    else
        D4 = 0;

    if(data_bit& 2)
        D5 = 1;
    else
        D5 = 0;

    if(data_bit& 4)
        D6 = 1;
    else
        D6 = 0;

    if(data_bit& 8) 
        D7 = 1;
    else
        D7 = 0;
}

void Lcd_Cmd(char a)
{
    RS = 0;           
    Lcd_SetBit(a); //Incoming Hex value
    EN  = 1;         
        __delay_ms(4);
        EN  = 0;         
}

Lcd_Clear()
{
    Lcd_Cmd(0); //Clear the LCD
    Lcd_Cmd(1); //Move the curser to first position
}

void Lcd_Set_Cursor(char a, char b)
{
    char temp,z,y;
    if(a== 1)
    {
      temp = 0x80 + b - 1; //80H is used to move the curser
        z = temp>>4; //Lower 8-bits
        y = temp & 0x0F; //Upper 8-bits
        Lcd_Cmd(z); //Set Row
        Lcd_Cmd(y); //Set Column
    }
    else if(a== 2)
    {
        temp = 0xC0 + b - 1;
        z = temp>>4; //Lower 8-bits
        y = temp & 0x0F; //Upper 8-bits
        Lcd_Cmd(z); //Set Row
        Lcd_Cmd(y); //Set Column
    }
}

void Lcd_Start()
{
  Lcd_SetBit(0x00);
  for(int i=1065244; i<=0; i--)  NOP();  
  Lcd_Cmd(0x03);
    __delay_ms(5);
  Lcd_Cmd(0x03);
    __delay_ms(11);
  Lcd_Cmd(0x03); 
  Lcd_Cmd(0x02); //02H is used for Return home -> Clears the RAM and initializes the LCD
  Lcd_Cmd(0x02); //02H is used for Return home -> Clears the RAM and initializes the LCD
  Lcd_Cmd(0x08); //Select Row 1
  Lcd_Cmd(0x00); //Clear Row 1 Display
  Lcd_Cmd(0x0C); //Select Row 2
  Lcd_Cmd(0x00); //Clear Row 2 Display
  Lcd_Cmd(0x06);
}

void Lcd_Print_Char(char data)  //Send 8-bits through 4-bit mode
{
   char Lower_Nibble,Upper_Nibble;
   Lower_Nibble = data&0x0F;
   Upper_Nibble = data&0xF0;
   RS = 1;             // => RS = 1
   Lcd_SetBit(Upper_Nibble>>4);             //Send upper half by shifting by 4
   EN = 1;
   for(int i=2130483; i<=0; i--)  NOP(); 
   EN = 0;
   Lcd_SetBit(Lower_Nibble); //Send Lower half
   EN = 1;
   for(int i=2130483; i<=0; i--)  NOP();
   EN = 0;
}

void Lcd_Print_String(char *a)
{
    int i;
    for(i=0;a[i]!='\0';i++)
       Lcd_Print_Char(a[i]);  //Split the string using pointers and call the Char function 
}

void ADC_Initialize()
{
  ADCON0 = 0b01000001; //ADC ON and Fosc/16 is selected
  ADCON1 = 0b11000000; // Internal reference voltage is selected
}

unsigned int ADC_Read(unsigned char channel)
{
  ADCON0 &= 0x11000101; //Clearing the Channel Selection Bits
  ADCON0 |= channel<<3; //Setting the required Bits
  __delay_ms(2); //Acquisition time to charge hold capacitor
  GO_nDONE = 1; //Initializes A/D Conversion
  while(GO_nDONE); //Wait for A/D Conversion to complete
  return ((ADRESH<<8)+ADRESL); //Returns Result
}
int main()
{
    TRISB = 0x00;
    Lcd_Start();
    Lcd_Clear();
    
    char str[8];
    float voltage_val = 0.0;
    float current_val = 0.0;
    float RLim = 0.0;
    float current_limit_val = 0.0;
    
    ADC_Initialize();
    int adc = 0;
    
    Lcd_Set_Cursor(1,6);
    Lcd_Print_String("V");
    Lcd_Set_Cursor(1,16);
    Lcd_Print_String("A");
    Lcd_Set_Cursor(2,1);
    Lcd_Print_String("Curr Limit");
    Lcd_Set_Cursor(2,16);
    Lcd_Print_String("A");
    
    adc = (ADC_Read(0));
    voltage_val = adc*0.488281/100/0.2;
    float voltage_val_prev = voltage_val;
    sprintf(str, "%.2f", (float) voltage_val);
    Lcd_Set_Cursor(1,1);
    Lcd_Print_String(str);
    voltage_val_prev = voltage_val;

    adc = (ADC_Read(1));
    current_val = adc*0.488281/100;
    float current_val_prev = current_val;
    sprintf(str, "%.2f", (float) current_val);
    Lcd_Set_Cursor(1,11);
    Lcd_Print_String(str);
    current_val_prev = current_val;

    
    adc = (ADC_Read(2));
    current_limit_val = adc*0.488281/100;
    float current_limit_val_prev = current_limit_val;
    sprintf(str, "%.1f", (float) current_limit_val);
    Lcd_Set_Cursor(2,12);
    Lcd_Print_String(str);
    current_limit_val_prev = current_limit_val;
    while(1)
    {
        
        adc = (ADC_Read(0));
        voltage_val = adc*0.488281/100/0.2;
        if (voltage_val != voltage_val_prev)
        {
            sprintf(str, "%.2f", (float) voltage_val);
            Lcd_Set_Cursor(1,1);
            Lcd_Print_String(str);
            voltage_val_prev = voltage_val;
        }
      
        adc = (ADC_Read(1));
        current_val = adc*0.488281/100;
        if (current_val != current_val_prev)
        {
            sprintf(str, "%.2f", (float) current_val);
            Lcd_Set_Cursor(1,11);
            Lcd_Print_String(str);
            current_val_prev = current_val;
        }
        
        adc = (ADC_Read(2));
        current_limit_val = adc*0.488281/100;
        RLim = 10000*current_limit_val/(5-current_limit_val);
        current_limit_val = 0.8*1000/RLim;
        
        if (current_limit_val != current_limit_val_prev)
        {
            if (current_limit_val >= 2.5)
            {
                current_limit_val = 2.5;
            }
            sprintf(str, "%.1f", (float) current_limit_val);
            Lcd_Set_Cursor(2,12);
            Lcd_Print_String(str);
            current_limit_val_prev = current_limit_val;
        }
        
        //__delay_ms(1000);
    }
    return 0;
}