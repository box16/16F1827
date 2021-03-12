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

    void createStopCondition()
    {
        // When PEN is set to 1, the stop condition generation starts.
        // Automatically becomes 0 when transmission is completed.
        SSP1CON2bits.PEN = 1;
        while(SSP1CON2bits.PEN == 1){}
        return;
    }

    void send8bitData(unsigned char data)
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

void SHT31Protocol(unsigned short *row_data)//16bit 2data in.
{
    //start condition
    sendStartCondition();

    //address
    send8bitData(0x45<<1 | write);
    waitReceivedACK();

    //command
    send8bitData(0x2C);
    waitReceivedACK();

    //command
    send8bitData(0x06);
    waitReceivedACK();

    //stop condition
    createStopCondition();

    //start condition
    sendStartCondition();
    
    // address
    send8bitData(0x45<<1 | read);
    waitReceivedACK();

    //receive temp 16bit
    row_data[0] = 0;
    row_data[0] |= receive8bitData();
    row_data[0] = row_data[0]<<8;
    waitSendACK();
    row_data[0] |= receive8bitData();
    waitSendACK();

    //receive check sum
    receive8bitData();
    waitSendACK();

    //receive humi 16bit
    row_data[1] = 0;
    row_data[1] |= receive8bitData();
    row_data[1] = row_data[1]<<8;
    waitSendACK();
    row_data[1] |= receive8bitData();
    waitSendACK();

    //receive check sum
    receive8bitData();
    waitSendNACK();

    //stop condition
    createStopCondition();
    return;
}

unsigned short calcDiv(unsigned short coef,unsigned short row_data)
{
    unsigned short current = coef;
    unsigned short i=0;
    unsigned short result = 0;
    while(i<16)
    {
        current = current >> 1;
        if((row_data<<i) & 0x8000)
        {
            result += current;
        }
        i++;
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

void i2c1602A(unsigned char type,unsigned char data)
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
    
    createStopCondition();

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

void showMessage(const char* message,unsigned char line_address,int last_index)
{
    i2c1602A(command,line_address);
    int i=0;
    while(i < last_index+1)
    {
        i2c1602A(data,message[i]);
        i++;
    }
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
    unsigned char temp = 0x00;
    unsigned char humi = 0x00;
    // 16'C 22%
    unsigned char status[8] = {};
    status[2] = 0xDF;
    status[3] = 0x43;
    status[4] = 0x20;
    status[7] = 0x25;

    while(1)
    {
        SHT31Protocol(temp_humi);
        
        temp = -45 + calcDiv(175,temp_humi[0]);
        convertNumber(temp,status,0);        
        humi = calcDiv(100,temp_humi[1]);
        convertNumber(humi,status,5);

        showMessage(status,first,7);
        
        __delay_ms(10000);
        lcdInit();
    }
}