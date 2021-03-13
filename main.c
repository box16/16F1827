/* 
 * File:   main.c
 * Author: BOX16
 *
 * Created on 2021/03/10, 10:53
 */

// PIC16F1827 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTOSC    // Oscillator Selection (INTOSC oscillator: I/O function on CLKIN pin)
#pragma config WDTE = OFF       // Watchdog Timer Enable (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable (PWRT enabled)
#pragma config MCLRE = OFF      // MCLR Pin Function Select (MCLR/VPP pin function is digital input)
#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Memory Code Protection (Data memory code protection is disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable (Brown-out Reset enabled)
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config IESO = OFF       // Internal/External Switchover (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is disabled)

// CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
#pragma config PLLEN = OFF      // PLL Enable (4x PLL disabled)
#pragma config STVREN = OFF     // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will not cause a Reset)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config LVP = OFF        // Low-Voltage Programming Enable (High-voltage on MCLR/VPP must be used for programming)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#define _XTAL_FREQ  8000000

enum LINE 
{
    first = 0x80,
    second = 0xC0
};

enum TYPE
{
    data = 0x01,
    command = 0x00
};

enum R_W
{
    read = 0x01,
    write = 0x00
};

enum number
{
    zero = 0x30,
    one = 0x31,
    two = 0x32,
    three = 0x33,
    four = 0x34,
    five = 0x35,
    six = 0x36,
    seven = 0x37,
    eight = 0x38,
    nine = 0x39,
};

//I2C Protocol
//{
    void sendStartCondition()
    {
        // When SEN is set to 1, the start condition generation starts.
        // Automatically becomes 0 when transmission is completed.
        SSP1CON2bits.SEN = 1;
        while(SSPCON2bits.SEN == 1){}
        return;
    }

    void sendStopCondition()
    {
        // When PEN is set to 1, the stop condition generation starts.
        // Automatically becomes 0 when transmission is completed.
        SSP1CON2bits.PEN = 1;
        while(SSP1CON2bits.PEN == 1){}
        return;
    }

    void send8bitData(const unsigned char data)
    {
        // If you put data in SSP1BUF, it will be sent automatically.
        // When the transmission is completed, SSP1IF becomes 1.
        SSP1IF = 0;
        SSP1BUF = data;
        while(SSP1IF == 0){}
        SSP1IF = 0;
        return;
    }

    unsigned char receive8bitData()
    {
        // Data reception starts when RCEN is set to 1.
        // RCEN becomes 0 when data reception is completed.
        // Received data is saved in SSP1BUF.
        SSP1CON2bits.RCEN = 1;
        while(SSP1CON2bits.RCEN == 1){}

        return SSP1BUF;
    }

    void waitSendACK()
    {
        // When ACKDT is set to 0, the generation of ACK signal is set.
        // By setting ACKEN to 1, the signal is transmitted according to the ACKDT setting.
        // ACKEN automatically becomes 0 when the signal transmission is completed.
        SSP1CON2bits.ACKDT = 0;
        SSP1CON2bits.ACKEN = 1;
        while(SSP1CON2bits.ACKEN == 1){}
        return;
    }

    void waitSendNACK()
    {
        // When ACKDT is set to 1, the generation of NACK signal is set.
        // By setting ACKEN to 1, the signal is transmitted according to the ACKDT setting.
        // ACKEN automatically becomes 0 when the signal transmission is completed.
        SSP1CON2bits.ACKDT = 1;
        SSP1CON2bits.ACKEN = 1;
        while(SSP1CON2bits.ACKEN == 1){}
        return;
    }

    void waitReceivedACK()
    {
        // ACKSTAT contains whether the ACK signal from the receiving side has been received.
        // Received at 0. 
        // Not received at 1.
        while (SSP1CON2bits.ACKSTAT == 1){}
        return; 
    }
//} I2C Protocol

// 1602A {
    void i2c1602A(const unsigned char type,
                  const unsigned char data)
    {
        sendStartCondition();
        
        //address
        send8bitData(0x27<<1 | write);
        waitReceivedACK();
        
        unsigned char latch = 0x00;

        //4bit upper data
        latch = type | (data&0xF0) | 0x08;
        send8bitData(latch);
        waitReceivedACK();
        send8bitData(latch | 0x04);
        waitReceivedACK();
        send8bitData(latch & 0xFB);
        waitReceivedACK();
        
        latch = type | ((data<<4)&0xF0) | 0x08;
        //4bit lower data
        send8bitData(latch);
        waitReceivedACK();
        send8bitData(latch | 0x04);
        waitReceivedACK();
        send8bitData(latch & 0xFB);
        waitReceivedACK();
        
        sendStopCondition();

        return;
    }

    void lcdInit()
    {
        i2c1602A(command,0x33);
        __delay_ms(5);
        i2c1602A(command,0x32);
        __delay_ms(1);
        i2c1602A(command,0x06);
        __delay_ms(1);
        i2c1602A(command,0x0C);
        __delay_ms(1);
        i2c1602A(command,0x28);
        __delay_ms(1);
        i2c1602A(command,0x01);
        __delay_ms(1);
    }

    void showMessage(const unsigned char* message,
                     const unsigned char line_address,
                     const unsigned char last_index)
    {
        i2c1602A(command,line_address);
        unsigned char i=0;
        for (unsigned char i=0; i < last_index+1; i++)
        {
            i2c1602A(data,message[i]);
        }
    }
//} 1602A

//SHT31{
    void i2cSHT31(unsigned short *temp_humi)
    {
        sendStartCondition();

        //address
        send8bitData(0x45<<1 | write);
        waitReceivedACK();

        //Clock stretch
        //On : 0x2C 
        //Off : 0x24
        send8bitData(0x2C);
        waitReceivedACK();

        //Repeat accuracy
        //6 pattern
        send8bitData(0x06);
        waitReceivedACK();

        sendStopCondition();

        sendStartCondition();
        
        // address
        send8bitData(0x45<<1 | read);
        waitReceivedACK();

        //Receives 16 bits of temperature data.
        temp_humi[0] = 0;
        temp_humi[0] |= receive8bitData() << 8;
        waitSendACK();
        temp_humi[0] |= receive8bitData();
        waitSendACK();

        //receive check sum
        receive8bitData();
        waitSendACK();

        //Receives 16 bits of humidity data.
        temp_humi[1] = 0;
        temp_humi[1] |= receive8bitData() << 8;
        waitSendACK();
        temp_humi[1] |= receive8bitData();
        waitSendACK();

        //receive check sum
        receive8bitData();
        waitSendNACK();

        sendStopCondition();
        return;
    }

    unsigned short calcDiv(unsigned short coef,unsigned short row_data)
    {
        // Calculate -> coef*row_data/(2^16-1)
        unsigned short result = 0;

        for (unsigned char i=0; i<16; i++)
        {
            coef = coef >> 1;
            if((row_data<<i) & 0x8000) result += coef;
        }

        return result;
    }
//}SHT31

unsigned char convertNumberto1602ACode(const unsigned char number)
{
    switch (number)
    {
        case 0x0: return zero;
        case 0x1: return one;
        case 0x2: return two;
        case 0x3: return three;
        case 0x4: return four;
        case 0x5: return five;
        case 0x6: return six;
        case 0x7: return seven;
        case 0x8: return eight;
        case 0x9: return nine;
        default: return 0xFF;
    }
} 

void convert2DigitNumber(unsigned char number,
                         unsigned char *result,
                         const unsigned int start_index)
{
    unsigned char carry_10 = 0x00;
    while(number >= 10)
    {
        number -= 10;
        carry_10++;
    }

    result[start_index] = convertNumberto1602ACode(carry_10);
    result[start_index+1] = convertNumberto1602ACode(number);
    return;
}

void convert4DigitNumber(unsigned short number,
                         unsigned char *result,
                         const unsigned int start_index)
{
    unsigned char carry_1000 = 0x00;
    unsigned char carry_100 = 0x00;
    unsigned char carry_10 = 0x00;
    while(number >= 1000)
    {
        number -= 1000;
        carry_1000++;
    }
    while(number >= 100)
    {
        number -= 100;
        carry_100++;
    }
    while(number >= 10)
    {
        number -= 10;
        carry_10++;
    }

    result[start_index] = convertNumberto1602ACode(carry_1000);
    result[start_index+1] = convertNumberto1602ACode(carry_100);
    result[start_index+2] = convertNumberto1602ACode(carry_10);
    result[start_index+3] = convertNumberto1602ACode(number);
    return;
}

unsigned short eusartMHZ19C()
{   
    // send
    TXIF = 0;
    while (TXIF==0);
    TXREG = 0xFF;
    while (TXIF==0);
    TXREG = 0x01;
    while (TXIF==0);
    TXREG = 0x86;
    while (TXIF==0);
    TXREG = 0x00;
    while (TXIF==0);
    TXREG = 0x00;
    while (TXIF==0);
    TXREG = 0x00;
    while (TXIF==0);
    TXREG = 0x00;
    while (TXIF==0);
    TXREG = 0x00;
    while (TXIF==0);
    TXREG = 0x79;

    unsigned short co2 = 0x0000;
    // send
    RCIF = 0;
    while (RCIF==0);
    // 0xFF
    while (RCIF==0);
    // 0x86
    while (RCIF==0);
    co2 = RCREG;
    co2 = co2 << 8;
    while (RCIF==0);
    co2 |= RCREG;
    while (RCIF==0);
    // -
    while (RCIF==0);
    // -
    while (RCIF==0);
    // -
    while (RCIF==0);
    // -
    while (RCIF==0);
    // checksum

    return co2;
}

int main(int argc, char** argv) 
{
    OSCCON = 0b01110010; // CLOCK : 8MHz
    ANSELA = 0x00;
    ANSELB = 0x04;
    TRISA  = 0x00;
    TRISB  = 0x16;// I2C1 SCL:RB4 SDA:RB1 EUSART RX:RB2
    PORTA = 0x00;
    PORTB = 0x00;
    
    SSP1STAT = 0x80;
    SSP1CON1 = 0x28;
    SSP1CON3 = 0x00;
    SSP1ADD = 0x13; //1<10kHz> = ((byte+1)*4)/80<8MHz>

    RCIE = 1;
    PEIE = 1;
    GIE = 1;

    TXCKSEL = 1;
    RXDTSEL = 1;
    TXSTA = 0x24;
    RCSTA = 0x90;
    BRG16 = 0;
    SPBRG = 51;// baud rate 9600

    
    __delay_ms(1000);
    lcdInit();

    unsigned short temp_humi[2] = {};
    unsigned short co2 = 0x0000;
    char temp = 0x00;
    unsigned char humi = 0x00;
    
    // Display example -> |2|0|'|C| |5|0|%| 
    unsigned char display_char_first[8] = {};
    display_char_first[2] = 0xDF;//'
    display_char_first[3] = 0x43;//C
    display_char_first[4] = 0x20;//_(space)
    display_char_first[7] = 0x25;//%

    // Display example -> |1|5|0|0|p|p|m|
    unsigned char display_char_second[7] = {};
    display_char_second[4] = 0x70;//p
    display_char_second[5] = 0x70;//p
    display_char_second[6] = 0x6D;//m
    
    while(1)
    {
        i2cSHT31(temp_humi);
        co2 = eusartMHZ19C();

        temp = -45 + (char)calcDiv(175,temp_humi[0]);
        convert2DigitNumber(temp,display_char_first,0);        
        
        humi = (unsigned char)calcDiv(100,temp_humi[1]);
        convert2DigitNumber(humi,display_char_first,5);

        convert4DigitNumber(co2,display_char_second,0);

        showMessage(display_char_first,first,7);
        showMessage(display_char_second,second,6);
        
        __delay_ms(10000);
        lcdInit();
        __delay_ms(1000);
    }
}