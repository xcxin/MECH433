#include<xc.h>
#include <sys/attribs.h> 
#include "i2c_imu.h"
#include "ILI9163C.h"
#include <stdio.h>

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


int main(){
    short accel[3];
    short gyro[3];
    short temperature;
    char str[50];
    char data[50]; 
    
    __builtin_disable_interrupts();

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;
    
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
    SPI1_init();
    
    __builtin_enable_interrupts();
    
    LCD_init();
    
    //sprintf(str, "Hello world 1337!");
    //draw_string(str, 28, 32);//start at 28,32
    
    
    _CP0_SET_COUNT(0);
     //start PWM timing
     T2CONSET=0x8000;
     OC1CONSET=0x8000;
     OC2CONSET=0x8000;
    
    initIMU();
    //char dev_addr=I2C_read_addr();
    
    //LCD_clearScreen(0x0000);
    //sprintf(str,"slave address = %c",dev_addr);
    //draw_string(str,0,0);
    //printf("%c",dev_addr);//check if the address is 0b01101011
    
    while(1){
        if (_CP0_GET_COUNT()>=4800000){ 
            //read from the chip at 50Hz
            _CP0_SET_COUNT(0);
            
            I2C_read_multiple(SLAV_ADDR, OUT_TEMP_L, (unsigned char *) data, 14);
            accel[0] = (short)(((unsigned char)data[9]) << 8 |((unsigned char)data[8]));//typecast unsigned char data to signed short,by shifting the high byte and ORing it with the low byte
            accel[1] = (short)(((unsigned char)data[11]) << 8 |((unsigned char)data[10]));
            accel[2] = (short)(((unsigned char)data[13]) << 8 |((unsigned char)data[12]));
            gyro[0] = (short)(((unsigned char)data[3]) << 8 |((unsigned char)data[2]));//typecast unsigned char data to signed short,by shifting the high byte and ORing it with the low byte
            gyro[1] = (short)(((unsigned char)data[5]) << 8 |((unsigned char)data[4]));
            gyro[2] = (short)(((unsigned char)data[7]) << 8 |((unsigned char)data[6]));
            temperature = (short)(((unsigned char)data[1]) << 8 |((unsigned char)data[0]));
            
            //sprintf(accelx,"accelx = %1.3f",(float)accel[0]/32768*2);//32768=2^15, the range of 16-bit by 2's complement
           //printf("acclex is %f", accel[0]);
           //printf("accley is %f", accel[1]);
           
           LCD_clearScreen(0x0000);
           sprintf(str,"accelx = %1.3f",(float)accel[0]);
           draw_string(str,0,0);
           sprintf(str,"accely = %1.3f",(float)accel[1]);
           draw_string(str,0,8);
           sprintf(str,"accelz = %1.3f",(float)accel[2]);
           draw_string(str,0,16);
           sprintf(str,"gyrox = %1.3f",(float)gyro[0]);
           draw_string(str,0,24);
           sprintf(str,"gyroy = %1.3f",(float)gyro[1]);
           draw_string(str,0,32);
           sprintf(str,"gyroz = %1.3f",(float)gyro[2]);
           draw_string(str,0,40);
           sprintf(str,"temperature = %1.3f",(float)temperature);
           draw_string(str,0,48);
               
    
        }
        
        //To do: OC1RS=
    }
     //while(1){;}
}
