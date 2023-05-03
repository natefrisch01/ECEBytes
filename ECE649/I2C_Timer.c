/*
This code successfully writes to the timer with address 0x68, first sending the register it would like to read from (0x03 for seconds).
Then, it reads from the device, though for an unknown reason the timer is not sending the right data.
*/

#include <msp430.h>

char data_in = 0;

/**
 * main.c
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;   // unlock the ports

    UCB0CTLW0 |= UCSWRST | UCSSEL__SMCLK; // software reset and slect clock source: SMCLK
    UCB0BRW |= 10;  // select prescaler to 10
    UCB0CTLW0 |= UCMODE_3;  // setting it to I2C mode
    UCB0CTLW0 |= UCMST; // setting MSP430 to master mode

    UCB0I2CSA = 0x68;  // set the RTC address
    UCB0CTLW1 |= UCASTP_2; // auto stop enable
    UCB0TBCNT = 1; // 1 byte tx mode


    UCB0CTLW0 &= ~UCSWRST; // clear the software reset

    // set up ports
        P1SEL1 &= ~BIT7;
        P1SEL0 |= BIT7;     // SCL
        P1SEL1 &= ~BIT6;
        P1SEL0 |= BIT6;     // SDA

     UCB0IE |= UCTXIE0; // local interrupt for Tx mode
     UCB0IE |= UCRXIE0; // local interrupt for Rx mode
     UCB0IE |= UCSTPIE; // stop condition interrupt enable
     __enable_interrupt(); // global interrupts


    while (1){
        while (UCB0CTLW0 & UCTXSTP);            // Ensure stop condition got sent
//        UCB0CTLW0 |=UCTR; // starting of the TX mode

        UCB0CTLW0 |= UCTR | UCTXSTT;            // I2C TX, start condition

        while(UCSTPIFG ==0) {}  // optional for you

        UCB0IFG &= ~UCSTPIFG;

        while (UCB0CTLW0 & UCTXSTP);            // Ensure stop condition got sent

        UCB0CTLW0 &= ~UCTR; // starting of the RX mode
        UCB0CTLW0 |= UCTXSTT;    // manually start a message
//        while(UCSTPIFG ==0) {}  // optional for you
        while(UCSTPIFG ==0) {}  // optional for you

        UCB0IFG &= ~UCSTPIFG;


    }


    return 0;
}

#pragma vector = USCI_B0_VECTOR  //USCI_B0_VECTOR
__interrupt void i2c(){

    switch(UCB0IV) {
    case 0x08:
        UCB0IFG &= ~UCSTPIFG;
        break;

    case 0x16:       // rx buffer register
        data_in = UCB0RXBUF; //do something
        break;


    case 0x18:      // tx buffer register    // USCI_I2C_UCTXIFG0
        UCB0TXBUF = 0x03; //write register address
        break;

    default:
        break;

    }

}
