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
#include <stdbool.h>
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

void createStartCondition()
{
    SSP1CON2bits.SEN = 1;
    while(SSPCON2bits.SEN){}
    return;
}

void createStopCondition()
{
    SSP1CON2bits.PEN = 1;
    while(SSP1CON2bits.PEN){}
    return;
}

void send8bitData(unsigned char data)
{
    SSP1IF = 0;
    SSP1BUF = data;
    while(SSP1IF == 0){}
    SSP1IF = 0;
    return;
}

void waitReceivedACK()
{
    // 0:received 1:not_recieved
    while (SSP1CON2bits.ACKSTAT == 1){}
    return; 
}

void write1602AFormatByte(unsigned char data)
{
    send8bitData(data);
    waitReceivedACK();
    send8bitData(data | 0x04);
    waitReceivedACK();
    send8bitData(data & 0xFB);
    waitReceivedACK();
}

void LCD1602AProtocol(unsigned char type,unsigned char data)
{
    //start condition
    createStartCondition();
    
    //address
    send8bitData(0x27<<1);
    waitReceivedACK();
    
    //4bit upper data
    write1602AFormatByte(type | (data&0xF0) | 0x08);
    //4bit lower data
    write1602AFormatByte(type | ((data<<4)&0xF0) | 0x08);
    
    //stop condition
    createStopCondition();

    return;
}

void lcdInit()
{
    LCD1602AProtocol(command,0x33);
    __delay_ms(5);
    LCD1602AProtocol(command,0x32);
    __delay_ms(1);
    LCD1602AProtocol(command,0x06);
    __delay_ms(1);
    LCD1602AProtocol(command,0x0C);
    __delay_ms(1);
    LCD1602AProtocol(command,0x28);
    __delay_ms(1);
    LCD1602AProtocol(command,0x01);
    __delay_ms(1);
}

void showMessage(const char* message,unsigned char line_address)
{
    LCD1602AProtocol(command,line_address);
    while(*message)
    {
        LCD1602AProtocol(data,*message++);
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
    while(true)
    {
        showMessage("Hello",first);
        showMessage("World",second);
        __delay_ms(10000);
        lcdInit();
        showMessage("GoodMorning",first);
        showMessage("Japan",second);
        __delay_ms(10000);
        lcdInit();
    }
}