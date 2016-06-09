#include<xc.h>
#include<math.h>
#include "spi.h"
#include "i2c.h"

//#include <proc/p32mx250f128b.h>
#include<sys/attribs.h> //ISR macro

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


#define CORE_TICKS 24000 //every 1ms has 24000 ticks
int t=0;
int sinwave[200]={0};
int triwave[200]={0};

//gen_wave(sinwave,triwave);
//sinwave=gen_wave(0);
//int *triwave=gen_wave(1);
//triwave=gen_wave(1);
//unsigned char tempbyte;


//use core timer to update data per 1ms
/*void __ISR(_CORE_TIMER_VECTOR,IPL6SOFT) UPDATEDATA(void){
    IFS0bits.CTIF =0;
    int tempsin=sinwave[t%200]+127;
    int temptri=triwave[t%200]+127;
    char tempbsin=tempsin & 0xFF;
    char tempbtri=temptri & 0xFF;
    setVoltage(0,tempbsin);
    setVoltage(1,tempbtri);
    t++;
    _CP0_SET_COUNT(0);
    _CP0_SET_COMPARE(CORE_TICKS);
    
}*/

int main(void) {
    
    int j;
    for (j=0;j<200;j++){
        triwave[j]=127/200*j;
        sinwave[j]=127*sin((j%100)*2*PI/100);
    }
    
     __builtin_disable_interrupts();
    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;
    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;
    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;
    
    initSPI1();
    __builtin_enable_interrupts();
    
     _CP0_SET_COUNT(0);
    
 // char read[] = "***************************";    // buffer for reading from ram
  
  //code to initialize MCP4902
    
  /*_CP0_SET_COMPARE(CORE_TICKS);
      
    IPC0bits.CTIP =6;
    IPC0bits.CTIS =0;
    IFS0bits.CTIF =0;
    IEC0bits.CTIE =1;
    __builtin_enable_interrupts();*/
    initExpander();
    initI2C2();
    
  
    while(1){
        int t=0;
        while (t<100000){
            if (_CP0_GET_COUNT()>=24000){ 
                //update data per 1ms
                _CP0_SET_COUNT(0);
                int tempsin=sinwave[t%200]+127;
                int temptri=triwave[t%200]+127;
                char tempbsin=tempsin & 0xFF;
                char tempbtri=temptri & 0xFF;
                setVoltage(0,tempbsin);
                setVoltage(1,tempbtri);
                t++;
            }
        }
        if ( (getExpander()>>7) & 0x01 )
            // Yes, turn on the LED 
            setExpander(MCP23008_GPIO , 0x01 );
        else
            // No, turn off the LED 
            setExpander( MCP23008_GPIO , 0x00 );
    }

  
    
    
    //I2C
    /*initExpander();
    initI2C2();
    
    while(1){
        //Get the state of the button on GP7 
        //char button =  getExpander();
        // Is the button pressed? 
        if ( (getExpander()>>7) & 0x01 )
            // Yes, turn on the LED 
            setExpander(MCP23008_GPIO , 0x01 );
        else
            // No, turn off the LED 
            setExpander( MCP23008_GPIO , 0x00 );
        
    }
    
*/
    
  return 0;
}