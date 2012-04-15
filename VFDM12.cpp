#include <avr/pgmspace.h>

#include "vfdascii.h"
#include "VFDM12.H"

VFDM12::VFDM12()
{
}

void VFDM12::init(int adr)
{
	vfdaddr=adr;
	Wire.beginTransmission(vfdaddr);
  //Wire.send(0x00);
	Wire.send(0x9f);
	int result = Wire.endTransmission();
	clear();}

void VFDM12::init()
{
	init(0xa0 >> 1);
}


void VFDM12::send_segments(char adress, char c, char d, char e)
{
    Wire.beginTransmission(vfdaddr);
    Wire.send(0xc0+adress);
    Wire.send(c);
    Wire.send(d);
    Wire.send(e);
    int result = Wire.endTransmission();
    
}


void VFDM12::send_led(char c)
{
    Wire.beginTransmission(vfdaddr);
    Wire.send(0x41);
    Wire.send(c);
    int result = Wire.endTransmission();
}

void VFDM12::clear()
{
    for (int i=0; i < 36; i+=3)
        send_segments(i, 0,0,0);
}

void VFDM12::send_char(char c)
{
    if (curadr <0)
    {
        curadr = 33;
    }
    if (c >=0)
    {
        int o=c*2;
//        vfd_send_segments(vfd_curadr, vfdchars[o], vfdchars[o+1], 0);
        send_segments(curadr, pgm_read_byte(&vfdchars[o]), pgm_read_byte(&vfdchars[o+1]), 0);
        curadr -= 0x03;
    }
}

void VFDM12::write_string(char *str)
{
    curadr=11*3;
    while (*str)
    {
        int o=*str++ * 2;
        send_segments(curadr, pgm_read_byte(&vfdchars[o]), pgm_read_byte(&vfdchars[o+1]), 0);
        if (curadr==0)
            return;
        curadr -= 0x03;
    }
}

