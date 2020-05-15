//======================================================================
//
//        Copyright (C) 2020 Jobcrazy    
//        All rights reserved
//        https://github.com/Jobcrazy
//
//======================================================================

#include <reg52.h>

//Button registers
sbit Step1m = P3^4;
sbit Step10m = P3^5;
sbit Step1h = P3^7;
sbit StepStart = P3^6;

//Beep register
sbit Beep = P1^6;

//Time Led Dot register
sbit TIME_DOT = P1^5;

//Time Led Segment register
sbit ShuMa_Duan_0 = P1^3;
sbit ShuMa_Duan_1 = P1^0;
sbit ShuMa_Duan_2 = P1^2;
sbit ShuMa_Duan_3 = P1^1;

//Initial settings
#define DEFAULT_HOUR 00
#define DEFAULT_MINUTE 00
#define DEFAULT_SECOND 00
volatile int Hour = DEFAULT_HOUR;
volatile int Minute = DEFAULT_MINUTE;
volatile int Second = DEFAULT_SECOND;

//Led Number Definitions
code unsigned char ShuMa_Data[10] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92,0x82,0xF8,0x80,0x90};

//Led Number Indexes
volatile int Shi_Left = 0;
volatile int Ge_Left = 0;
volatile int Shi_Right = 0;
volatile int Ge_Right = 0;

//Delay 1 minisecond
void Delay_1ms( int i )
{       
    for( ; i > 0; --i )
    {
        int j = 0;
        for( ; j < 110; ++j )
        {
        }
    }
}

//Timer Settings
void Init_Timer()
{
    TMOD = 0x11; //Using timer 0 and timer 1
    TH0  = 0xB8;
    TL0  = 0x00;  //Timer0 = 20ms
    
    TH1  = 0xB8;
    TL1  = 0x00;  //Timer1 = 20ms
    
    TR0  = 0x01;  //Enable Timer 0
    TR1  = 0x00;
}

//Enable Timer
void Init_Interrupt()
{
    ET0 = 1;
    ET1 = 1;
    EA = 1;
}

//Show Number on the Timer Led
void  ShuMa(unsigned char ShuMa_Wei, unsigned char ShuMa_Index)
{
    P2 = ShuMa_Data[ShuMa_Index];
    
    if( ShuMa_Wei == 0 )
    {
        ShuMa_Duan_0 = 1;
        ShuMa_Duan_1 = 0;
        ShuMa_Duan_2 = 0;
        ShuMa_Duan_3 = 0;	
    }
    else if( ShuMa_Wei == 1 )
    {
        ShuMa_Duan_0 = 0;
        ShuMa_Duan_1 = 1;
        ShuMa_Duan_2 = 0;
        ShuMa_Duan_3 = 0;
    }
    else if( ShuMa_Wei == 2 )
    {
        ShuMa_Duan_0 = 0;
        ShuMa_Duan_1 = 0;
        ShuMa_Duan_2 = 1;
        ShuMa_Duan_3 = 0;
    }
    else if( ShuMa_Wei == 3 )
    {
        ShuMa_Duan_0 = 0;
        ShuMa_Duan_1 = 0;
        ShuMa_Duan_2 = 0;
        ShuMa_Duan_3 = 1;
    }
    
    //We must delay 1ms here, or else the Led will not show a correct number
    Delay_1ms(1);
}

//Convert the time to a displayable format
void CalcSecond()
{
    if( 0 < Hour )
    {
        Shi_Left = Hour / 10;
        Ge_Left = Hour % 10;
        
        Shi_Right = Minute / 10;
        Ge_Right =  Minute % 10;
    }
    else
    {
        Shi_Left = Minute / 10;
        Ge_Left = Minute % 10;
        
        Shi_Right = Second / 10;
        Ge_Right = Second % 10;
    }	
}

//Counter Timer
void Interrupt_Timer_0() interrupt 1
{
    static int Counter = 0;
    TH0 = 0xB8;
    TL0 = 0x00;
    
    ++Counter;
    
    if( 49 < Counter )
    {
        //Ths scope will run 1 time per second
        Counter = 0;
        TIME_DOT = ~TIME_DOT;
        
        Second--;
        
        //Let's see how long time is left
        if( Second < 0 )
        {
            Second = 59;
            Minute -= 1;
        }
        
        if( Minute < 0 )
        {
            if( 0 < Hour )
            {
                Hour--;
                Minute = Second = 59; 
            }
            else
            {
                Hour = Minute = Second = 0;
                TR0 = 0; //Disable counter timer
                TR1 = 1; //Enable Beep Timer
            }
        }
        
        CalcSecond();
    }	 
}

//Beep timer
void Interrupt_Timer_1() interrupt 3
{
    static int n = 0, Counter = 0;
    TH1 = 0xB8;
    TL1 = 0x00;
    
    ++Counter;
    
    if( Counter < 10 )
    {
        Beep = 1;
    }
    else if( Counter < 30 )
    {
        Beep = 0;
    }
    else if( Counter > 49 )
    {
        Counter = 0;
        TIME_DOT = ~TIME_DOT;
        
        n++;
        
        if( n >= 5 )
        {
            Hour = 	DEFAULT_HOUR;
            Minute = DEFAULT_MINUTE;
            Second = DEFAULT_SECOND;
            n = 0;
            
            //Disable two timers
            TR0 = 0;
            TR1 = 0;	
        }	
    }
}

//Covert time to be recognizable
void formatTime()
{
    if( Minute >= 60 )
    {
        Minute = 0;
        Hour = Hour + 1;	
    }
    
    if( Hour >= 99 )
    {
        Hour = 0;
        Minute = 0;
        Second = 0;
    }
    
    CalcSecond();	
}

void main()
{
    //To avoid long press
    int Pressed = 0;
    
    //Light Timer Dot
    TIME_DOT = 0;

    //Turn Off Beep
    Beep = 0;
    
    //Make P3 output high-level signal
    P3 = 0xFF;
    
    CalcSecond();
    
    //Infinite loop
    while(1)
    {
        //Covert time to be recognizable
        formatTime();

        //Show time the led screen
        ShuMa( 0, Shi_Left );			
        ShuMa( 1, Ge_Left );
        ShuMa( 2, Shi_Right );
        ShuMa( 3, Ge_Right );
        
        if( 0 == TR0 && 0 == TR1 ) //If timer is not running
        {
            if( 0 == Step1m && 0 == Pressed )
            {
                //If Step1m button is pressed down
                Pressed = 1;
                Minute = Minute + 1;
            }
            else if( 0 == Step10m && 0 == Pressed )
            {
                //If Step10m button is pressed down
                Pressed = 1;
                Minute = Minute + 10;
            }
            else if( 0 == Step1h && 0 == Pressed )
            {
                //If Step1h button is pressed down
                Pressed = 1;
                Hour = Hour + 1;	
            }
            else if( 0 == StepStart && 0 == Pressed )
            {
                //If StepStart button is pressed down
                Pressed = 1;
                if ( 0 != Hour || 0 != Minute || 0 != Second )
                {
                    Init_Timer();
                    Init_Interrupt();
                }                		
            }
            else
            {
                if( 1 == Step1m && 1 == Step10m && 1 == Step1h && 1 == StepStart )
                {
                    //If no button is pressed down
                    Pressed = 0;
                }	
            }
        }	
        
    }
}