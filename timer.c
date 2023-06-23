/*
 * timer.c
 *
 *  Created on: Aug 10, 2015
 *      Author: TNSolution
 */
#include "timer.h"
#include "msp430.h"

//====================Timers and PWM=============================
/*
 * TA0CCRO TA0.0 : P1.1, P1.5
 * TA0CCR1 TA0.1 : P1.2, P1.6
 * TA1CCRO TA1.0 : P2.0, P2.3
 * TA1CCR1 TA1.1 : P2.1, P2.2
 * TA1CCR2 TA1.2 : P2.4, P2.5
 */
void TimerA_Init()
{
	/* Configure timer A as a clock divider to generate timed interrupt */
	TACCTL0 = CCIE; // Enable counter interrupt on counter compare register 0
	TACTL = TASSEL_2 +ID_3 + MC_1; // Use the SMCLK to clock the counter, SMCLK/8, count up mode
	TACCR0 = 1600-1; // Set maximum count (Interrupt frequency 1MHz/8/12500 = 10Hz)


}

void TimerA1_Init()
{
	TA1CCTL0 = CM_3 + SCS + CCIS_0 + CCIE;// falling edge + CCI0A (P2.0)// + Capture Mode
	TA1CTL = TASSEL_2 + MC_2;
}
void PWM_TA01_Init(char BIT)
{
	P1DIR |= BIT;
	P1SEL |= BIT;

	TA0CCTL1 = OUTMOD_7;           	// TACCR1 reset/set
    TA0CTL   = TASSEL_2 + MC_1;    	// SMCLK, upmode
    TA0CCR0  = 12000;        		// PWM Period
    TA0CCR1  = 0;            		// TACCR1 PWM Duty Cycle
}
void PWM_TA11_Init(char BIT)
{

	P2DIR |= BIT0 + BIT1 + BIT4 + BIT5;
	P2SEL |= BIT1 + BIT4;          	//Selected for Pulse Width Modulation

    TA1CCTL1 = OUTMOD_7 ;           	// TACCR1 reset/set
    TA1CTL   = TASSEL_2 + MC_1;    	// SMCLK, upmode
    TA1CCR0  = 12000;        		// PWM Period
    TA1CCR1  = 0;            		// TACCR1 PWM Duty Cycle

}
void PWM_TA12_Init(char BIT)
{

	P2DIR |= BIT;
	P2SEL |= BIT;          	// Selected for Pulse Width Modulation

    TA1CCTL2 = OUTMOD_7;           	// TACCR2 reset/set
    TA1CCR0  = 12000;        		// PWM Period
    TA1CCR2  = 0;            		// TACCR1 PWM Duty Cycle
}

