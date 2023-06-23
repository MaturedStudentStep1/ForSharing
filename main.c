#include <msp430.h> 
#include "uart.h"

/**
 * main.c
 */

#define STX             '{'
#define ETX             '}'

#define LEFT_PWM        BIT5    //P2.5
#define LEFT_FORWARD    BIT4    //P2.4
#define LEFT_BACK       BIT6    //P1.6

#define LEFT_ENCA       BIT5    //P1.5
#define LEFT_ENCB       BIT4    //P1.4

#define RIGHT_PWM       BIT1    //P2.1
#define RIGHT_FORWARD   BIT7    //P1.7
#define RIGHT_BACK      BIT3    //P2.3

#define RIGHT_ENCA      BIT0    //P2.0
#define RIGHT_ENCB      BIT2    //P2.2

#define LED             BIT0    //P1.0

long long _Left_Counter = 0;
volatile unsigned char _Left_State_ENCA;
volatile unsigned char _Left_LastState_ENCA;

long long _Right_Counter = 0;
volatile unsigned char _Right_State_ENCA;
volatile unsigned char _Right_LastState_ENCA;

unsigned char _Start = 0;

unsigned char MAC_Address = 0x65;  // 'e'
unsigned char _Address_Send;
unsigned char _FlagBegin = 0;
volatile unsigned char _UART_Buffer[20];
volatile unsigned int _UART_Index = 0;
unsigned int _PWM_Period = 1024;    // PWM Period

void MotorSetup();
void SetLeftMotorSpeed();
void SetRightMotorSpeed();
void SetSpeeds(int lspeed, int rspeed);
void SetBrakes();
void GetPosition();
void ProcessPacketComming();
void Delay_Second();
unsigned char DigitalRead_P1(unsigned char P_INPUT);
unsigned char DigitalRead_P2(unsigned char P_INPUT);

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
    BCSCTL1 = CALBC1_16MHZ;
    DCOCTL = CALDCO_16MHZ;

    P1DIR |= LED;
    MotorSetup();
    uart_init(UART_16MHz_115200);
    __bis_SR_register(GIE); // interrupts of UART

    _Left_LastState_ENCA = DigitalRead_P1(LEFT_ENCA);
    _Right_LastState_ENCA = DigitalRead_P2(RIGHT_ENCA);
    //SetRightMotorSpeed(700);
    //SetLeftMotorSpeed(700);
    while(1)
    {
            _Left_State_ENCA = DigitalRead_P1(LEFT_ENCA); // Reads the "current" state of the outputA
            // If the previous and the current state of the outputA are different, that means a Pulse has occured
            if (_Left_State_ENCA != _Left_LastState_ENCA)
            {
                // If the outputB state is different to the outputA state, that means the encoder is rotating clockwise
                if (DigitalRead_P1(LEFT_ENCB) != _Left_State_ENCA)
                {
                    _Left_Counter = _Left_Counter + 1;
                }
                else
                {
                    _Left_Counter = _Left_Counter - 1;
                }

            }
            _Left_LastState_ENCA = _Left_State_ENCA; // Updates the previous state of the outputA with the current state
            //=============================================================
            _Right_State_ENCA = DigitalRead_P2(RIGHT_ENCA); // Reads the "current" state of the outputA
           // If the previous and the current state of the outputA are different, that means a Pulse has occured
           if (_Right_State_ENCA != _Right_LastState_ENCA)
           {
               // If the outputB state is different to the outputA state, that means the encoder is rotating clockwise
               if (DigitalRead_P2(RIGHT_ENCB) == _Right_State_ENCA)
               {
                   _Right_Counter = _Right_Counter + 1;
               }
               else
               {
                   _Right_Counter = _Right_Counter - 1;
               }

           }
           _Right_LastState_ENCA = _Right_State_ENCA; // Updates the previous state of the outputA with the current state
        }

}
//==========================================================

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
    if (IFG2 & UCA0RXIFG)
    {
        unsigned char zByteComming = UCA0RXBUF;
        //uart_send_char(zByteComming);
        if (_FlagBegin == 0)
        {
            if (zByteComming == STX)
            {
                _UART_Index = 0;
                _FlagBegin = 1;
            }
        }
        else
        {
            if (zByteComming == ETX)
            {
                ProcessPacketComming();
                _FlagBegin = 0;
                _UART_Index = 0;
            }
            else
            {
                _UART_Buffer[_UART_Index] = zByteComming;
                _UART_Index++;
            }
        }
        IFG2 &= ~UCA0RXIFG;         // clear UART-Receive Flag
    }
}
//*********************************************************************************************
void ProcessPacketComming()
{
    unsigned char zAddress_Receive;
    unsigned long zLeftSpeed, zRightSpeed;
    unsigned char zDirection;
    zAddress_Receive = _UART_Buffer[0];
    _Address_Send = _UART_Buffer[1];

    if (zAddress_Receive == MAC_Address)
    {
        P1OUT ^= LED;
        char zStyle = _UART_Buffer[2];
        char zCommand = _UART_Buffer[3];
        switch (zCommand) //+12345-78901
        {
            case '0' : // '0' Set Position
                _Left_Counter = 0;
                _Right_Counter = 0;
                break;
            case 'P' : // 'P' Request get Position
                uart_send_char('{');
                uart_send_char('#');
                uart_send_int32(_Left_Counter);
                uart_send_int32(_Right_Counter);
                uart_send_char('}');
                break;
            case 'L':         // 'L' Set Speed for Left Wheel
                uart_send_char(zCommand);
                zDirection = _UART_Buffer[4];
                zLeftSpeed =    (_UART_Buffer[5] - '0') * 10000
                                + (_UART_Buffer[6] - '0') * 1000
                                + (_UART_Buffer[7] - '0') * 100
                                + (_UART_Buffer[8] - '0') * 10;
                                + (_UART_Buffer[9] - '0');
                if(zDirection == '-')
                    zLeftSpeed = (-1)*zLeftSpeed;

                SetLeftMotorSpeed(zLeftSpeed);
                break;

            case 'R':
                zDirection = _UART_Buffer[4];
                    zRightSpeed =   (_UART_Buffer[5] - '0') * 10000
                                + (_UART_Buffer[6] - '0') * 1000
                                + (_UART_Buffer[7] - '0') * 100
                                + (_UART_Buffer[8] - '0') * 10
                                + (_UART_Buffer[9] - '0');
                if(zDirection == '-')
                    zRightSpeed = (-1)*zRightSpeed;

                SetRightMotorSpeed(zRightSpeed);
                break;
            case 'U':         //Request 'U' Update Speed
                uart_send_char(zCommand);
                zDirection = _UART_Buffer[4];
                zLeftSpeed =      (_UART_Buffer[5] - '0') * 10000
                                + (_UART_Buffer[6] - '0') * 1000
                                + (_UART_Buffer[7] - '0') * 100
                                + (_UART_Buffer[8] - '0') * 10;
                                + (_UART_Buffer[9] - '0');

                if(zDirection == '-')
                   zLeftSpeed = (-1)*zLeftSpeed;

                zDirection = _UART_Buffer[10];
                zRightSpeed =   (_UART_Buffer[11] - '0') * 10000
                            + (_UART_Buffer[12] - '0') * 1000
                            + (_UART_Buffer[13] - '0') * 100
                            + (_UART_Buffer[14] - '0') * 10
                            + (_UART_Buffer[15] - '0');

                if(zDirection == '-')
                    zRightSpeed = (-1)*zRightSpeed;

                SetSpeeds(zLeftSpeed,zRightSpeed);
                GetPosition();
                break;
            case 'S': //'S'
                SetBrakes();
                GetPosition();
                break;
        }
    }
}
//===================================================
void MotorSetup()
{
    P2DIR |= LEFT_PWM + RIGHT_PWM;//Pin 2.1 -> left motor speed ; Pin 2.5 -> Right Motor Speed
    P2SEL |= LEFT_PWM + RIGHT_PWM;

    //-------------------LEFT-------------------
    P2DIR |= LEFT_FORWARD;
    P1DIR |= LEFT_BACK;

    P2OUT &= ~LEFT_FORWARD;
    P1OUT &= ~LEFT_BACK;

    //-------------------RIGHT-------------------
    P1DIR |= RIGHT_FORWARD;             //POUT
    P2DIR |= RIGHT_BACK;

    P2DIR &= ~(RIGHT_ENCA + RIGHT_ENCB); //     PIN Input

    P1OUT &= ~RIGHT_FORWARD;
    P2OUT &= ~RIGHT_BACK;

    /*** Timer1_A Set-Up ***/
    TA1CCR0 |= _PWM_Period - 1;
    TA1CCTL1 |= OUTMOD_7;
    TA1CCTL2 |= OUTMOD_7;
    TA1CCR1 |= 0;
    TA1CCR2 |= 0;
    TA1CTL |= TASSEL_2 + MC_1;
}

void SetLeftMotorSpeed(int speed)
{

    if (speed > 0) // Forward
    {
        P2OUT |= LEFT_FORWARD;
        P1OUT &= ~LEFT_BACK;
        if (speed > _PWM_Period - 1)
        {
            speed = _PWM_Period - 1; //prevent CCR2 from being negative
        }
        TA1CCR2 = speed;
    }
    else if (speed < 0) // Back
    {
        P2OUT &= ~LEFT_FORWARD;
        P1OUT |= LEFT_BACK;

        speed = -speed;
        if (speed > _PWM_Period - 1)
        {
            speed = _PWM_Period - 1; //prevent CCR1 from being negative
        }
        TA1CCR2 = speed;
    }
}

void SetRightMotorSpeed(int speed)
{
    if (speed > 0) // Forward
    {
        P1OUT |= RIGHT_FORWARD;
        P2OUT &= ~RIGHT_BACK;

        if (speed > _PWM_Period - 1)
        {
            speed = _PWM_Period -1; //prevent CCR2 from being negative
        }
        TA1CCR1 = speed;
    }
    else if (speed < 0) // Back
    {
        P1OUT &= ~RIGHT_FORWARD;
        P2OUT |= RIGHT_BACK;

        speed = -speed;
        if (speed > _PWM_Period - 1)
        {
            speed = _PWM_Period - 1; //prevent CCR2 from being negative
        }
        TA1CCR1 = speed;
    }
}
void SetSpeeds(int lspeed, int rspeed)
{
    SetLeftMotorSpeed(lspeed);
    SetRightMotorSpeed(rspeed);
}
void SetBrakes()
{
    P2OUT &= ~LEFT_FORWARD;
    P1OUT &= ~LEFT_BACK;

    P1OUT &= ~RIGHT_FORWARD;
    P2OUT &= ~RIGHT_BACK;

    TA1CCR1 = _PWM_Period - 1;
    TA1CCR2 = _PWM_Period - 1;
}
void GetPosition()
{
    uart_send_char('{');
    uart_send_char('#');
    uart_send_int32(_Left_Counter);
    uart_send_int32(_Right_Counter);
    uart_send_char('}');
}
void Delay_Second(unsigned int ms)
{
    unsigned int i;
    for(i=0;i<ms;i++)
    {
        __delay_cycles(6000000);
    }
}
unsigned char DigitalRead_P1(unsigned char P_INPUT)
{
    if (!(P1IN & P_INPUT))  //PIN is low
       return 0;
    else
       return 1;

}
unsigned char DigitalRead_P2(unsigned char P_INPUT)
{
    if (!(P2IN & P_INPUT))  //PIN is low
       return 0;
    else
       return 1;

}
