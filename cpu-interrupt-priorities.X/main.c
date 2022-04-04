/*
    \file   main.c
    \brief  CPU Interrupt priorities
    (c) 2022 Microchip Technology Inc. and its subsidiaries.
    Subject to your compliance with these terms, you may use Microchip software and any
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party
    license terms applicable to your use of third party software (including open source software) that
    may accompany Microchip software.
  
    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS
    FOR A PARTICULAR PURPOSE.
  
    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS
    SOFTWARE.
*/

#define F_CPU                       4000000UL

#include <xc.h>
#include <util/delay.h>
#include <avr/interrupt.h>

/* Default fuses configuration:
- BOD disabled
- Oscillator in High-Frequency Mode
- UPDI pin active(WARNING: DO NOT CHANGE!)
- RESET pin used as GPIO
- CRC disabled
- MVIO enabled for dual supply
- Watchdog Timer disabled
*/
FUSES =
{
.BODCFG = ACTIVE_DISABLE_gc | LVL_BODLEVEL0_gc | SAMPFREQ_128Hz_gc | SLEEP_DISABLE_gc,
.BOOTSIZE = 0x0,
.CODESIZE = 0x0,
.OSCCFG = CLKSEL_OSCHF_gc,
.SYSCFG0 = CRCSEL_CRC16_gc | CRCSRC_NOCRC_gc | RSTPINCFG_GPIO_gc | UPDIPINCFG_UPDI_gc,
.SYSCFG1 = MVSYSCFG_DUAL_gc | SUT_0MS_gc,
.WDTCFG = PERIOD_OFF_gc | WINDOW_OFF_gc,
};

#define USART0_BAUD_RATE(BAUD_RATE) ((float)(F_CPU * 64 / (16 * (float)(BAUD_RATE))) + 0.5)
#define LED_ON()                    PORTF.OUTCLR = PIN5_bm
#define LED_OFF()                   PORTF.OUTSET = PIN5_bm
#define PULSE_TIME                  1000 /* time in ms */
#define TCB0_TOP_VALUE              10
#define TCB1_TOP_VALUE              20

static void USART0_init(void);
static void TCB0_init(void);
static void TCB1_init(void);
static void LED_init(void);
static void CPUINT_init(void);

static void USART0_sendChar(char c);

ISR(USART0_RXC_vect)
{
    char c = USART0.RXDATAL;
    /* Echo the received character */
    USART0_sendChar(c);
}

ISR(TCB0_INT_vect)
{
    /* Clear the interrupt flag */
    TCB0.INTFLAGS |= TCB_CAPT_bm;
    
    LED_ON();
    
    _delay_ms(PULSE_TIME);
}

ISR(TCB1_INT_vect)
{
    /* Clear the interrupt flag */
    TCB1.INTFLAGS |= TCB_CAPT_bm;
    
    LED_OFF();
    
    _delay_ms(PULSE_TIME);
}

static void USART0_init(void)
{
    /* Configure pin positions for the USART0 signal */
    PORTMUX.USARTROUTEA = PORTMUX_USART0_ALT3_gc;
    
    /* Configure the RX pin as input and the TX pin as output */
    PORTD.DIRCLR = PIN5_bm;
    PORTD.DIRSET = PIN4_bm;
    
    /* Configure the USART baud rate: 115200 */
    USART0.BAUD = (uint16_t)USART0_BAUD_RATE(115200);
    /* Enable the RX and the TX */
    USART0.CTRLB |= USART_TXEN_bm | USART_RXEN_bm; 
    /* Enable the Receive Interrupt */
    USART0.CTRLA |= USART_RXCIE_bm;
}

static void TCB0_init(void)
{
    /* Set a TOP value - when the timer reaches this value, an interrupt is triggered */
    /* TOP value for TCB0 is 10 -> Overflow time (us) = 1/(F_CPU/1000000) * (10 + 1) = 2.75 */
    TCB0.CCMP = TCB0_TOP_VALUE;
    /* Enable the Capture interrupt */
    TCB0.INTCTRL |= TCB_CAPT_bm;
    /* Enable the timer */
    TCB0.CTRLA |= TCB_ENABLE_bm;
}

static void TCB1_init(void)
{
    /* Set a TOP value - when the timer reaches this value, an interrupt is triggered */
    /* TOP value for TCB0 is 20 -> Overflow time (us) = 1/(F_CPU/1000000) * (20 + 1) = 5.25 */
    TCB1.CCMP = TCB1_TOP_VALUE;
    /* Enable the Capture interrupt */
    TCB1.INTCTRL |= TCB_CAPT_bm;
    /* Enable the timer */
    TCB1.CTRLA |= TCB_ENABLE_bm;
}

static void LED_init(void)
{
    /* Configure pin as output */
    PORTF.DIRSET = PIN5_bm;
    /* Turn the LED off */
    PORTF.OUTSET = PIN5_bm;
}

static void CPUINT_init(void)
{
    /* Interrupt priorities are available in the Interrupt Vector Mapping table from data sheet */  
    
    /* Enable Round-Robin feature */
    CPUINT.CTRLA |= CPUINT_LVL0RR_bm;
    /* Configure priority level 1 for USART0 */
    CPUINT.LVL1VEC = USART0_RXC_vect_num;
}

static void USART0_sendChar(char c)
{
    while (!(USART0.STATUS & USART_DREIF_bm))
    {
        ;
    }        
    USART0.TXDATAL = c;
}

int main(void) 
{
    USART0_init();
    TCB0_init();
    TCB1_init();
    LED_init();
    CPUINT_init();
    
    /* Enable the global interrupts */
    sei();
    
    while(1)
    {
        ;
    }
}
