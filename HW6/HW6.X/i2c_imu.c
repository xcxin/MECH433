#include "i2c_imu.h"
#include<xc.h>


void initI2C2(){
    //OC1-PIN 2, OC2-PIN 17
    RPA0Rbits.RPA0R = 0b0101;
    RPB8Rbits.RPB8R = 0b0101;
    
    i2c_master_setup();
    ANSELBbits.ANSB2 = 0;//turn off analog input for pin 6 (SDA2/B2/AN4)
}

void initIMU(void){
    setregister(CTRL1_XL,0b10000000); //accelerometer:1.66kHz,2g sensitivity, x filter(?)
    setregister(CTRL2_G,0b10000000); //gyrometer:1.66kHz,256 depth, x filter
    //setRegister(CTRL3_C.IF_INC=1;//enable register address increment during sequential reading,this is default
}

void I2C_read_multiple(char address, char mregister, unsigned char * data, char length){
    
    i2c_master_start(); // make the start bit

    i2c_master_send((address <<1)|0); // write the address, shifted left by 1, or'ed with a 0 to indicate writing

    i2c_master_send(mregister); // the register to read from
    i2c_master_restart();                   // send a RESTART so we can begin reading 
    i2c_master_send((address << 1) | 1); // send slave address, left shifted by 1,
                                            // and then a 1 in lsb, indicating read
    int p;
    
    for (p=0;p<length;p++){
        data[p] = i2c_master_recv(); // receive a byte from the bus
        i2c_master_ack(0);  
    }
    //i2c_master_ack(0);                      // send ACK (0): master wants another byte!
    //master_read1 = i2c_master_recv();       // receive another byte from the bus
    i2c_master_ack(1);                      // send NACK (1):  master needs no more bytes
    i2c_master_stop();                      // send STOP:  end transmission, give up bus
    
}

char I2C_read_addr(){
    
    i2c_master_start(); // make the start bit

    i2c_master_send((SLAV_ADDR <<1)|0); // write the address, shifted left by 1, or'ed with a 0 to indicate writing

    i2c_master_send(WHO_AM_I); // the register to read from
    i2c_master_restart();                   // send a RESTART so we can begin reading 
    i2c_master_send((SLAV_ADDR << 1) | 1); // send slave address, left shifted by 1,
                                            // and then a 1 in lsb, indicating read
    char read0;
    read0 = i2c_master_recv();       // receive a byte from the bus
    //i2c_master_ack(0);                      // send ACK (0): master wants another byte!
    //master_read1 = i2c_master_recv();       // receive another byte from the bus
    i2c_master_ack(1);                      // send NACK (1):  master needs no more bytes
    i2c_master_stop();                      // send STOP:  end transmission, give up bus
    
    return read0;

    
}

//set register value
void setregister(char myreg, char value)
{
    i2c_master_start();                     // Begin the start sequence
    i2c_master_send(SLAV_ADDR << 1);       // send the slave address, left shifted by 1, 
                                            // which clears bit 0, indicating a write
     
    i2c_master_send(myreg); // the register to write to
    i2c_master_send(value);         // send a byte to the slave       
    //i2c_master_send(master_write1);         // send another byte to the slave
    i2c_master_stop(); // make the stop bit
}


// I2C Master utilities, 100 kHz, using polling rather than interrupts
// The functions must be callled in the correct order as per the I2C protocol
// Change I2C1 to the I2C channel you are using
// I2C pins need pull-up resistors, 2k-10k

void i2c_master_setup(void) {
  I2C2BRG = 233;            // I2CBRG = [1/(2*Fsck) - PGD]*Pblck - 2,
                                               //PBlck=48MHz,PGD=104ns,bit rate Fsck=100kHz (Fsck can be 100kHz or 400kHz) 
                                    // look up PGD for your PIC32
  I2C2CONbits.ON = 1;               // turn on the I2C1 module
}

// Start a transmission on the I2C bus
void i2c_master_start(void) {
    I2C2CONbits.SEN = 1;            // send the start bit
    while(I2C2CONbits.SEN) { ; }    // wait for the start bit to be sent
}

void i2c_master_restart(void) {     
    I2C2CONbits.RSEN = 1;           // send a restart 
    while(I2C2CONbits.RSEN) { ; }   // wait for the restart to clear
}

void i2c_master_send(unsigned char byte) { // send a byte to slave
  I2C2TRN = byte;                   // if an address, bit 0 = 0 for write, 1 for read
  while(I2C2STATbits.TRSTAT) { ; }  // wait for the transmission to finish
  if(I2C2STATbits.ACKSTAT) {        // if this is high, slave has not acknowledged
    // ("I2C2 Master: failed to receive ACK\r\n");
  }
}

unsigned char i2c_master_recv(void) { // receive a byte from the slave
    I2C2CONbits.RCEN = 1;             // start receiving data
    while(!I2C2STATbits.RBF) { ; }    // wait to receive the data
    return I2C2RCV;                   // read and return the data
}

void i2c_master_ack(int val) {        // sends ACK = 0 (slave should send another byte)
                                      // or NACK = 1 (no more bytes requested from slave)
    I2C2CONbits.ACKDT = val;          // store ACK/NACK in ACKDT
    I2C2CONbits.ACKEN = 1;            // send ACKDT
    while(I2C2CONbits.ACKEN) { ; }    // wait for ACK/NACK to be sent
}

void i2c_master_stop(void) {          // send a STOP:
  I2C2CONbits.PEN = 1;                // comm is complete and master relinquishes bus
  while(I2C2CONbits.PEN) { ; }        // wait for STOP to complete
}