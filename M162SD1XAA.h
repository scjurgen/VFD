
//#include <avr/pgmspace.h>
 #include <assert.h>
// 
/*
                   MSB 1st Byte               MSB 2nd Byte
                   B7 B6 B5 B4 B3 B2 B1 B0    B7 B6 B5 B4 B3 B2 B1 B0
DCRAM_A DATA WRITE  0  0  0 X4 X3 X2 X1 X0    C7 C6 C5 C4 C3 C2 C1 C0
DCRAM_B DATA WRITE  0  0  1 X4 X3 X2 X1 X0    C7 C6 C5 C4 C3 C2 C1 C0
CGRAM DATA WRITE    0  1  0  *  * Y2 Y1 Y0    D30 D25 D20 D15 D10 D5 D0 2nd Byte
                                         	  * D31 D26 D21 D16 D11 D6 D1 3rd Byte
											  * D32 D27 D22 D17 D12 D7 D2 4th Byte
											  * D33 D28 D23 D18 D13 D8 D3 5th Byte
											  * D34 D29 D24 D19 D14 D9 D4 6th Byte
NUMBER OF DIGIT SET 1  1  1  0  0  0  *  *    0  *  *  *  F3 F2 F1 F0
DIMMING SET         1  1  1  0  0  1  *  *    H7 H6 H5 H4 H3 H2 H1 H0
GRAY-LEVEL SET      1  0  1  *  * J2 J1 J0    I7 I6 I5 I4 I3 I2 I1 I0
GRAY ON/OFF SET     1  1  0 X4 X3 X2 X1 X0    *  *  0  0  0  0  K1 K0
DISPLAY LIGHT SET   1  1  1  0  1  0 LS HS    *  *  *  *  *  *  *  *
*: Not Relevant
Xn: Duty Timing (Digit) Address Set, n = 0 to 4
Cn: CGRAM/CGROM Character Code Bit, n = 0 to 7
Yn: CGRAM Address Bit, n = 0 to 2
Dn: CGRAM Character Code Setting, n = 0 to 34
Fn: Number of Digits Set, n = 0 to 3
Hn: Dimming Quantity Setting, n = 0 to 7
Jn: Gray-Level Register Setting, n = 0 to 2
In: Gray-Level Quantity Setting, n = 0 to 7
Kn: Each Gray-Level Enable/Disable Setting, n = 0 to 1
HS: 1: All Output (Anode, Segment) Data = H 0: Normal Mode
LS: 1: All Output (Anode, Segment) Data = L 0: Normal Mode
*/
#define VFDM162_DCRAMADW 0x00   // + X4...X0 duty timing digit addres set n=0..4
#define VFDM162_DCRAMBDW 0x20
#define VFDM162_CGRAMADW 0x40   // + Y2..Y0 CGRAM Address Bit, n = 0 to 2 
								// send 6 bytes in total * D30D25 (vertical encoded)

#define VFDM162_NUMOFDIGITSET 0xE0 // 2nd byte 0 * * * F3..F0
#define VFDM162_DIMMINGSET 0xE4 // 2nd byte H7..F0
#define VFDM162_GRAYLEVEL 0xA0  // + J2..J0   //2nd byte I7..I0
#define VFDM162_GRAYLEVELONOFF 0xC0 //+X4..X0 // 2nd Byte  * * 0 0 0 0 K1 K0
#define VFDM162_DISPLIGHTSET 0xE8     // +
#define MAXCHARPOS 16

class M162SD13AA
{
private:
	void sendCommandData(uint8_t cmd, uint8_t data);
	void sendDgramData(uint8_t cmd, uint8_t data[5]);
	int curPos;
public:
	char _cspin;
	M162SD13AA();
	void init();
	void init(uint8_t cspin);
	void clear();
	void writeChar(char c);
	void writeChar(char c, uint8_t col);
	void writeChar(uint8_t row, uint8_t col,char c);
	void writeString(char *str);
	void writeString(uint8_t row, char *str);
	void switchOn();
	void switchOff();
	void setDimming(uint8_t val); // 0..240, higher values equal 240
	void setGray(uint8_t addr, uint8_t enable); // 0..4,  set gray level on off
	void setGrayLevel(uint8_t row, uint8_t level); // don't set level=0
	void setCustomCharacter(uint8_t nr, uint8_t data[5]);
	void setNumberOfVisibleDigits(uint8_t c);
};

M162SD13AA::M162SD13AA()
{
	curPos=0;
}

void M162SD13AA::init(uint8_t cspin)
{
	_cspin=cspin;
	pinMode(_cspin,OUTPUT);
	SPI.begin();
    SPI.setBitOrder(LSBFIRST); 
    SPI.setDataMode(SPI_MODE0);
    SPI.setClockDivider(SPI_CLOCK_DIV32); 
	curPos=0;
	for (int i=0; i < 16; i++)
		setGray(i,3);
	//for (int i=0; i < 7; i++)
	setGrayLevel(0,240);
	setGrayLevel(1,240);
	setDimming(240);
	setNumberOfVisibleDigits(15);
	writeString("Hello world");

	sendCommandData(VFDM162_DISPLIGHTSET+0,0);
	delay(1000);
}


void M162SD13AA::sendCommandData(byte cmd, byte data)
{
	digitalWrite(_cspin, LOW);
	SPI.transfer(cmd);
	SPI.transfer(data);
	digitalWrite(_cspin, HIGH);
}

void M162SD13AA::sendDgramData(byte cmd, byte data[5])
{
	digitalWrite(_cspin, LOW);
	SPI.transfer(cmd);
	for (int i=0; i < 5; i++) {
		SPI.transfer(data[i]);
	}
	digitalWrite(_cspin, HIGH);
}
   
void M162SD13AA::init()
{
	init(10);
}


void M162SD13AA::clear()
{
}

void M162SD13AA::setNumberOfVisibleDigits(uint8_t pos)
{
	assert(pos<16);
   sendCommandData(VFDM162_NUMOFDIGITSET,(uint8_t)pos);
}

void M162SD13AA::writeChar(char c, uint8_t pos)
{
   sendCommandData(VFDM162_DCRAMADW+pos,(uint8_t)c);
}

void M162SD13AA::writeChar(uint8_t line, uint8_t pos, char c)
{
	switch(line)
	{
		case 0:
			sendCommandData(VFDM162_DCRAMADW+pos,(uint8_t)c);
			break;
		case 1:
			sendCommandData(VFDM162_DCRAMBDW+pos,(uint8_t)c);
			break;
		default:
			assert(0);
			break;
	}
}

void M162SD13AA::writeChar(char c)
{
   sendCommandData(VFDM162_DCRAMADW+curPos,(uint8_t)c);
   curPos++;
   if (curPos>=MAXCHARPOS)
   	curPos=0;
}


void M162SD13AA::setGray(uint8_t dutyTime, uint8_t enable)
{
   sendCommandData(VFDM162_GRAYLEVELONOFF+dutyTime,enable);
}
void M162SD13AA::setGrayLevel(uint8_t reg, uint8_t level)
{
   sendCommandData(VFDM162_GRAYLEVEL+reg,level);
}

void M162SD13AA::writeString(char *str)
{
    while (*str)
    {
       writeChar(*str++);
    }
}

void M162SD13AA::writeString(uint8_t line, char *str)
{
	uint8_t pos=0;
    while (*str)
    {
       writeChar(line, pos++, *str++);
    }
}

void M162SD13AA::switchOn()
{
	sendCommandData(VFDM162_DISPLIGHTSET+0,0);
}

void M162SD13AA::switchOff()
{
	sendCommandData(VFDM162_DISPLIGHTSET+1,0);
}

void M162SD13AA::setDimming(uint8_t val)
{
	sendCommandData(VFDM162_DIMMINGSET,val);
}


void M162SD13AA::setCustomCharacter(uint8_t nr, uint8_t data[5])
{
	sendDgramData(VFDM162_CGRAMADW+nr,data);
}

