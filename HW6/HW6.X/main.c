#include<xc.h>
#include "i2c_imu.h"
#include <sys/attribs.h> 
#include <stdio.h>
//#include "PIC32_reg_init.h"

//DEVCFG registers initialization
// DEVCFG0
#pragma config DEBUG = OFF // no debugging
#pragma config JTAGEN = OFF // no jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // no write protect
#pragma config BWP = OFF // no boot write protect
#pragma config CP = OFF // no code protect

// DEVCFG1
#pragma config FNOSC = PRIPLL // use primary oscillator with pll
#pragma config FSOSCEN = OFF // turn off secondary oscillator
#pragma config IESO = OFF // no switching clocks
#pragma config POSCMOD = HS // high speed crystal mode
#pragma config OSCIOFNC = OFF // free up secondary osc pins
#pragma config FPBDIV = DIV_1 // divide CPU freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // do not enable clock switch
#pragma config WDTPS = PS1048576 // slowest wdt
#pragma config WINDIS = OFF // no wdt window
#pragma config FWDTEN = OFF // wdt off by default
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the CPU(and peripheral bus) clock to 48MHz,USB communication works strictly under 48MHz
//CPU=INPUT CLOCK(CRYSTAL,8MHZ)/FPLLIDIV*FPLLMUL/FPLLODIV
//USB MULTIPLIES INPUT BY 12, SO WE NEED 4MHZ, WHICH IS INPUT CLOCK(8MHZ)/UPLLIDIV
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz
#pragma config UPLLIDIV = DIV_2 // divider for the 8MHz input clock, then multiply by 12 to get 48MHz for USB
#pragma config UPLLEN = ON // USB clock on

// DEVCFG3
#pragma config USERID = 0 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations
#pragma config FUSBIDIO = ON // USB pins controlled by USB module
#pragma config FVBUSONIO = ON // USB VBUSON controlled by USB module



int main(void){
    
    short accel[3];
    short gyro[3];
    short temperature;
    char str[50];
    char data[13];
    
    __builtin_disable_interrupts();

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;
    
    //define pins here, OC pins defined in initI2C2
    
    
    //use OC1,OC2 to generate PWM
    T2CONbits.TCKPS=4; //prescalar=16
    PR2=2999; //period=(PR2+1)*prescalar*1000/48=1000us
    TMR2=0;
    OC1CONbits.OCM=0b110;//PWM mode without fault pin
    OC1RS=1500; //COUNT=3000,so 50% DC is 1500
    OC1R=1500;
    T2CONbits.ON=1;
    OC1CONbits.ON=1;
    OC2CONbits.OCM=0b110;//PWM mode without fault pin
    OC2RS=1500; //COUNT=3000,so 50% DC is 1500
    OC2R=1500;
    OC2CONbits.ON=1;
    
    initI2C2();
     __builtin_enable_interrupts();
    
     _CP0_SET_COUNT(0);
     //start PWM timing
     T2CONSET=0x8000;
     OC1CONSET=0x8000;
     OC2CONSET=0x8000;
    
    initIMU();
    //char dev_addr=I2C_read_addr();
    //printf("%c",dev_addr);//check if the address is 0b01101011
    
    while(1){
        if (_CP0_GET_COUNT>=4800000){ 
            //read from the chip at 50Hz
            _CP0_SET_COUNT(0);
            
            I2C_read_multiple(SLAV_ADDR, OUT_TEMP_L, (unsigned char *) data, 14);
            accel[0] = (short)(((unsigned char)data[9]) << 8 |((unsigned char)data[8]));//typecast unsigned char data to signed short,by shifting the high byte and ORing it with the low byte
            accel[1] = (short)(((unsigned char)data[11]) << 8 |((unsigned char)data[10]));
            //sprintf(accelx,"accelx = %1.3f",(float)accel[0]/32768*2);//32768=2^15, the range of 16-bit by 2's complement
           
          // sprintf(str,"accely = %1.3f",(float)data[5]/32768*2);      
    
        }
        
        //To do: OC1RS=
    }
    return 0;
}