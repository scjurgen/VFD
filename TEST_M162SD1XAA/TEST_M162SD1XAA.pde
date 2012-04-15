#include <SPI.h>
#include "M162SD1XAA.h"

M162SD13AA vfd;

#define CSPIN 4


/*
0  
1   X X
2  
3  X   x
4   xxx
5
6




*/
void setup()
{
	uint8_t customChar[5]={8,18,16,18,8};
	vfd.init(CSPIN);
	vfd.setCustomCharacter(2,customChar);
	vfd.setDimming(200);
	vfd.setGrayLevel(0,60);
	vfd.setGrayLevel(1,240);
}




void loop()
{
	static const char *tickermessage="                Hello VFD reader! \002                ";
	static uint8_t curTickerPos=0;
	static uint32_t nextT=0;
	static uint8_t bounceCharacter;
	if (millis()>nextT)
	{
		char outb[18];
		memcpy(outb,tickermessage+curTickerPos,16);
		outb[16]=0;
		curTickerPos++;
		if (curTickerPos >=strlen(tickermessage)-16)
			curTickerPos=0;
		vfd.writeString(0,outb);
		int secs=nextT/1000;
		sprintf(outb,"T=%2d:%02d %c", secs/60,secs%60, bounceCharacter+16);
		bounceCharacter++;
		if (bounceCharacter==13)
			bounceCharacter=0;
		vfd.writeString(1,outb);
		nextT=millis()+100;
	}
}