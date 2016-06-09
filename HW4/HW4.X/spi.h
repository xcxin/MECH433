//SPI communication between PIC32 and 

#ifndef SPI_H__
#define SPI_H__



#define PI 3.141592653589793238462643383

#define CS LATBbits.LATB7       // chip select pin

//functions
void initSPI1(void);
unsigned char SPI1_IO(unsigned char);
void setVoltage(int,unsigned char); 
//void gen_wave(int*,int*);

#endif
