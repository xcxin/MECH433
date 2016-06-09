//a blinking LED, at 1kHz, that stops when you push and hold the user pushbutton, and continues when you release the pushbutton.


#include<xc.h>
#include<sys/attribs.h> //ISR macro
#define CORE_TICKS 12000 //every 1ms has 24000 ticks

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
/*
void __ISR(_CORE_TIMER_VECTOR,IPL6SOFT) toggleISR(void){
    IFS0bits.CTIF =0;
    LATAbits.LATA4=!LATAbits.LATA4; //toggle LED on A4
    _CP0_SET_COUNT(0);
    _CP0_SET_COMPARE(CORE_TICKS);
    
}
*/

int main()
{
   /* TRISAbits.TRISA4=0;   //initialize the LED pin as an output that is on
    TRISBbits.TRISB4=1;   //initialize the pushbutton pin as an input pin
    LATAbits.LATA4=1;*/
    
    __builtin_disable_interrupts();
    //__builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);
    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;
    
    // do your TRIS and LAT commands here
    TRISAbits.TRISA4=0;   //initialize the LED pin as an output that is on
    
    TRISBbits.TRISB4=1;   //initialize the pushbutton pin as an input pin
    LATAbits.LATA4=1;
    //PORTBbits.RB4= 1; 
   /* _CP0_SET_COMPARE(CORE_TICKS);
      
    IPC0bits.CTIP =6;
    IPC0bits.CTIS =0;
    IFS0bits.CTIF =0;
    IEC0bits.CTIE =1;*/
    __builtin_enable_interrupts();
    
    //_CP0_SET_COUNT(0);
    //push the user pushbutton->LED stops blinking
   
    while(1){
        _CP0_SET_COUNT(0);
        LATAbits.LATA4=1;
        while (_CP0_GET_COUNT() < CORE_TICKS){
            while(!PORTBbits.RB4)
            {
                ;
            }
            //LATAbits.LATA4=0;
        }
        LATAbits.LATA4=0;
        while (_CP0_GET_COUNT() < CORE_TICKS*2){
            ;
        }
    }
    /*
    //use core timer to toggle LED at 1kHz
    while(1) {
	    // use _CP0_SET_COUNT(0) and _CP0_GET_COUNT() to test the PIC timing
		// remember the core timer runs at half the CPU speed
        
        //read pushbutton, if it's pushed, wait until it's unpushed before continuing
        
        
        //set core timer to zero
        _CPO_SET_COUNT(0);
        //turn on LED
        LATAINV=0x2;
        //wait for 0.5ms
        //CPO_SET_COMPARE(CORE_TICKS);
        _CPO_SET_COUNT(0);
        while (_CPO_GET_COUNT()<0.5*CORE_TICKS)
        {;}
        //turn off LED
        LATAINV=0x2;
        //wait for 0.5ms
        
    }*/
    
    return 0;
}