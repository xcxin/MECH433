#include<xc.h>
#include "spi.h"
//#include "PIC32_reg_init.h"
//#include <proc/p32mx250f128b.h>
#include<sys/attribs.h> //ISR macro

//the library is for communication with MCP4902 DAC circuit,Use SPI1 to talk to the DAC 
//uses the DAC to output a 10Hz sine wave on VoutA and a 5Hz triangle wave on VoutB, updating the values 1000 times a second 

void initSPI1(void)
{
    SDI1Rbits.SDI1R = 0b0000; // A1 is SDI1
    RPB13Rbits.RPB13R = 0b0011; // B13 is SDO1
    RPB7Rbits.RPB7R=0b0011;// B7 is SS1
    
    TRISBbits.TRISB7 = 0; // set SS1 to output
    LATBbits.LATB7 = 1; //set SS1 initially high
    
    SPI1CON=0; //turn off SPI1 for resetting
    SPI1BUF; //clear rx buffer by reading from it
    SPI1BRG=0x1; //baud rate to 12MHz,[SPI1BRG = (48000000/(2*desired))-1],SPI1BRG=1;
    SPI1CONbits.CKE = 1; // data changes when clock goes from hi to lo (since CKP is 0)
    SPI1STATbits.SPIROV =0; //clear the overflow bit
    SPI1CONbits.MODE16=1; //use 16-bit mode
    SPI1CONbits.MODE32=0; //use 16-bit mode
    SPI1CONbits.MSTEN=1; //master operation
    SPI1CONbits.ON=1; //turn on SPI1
    
    CS=1;// SS starts high
}


//send a byte via SPI and read the response
unsigned char SPI1_IO(unsigned char write)
{
    SPI1BUF=write;
    while (!SPI1STATbits.SPIRBF)// wait to receive the byte
    {;}
    return SPI1BUF;
}



// write a byte to the DAC
//channel is 0 or 1 (for VoutA and VoutB), and voltage is the 8-bit output level
void setVoltage(int channel,  unsigned char data) {
  char left=0b00000000;
  char right=0b00000000;
  
  if (channel==0){
    left=((data>>4)|0b00110000); //channel 0,unbuffered,gain=1,VOUT activated
  
  }
  else if (channel==1){
      left=((data>>4)|0b10110000);
  }
right=data<<4;
CS=0; // enable the ram by lowering the chip select line
SPI1_IO(left);
SPI1_IO(right);
CS=1;
}

//generate waves
/*void gen_wave(int *sinwave,int *triwave){
     int j;
     //int *sinwave[];
     //int *triwave[];

    for (j=0;j<200;j++){
    triwave[j]=127/200*j;
    sinwave[j]=127*sin((j%100)*2*PI/100);
    }
    /* if (channel==0){
    return sinwave;}
     else if (channel==1){
         return triwave;
}
}*/





/*// read len bytes from ram, starting at the address addr
void ram_read(unsigned short addr, char data[], int len) {
  int i = 0;
  CS = 0;
  spi_io(0x3);                   // ram read operation
  spi_io((addr & 0xFF00) >> 8);  // most significant address byte
  spi_io(addr & 0x00FF);         // least significant address byte
  for(i = 0; i < len; ++i) {
    data[i] = spi_io(0);         // read in the data
  }
  CS = 1;
}

int main(void) {
  unsigned short addr1 = 0x1234;                  // the address for writing the ram
  char data[] = "Help, I'm stuck in the RAM!";    // the test message
  char read[] = "***************************";    // buffer for reading from ram
  char buf[100];                                  // buffer for comm. with the user
  unsigned char status;                           // used to verify we set the status 
  NU32_Startup();   // cache on, interrupts on, LED/button init, UART init
  ram_init(); 

  // check the ram status
  CS = 0;
  spi_io(0x5);                                      // ram read status command
  status = spi_io(0);                               // the actual status
  CS = 1;

  sprintf(buf, "Status 0x%x\r\n",status);
  NU32_WriteUART3(buf);

  sprintf(buf,"Writing \"%s\" to ram at address 0x%x\r\n", data, addr1);
  NU32_WriteUART3(buf);
                                                    // write the data to the ram
  ram_write(addr1, data, strlen(data) + 1);         // +1, to send the '\0' character
  ram_read(addr1, read, strlen(data) + 1);          // read the data back
  sprintf(buf,"Read \"%s\" from ram at address 0x%x\r\n", read, addr1);
  NU32_WriteUART3(buf);

  while(1) {
    ;
  }
  return 0;
}*/