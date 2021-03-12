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
    ten = 0x41,
    eleven = 0x42,
    twelve = 0x43,
    thirteen = 0x44,
    fourteen = 0x45,
    fifteen = 0x46
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


void SHT31Protocol(unsigned short *temp_humi)
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

void convertNumber(unsigned char number,unsigned char *result,unsigned int start_index)
{
    result[start_index] = zero;
    result[start_index+1] = zero;
    switch (number>>4)
    {
        case 0x0: result[start_index]=zero; break;
        case 0x1: result[start_index]=one; break;
        case 0x2: result[start_index]=two; break;
        case 0x3: result[start_index]=three; break;
        case 0x4: result[start_index]=four; break;
        case 0x5: result[start_index]=five; break;
        case 0x6: result[start_index]=six; break;
        case 0x7: result[start_index]=seven; break;
        case 0x8: result[start_index]=eight; break;
        case 0x9: result[start_index]=nine; break;
        case 0xA: result[start_index]=ten; break;
        case 0xB: result[start_index]=eleven; break;
        case 0xC: result[start_index]=twelve; break;
        case 0xD: result[start_index]=thirteen; break;
        case 0xE: result[start_index]=fourteen; break;
        case 0xF: result[start_index]=fifteen; break;
    }

    switch (number&0x0F)
    {
        case 0x0: result[start_index+1]=zero; break;
        case 0x1: result[start_index+1]=one; break;
        case 0x2: result[start_index+1]=two; break;
        case 0x3: result[start_index+1]=three; break;
        case 0x4: result[start_index+1]=four; break;
        case 0x5: result[start_index+1]=five; break;
        case 0x6: result[start_index+1]=six; break;
        case 0x7: result[start_index+1]=seven; break;
        case 0x8: result[start_index+1]=eight; break;
        case 0x9: result[start_index+1]=nine; break;
        case 0xA: result[start_index+1]=ten; break;
        case 0xB: result[start_index+1]=eleven; break;
        case 0xC: result[start_index+1]=twelve; break;
        case 0xD: result[start_index+1]=thirteen; break;
        case 0xE: result[start_index+1]=fourteen; break;
        case 0xF: result[start_index+1]=fifteen; break;
    }
    return;
}



int main(int argc, char** argv) 
{
    OSCCON = 0b01110010; // CLOCK : 8MHz
    ANSELA = 0x00;
    ANSELB = 0x00;
    TRISA  = 0x00;
    TRISB  = 0x12;// I2C1 SCL:RB4 SDA:RB1
    PORTA = 0x00;
    PORTB = 0x00;
    
    SSP1STAT = 0x80;
    SSP1CON1 = 0x28;
    SSP1CON3 = 0x00;
    SSP1ADD = 0x13; //1<10kHz> = ((byte+1)*4)/80<8MHz>
    
    __delay_ms(1000);
    lcdInit();

    unsigned short temp_humi[2] = {};
    char temp = 0x00;
    unsigned char humi = 0x00;
    
    // Display exsample → |2|0|'|C| |5|0|%| 
    unsigned char display_char[8] = {};
    display_char[2] = 0xDF;//'
    display_char[3] = 0x43;//C
    display_char[4] = 0x20;//_(space)
    display_char[7] = 0x25;//%

    while(1)
    {
        SHT31Protocol(temp_humi);
        
        temp = -45 + (char)calcDiv(175,temp_humi[0]);
        convertNumber(temp,display_char,0);        
        
        humi = (unsigned)calcDiv(100,temp_humi[1]);
        convertNumber(humi,display_char,5);

        showMessage(display_char,first,7);
        
        __delay_ms(10000);
        lcdInit();
    }
}