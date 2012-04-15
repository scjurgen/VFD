#include <../SPI/SPI.h> 

class VFD128x64BWMem
{
    public:
    	#define SCRWIDTH 128
    	#define SCRHEIGHT 64
    
    #define FIRSTDISPLAYON 1
    #define SECONDDISPLAYON 2
    #define INCADR 4
    #define HOLDADR 5
    #define CLEARSCREEN 6
    #define CONTROLPOWER 7
    #define DATAWRITEIN 8
    #define DATAREADOUT 9
    #define SCREEN1LSB 0x0a
    #define SCREEN1MSB 0x0b
    #define SCREEN2LSB 0x0c
    #define SCREEN2MSB 0x0d
    #define RWLSB 0x0e
    #define RWMSB 0x0f
    #define DISPLAYOR 0x10
    #define DISPLAYXOR 0x11
    #define DISPLAYAND 0x12
    #define LUMINANCE 0x13
    #define DISPLAYMODE 0x14 // 0x10=BW 0x014=GRAYMODE
    #define INTSIGNALON 0x15 
    #define SHOWCHARACTER 0x20
    #define CHARACTERXY 0x21
    #define CHARACTERSIZE 0x22
    #define CHARACTERBRIGHTNESS 0x24
    #define BWSCREEN 0x10
    #define GRAY4SCREEN 0x14

    #define DATAOUT LOW
    #define COMMANDOUT HIGH

private:
    int _cspin, _auxpin;
    byte pos[3];
    byte charsize[2];
   	byte *scrBuffer;
   	int bufsize;
public:
    VFD128x64BWMem(int cspin, int auxpin)
    {
    	charsize[0]=0;
    	charsize[1]=0;
    	pos[0]=0;
    	pos[1]=0;
    	pos[1]=0;
    	_cspin=cspin;
    	_auxpin=auxpin;
	    #if GRAYMODE
	    	bufsize=0x800;
	    #else
	    	bufsize=0x400;
	    #endif
	    scrBuffer=(byte*)malloc(bufsize);
    }
    
    ~VFD128x64BWMem()
    {
	    free(scrBuffer);
    	scrBuffer=0;
	}
    
    void gridWriteStrAt(int x, int y, char *str)
    {
    	pos[0]=x*6;
    	pos[2]=y*8;
    	sendSingleByteCommand(INCADR);
    	ByteCommand(CHARACTERXY,3,pos);
    	ByteCommand(CHARACTERSIZE,2,charsize);
    	StrCommand(SHOWCHARACTER,str);
	}
    
    void writeStrAt(int x, int y, char *str)
    {
    	pos[0]=x;
    	pos[2]=y;
    	sendSingleByteCommand(INCADR);
    	ByteCommand(CHARACTERXY,3,pos);
    	ByteCommand(CHARACTERSIZE,2,charsize);
    	StrCommand(SHOWCHARACTER,str);
	}
    
    void init()
    {
        pinMode(_cspin,OUTPUT);
        pinMode(_auxpin,OUTPUT);
     
        SPI.begin();
        SPI.setBitOrder(LSBFIRST); 
        SPI.setDataMode(SPI_MODE0);
        SPI.setClockDivider(SPI_CLOCK_DIV2); 
        SPSR|=SPI_2XCLOCK_MASK;
        DataCommand(SCREEN1LSB, 0);
        DataCommand(SCREEN1MSB, 0);
        #if GRAYMODE
	        DataCommand(DISPLAYMODE,GRAY4SCREEN);// display mode
        #else
    	    DataCommand(DISPLAYMODE,BWSCREEN);// display mode
        #endif
        sendSingleByteCommand(FIRSTDISPLAYON); // 1st screen
        DataCommand(LUMINANCE,0x10);    
        sendSingleByteCommand(CLEARSCREEN);// clear screen;
        sendSingleByteCommand(INCADR);
        EraseScreen();
        writeStrAt(0,0,"Ready!");
    }
private: 
    void sendSingleByteCommand(byte cmd)
    {
        digitalWrite(_auxpin, COMMANDOUT);
        digitalWrite(_cspin, LOW);
        SPI.transfer(cmd);
        digitalWrite(_cspin, HIGH);
    }

    void sendSingleByteData(byte cmd)
    {
        digitalWrite(_auxpin, DATAOUT);
        digitalWrite(_cspin, LOW);
        SPI.transfer(cmd);
        digitalWrite(_cspin, HIGH);
        digitalWrite(_auxpin, COMMANDOUT);
    }

public:    
    
    void ByteCommand(int cmd, int size, byte* data)
    {
	    sendSingleByteCommand(cmd);
        while (size--)
        {
           sendSingleByteData(*data++);
        }
        digitalWrite(_cspin, HIGH);    
        digitalWrite(_auxpin, COMMANDOUT);
    }
    
    void ClearCommand(int cmd, int size)
    {
	    sendSingleByteCommand(cmd);
        while (size--)
        {
           sendSingleByteData(0x00);
        }
        digitalWrite(_auxpin, COMMANDOUT);
    }
    
    void DataCommand(int cmd, int data)
    {
	    sendSingleByteCommand(cmd);
	    sendSingleByteData(data);
    }
    
    
    unsigned ReadCommand(int cmd, int data)
    {
	    sendSingleByteCommand(cmd);

        digitalWrite(_auxpin, DATAOUT);
        digitalWrite(_cspin, LOW);
        int dummy=SPI.transfer(data);
        byte val1=SPI.transfer(data);
        byte val2=SPI.transfer(data);
        digitalWrite(_cspin, HIGH);    
        digitalWrite(_auxpin, COMMANDOUT);
        return (unsigned)val1|(val2<<8);
    }
    
        
public:    
    void StrCommand(int cmd, char* data)
    {
        digitalWrite(_auxpin, COMMANDOUT);
        digitalWrite(_cspin, LOW);
        SPI.transfer(cmd);
        digitalWrite(_cspin, HIGH);
        while(*data)
            sendSingleByteData(*data++);
        digitalWrite(_auxpin, COMMANDOUT);
    }

    void SetPixel(int x, int y)
    {
    	if (x<0) return;
    	if (y<0) return;
    	if (x>=SCRWIDTH) return;
    	if (y>=SCRHEIGHT) return;
        int adr=(y/8)+(x*8);
        int bitset=1<<(7-(y&7));
        scrBuffer[adr]|=bitset;        
    }
    
    void ClrPixel(int x, int y)
    {
    	if (x<0) return;
    	if (y<0) return;
    	if (x>=SCRWIDTH) return;
    	if (y>=SCRHEIGHT) return;
        int adr=(y/8)+(x*8);
        int bitset=1<<(7-(y&7));
        scrBuffer[adr]&=~bitset;
    }
    
    void HorLine(int x, int y, int h)
    {
        while (h--)
        {
            SetPixel(x, y++);
        }
    }
    
    void VertLine(int x, int y, int h)
    {
        if (h < 0)
        {
            y+=h;
            h=-h;
        }
        // this can be optimized by calculating the complete pattern first and then or it to the memory
        while (h--)
        {
            SetPixel(x, y++);
        }
    }

    
    
    unsigned char vertLineArray[8];
    
    void VLine(int x, int y, int h)
    {
        if (h<0)
        {
            y+=h;
            h=-h;
        }
    	if (x<0) return;
    	if (y<0) return;
    	if (x>=SCRWIDTH) return;
    	if (y>=SCRHEIGHT) return;
        int adr=(y/8)+(x*8);
        int bitset=1<<(7-(y&7));
        while (h--)
        {
            scrBuffer[adr]|=bitset;
            if (bitset & 0x1)
            {
                bitset=0x80;
                adr+=8;
                if (adr >= bufsize)
                    break;
            }
            else
                bitset >>= 1;
        }
        
    } 

    
    void line(int x0, int y0, int x1, int y1)
    {
        int sx,sy;
        int dx = abs(x1-x0);
        int dy = abs(y1-y0);
        if (x0 < x1)
            sx = 1;
        else
            sx = -1;
        if (y0 < y1)
            sy = 1;
        else
            sy = -1;
        int err = dx-dy;
    
        SetPixel(x0,y0);
        while (x0 != x1 || y0 != y1)
        {
            int e2 = 2*err;
            if (e2 > -dy)
            {
                err = err - dy;
                x0 = x0 + sx;
            }
            else
            if (e2 <  dx)
            {
                err = err + dx;
                y0 = y0 + sy;
            }
	        SetPixel(x0,y0);
        }
    }
    
    void NewScreen()
    {
    	memset(scrBuffer,0,bufsize);
	}
   
    void ShowScreen()
  	{
        DataCommand(RWLSB,0);
        DataCommand(RWMSB,0);
        sendSingleByteCommand(INCADR);     
	    sendSingleByteCommand(DATAWRITEIN);
	    for (int i=0; i < bufsize; i++)
        {
           sendSingleByteData(scrBuffer[i]);
        }
  	}
    
    // maybe use erase with big character space
    void EraseScreen()
    {
        DataCommand(RWLSB,0);
        DataCommand(RWMSB,0);
        sendSingleByteCommand(INCADR);        
        ClearCommand(DATAWRITEIN, 0x400);   
    }
    
};

class VFD128x64Gray4Mem
{
    public:
    	#define SCRWIDTH 128
    	#define SCRHEIGHT 64
    
    #define FIRSTDISPLAYON 1
    #define SECONDDISPLAYON 2
    #define INCADR 4
    #define HOLDADR 5
    #define CLEARSCREEN 6
    #define CONTROLPOWER 7
    #define DATAWRITEIN 8
    #define DATAREADOUT 9
    #define SCREEN1LSB 0x0a
    #define SCREEN1MSB 0x0b
    #define SCREEN2LSB 0x0c
    #define SCREEN2MSB 0x0d
    #define RWLSB 0x0e
    #define RWMSB 0x0f
    #define DISPLAYOR 0x10
    #define DISPLAYXOR 0x11
    #define DISPLAYAND 0x12
    #define LUMINANCE 0x13
    #define DISPLAYMODE 0x14 // 0x10=BW 0x014=GRAYMODE
    #define INTSIGNALON 0x15 
    #define SHOWCHARACTER 0x20
    #define CHARACTERXY 0x21
    #define CHARACTERSIZE 0x22
    #define CHARACTERBRIGHTNESS 0x24
    #define BWSCREEN 0x10
    #define GRAY4SCREEN 0x14

    #define DATAOUT LOW
    #define COMMANDOUT HIGH

private:
    int _cspin, _auxpin;
    byte pos[3];
   	byte scrBuffer[0x800];
public:
    VFD128x64Gray4Mem(int cspin, int auxpin)
    {
    	pos[0]=0;
    	pos[1]=0;
    	pos[1]=0;
    	_cspin=cspin;
    	_auxpin=auxpin;
    }

    
    void writeStrAt(int x, int y, char *str, int col) {
        pos[0]=x;
        pos[2]=y;
        sendSingleByteCommand(INCADR);
        DataCommand(CHARACTERBRIGHTNESS, col);
        ByteCommand(CHARACTERXY,3,pos);
        StrCommand(SHOWCHARACTER,str);
    }

    
    void init()
    {
        pinMode(_cspin,OUTPUT);
        pinMode(_auxpin,OUTPUT);
     
        SPI.begin();
        SPI.setBitOrder(LSBFIRST); 
        SPI.setDataMode(SPI_MODE0);
        SPI.setClockDivider(SPI_CLOCK_DIV2); 
        SPSR|=SPI_2XCLOCK_MASK;
        DataCommand(SCREEN1LSB, 0);
        DataCommand(SCREEN1MSB, 0);
        DataCommand(DISPLAYMODE,GRAY4SCREEN);// display mode
        sendSingleByteCommand(FIRSTDISPLAYON); // 1st screen
        DataCommand(LUMINANCE,0x10);    
        sendSingleByteCommand(CLEARSCREEN);// clear screen;
        sendSingleByteCommand(INCADR);
        EraseScreen();
        writeStrAt(0,0,"Ready!",1);
    }
private: 
    void sendSingleByteCommand(byte cmd)
    {
        digitalWrite(_auxpin, COMMANDOUT);
        digitalWrite(_cspin, LOW);
        SPI.transfer(cmd);
        digitalWrite(_cspin, HIGH);
    }

    void sendSingleByteData(byte cmd)
    {
        digitalWrite(_auxpin, DATAOUT);
        digitalWrite(_cspin, LOW);
        SPI.transfer(cmd);
        digitalWrite(_cspin, HIGH);
        digitalWrite(_auxpin, COMMANDOUT);
    }

public:    
    
    void ByteCommand(int cmd, int size, byte* data)
    {
	    sendSingleByteCommand(cmd);
        while (size--)
        {
           sendSingleByteData(*data++);
        }
        digitalWrite(_cspin, HIGH);    
        digitalWrite(_auxpin, COMMANDOUT);
    }
    
    void ClearCommand(int cmd, int size)
    {
	    sendSingleByteCommand(cmd);
        while (size--)
        {
           sendSingleByteData(0x00);
        }
        digitalWrite(_auxpin, COMMANDOUT);
    }
    
    void DataCommand(int cmd, int data)
    {
	    sendSingleByteCommand(cmd);
	    sendSingleByteData(data);
    }
    
    
    unsigned ReadCommand(int cmd, int data)
    {
	    sendSingleByteCommand(cmd);

        digitalWrite(_auxpin, DATAOUT);
        digitalWrite(_cspin, LOW);
        int dummy=SPI.transfer(data);
        byte val1=SPI.transfer(data);
        byte val2=SPI.transfer(data);
        digitalWrite(_cspin, HIGH);    
        digitalWrite(_auxpin, COMMANDOUT);
        return (unsigned)val1|(val2<<8);
    }
    
        
public:    
    void StrCommand(int cmd, char* data)
    {
        digitalWrite(_auxpin, COMMANDOUT);
        digitalWrite(_cspin, LOW);
        SPI.transfer(cmd);
        digitalWrite(_cspin, HIGH);
        while(*data)
            sendSingleByteData(*data++);
        digitalWrite(_auxpin, COMMANDOUT);
    }

    void SetPixel(int x, int y, int col)
    {
    	if (x<0) return;
    	if (y<0) return;
    	if (x>=SCRWIDTH) return;
    	if (y>=SCRHEIGHT) return;
        int adr=(y/4)+(x*16);
        int bitset=col<<((3-(y&3))<<1);
        scrBuffer[adr]|=bitset;        
    }
    
    void ClrPixel(int x, int y)
    {
    	if (x<0) return;
    	if (y<0) return;
    	if (x>=SCRWIDTH) return;
    	if (y>=SCRHEIGHT) return;
        int adr=(y/4)+(x*16);
        int bitset=3<<((3-(y&3))<<1);
        scrBuffer[adr]&=~bitset;
    }
    

    void HorLine(int x, int y, int h, int col) {
        while (h--) {
            SetPixel(x, y++, col);
        }
    }


    void VertLine(int x, int y, int h, int col) {
        if (h < 0) {
            y+=h;
            h=-h;
        }
        while (h-- > 0) {
            SetPixel(x, y++, col);
        }
    }


    void line(int x0, int y0, int x1, int y1, int col) {
        int sx,sy;
        int dx = abs(x1-x0);
        int dy = abs(y1-y0);
        if (x0 < x1)
            sx = 1;
        else
            sx = -1;
        if (y0 < y1)
            sy = 1;
        else
            sy = -1;
        int err = dx-dy;

        SetPixel(x0,y0, col);
        while (x0 != x1 || y0 != y1) {
            int e2 = 2*err;
            if (e2 > -dy) {
                err = err - dy;
                x0 = x0 + sx;
            } else
                if (e2 <  dx) {
                    err = err + dx;
                    y0 = y0 + sy;
                }
            SetPixel(x0, y0, col);
        }
    }

    void NewScreen()
    {
    	memset(scrBuffer,0,sizeof(scrBuffer));
	}

    void ClearScreen() {
        memset(scrBuffer,0,sizeof(scrBuffer));
    }
    
    void ShowScreen()
    {
        DataCommand(RWLSB, 0);
        DataCommand(RWMSB, 0);
        sendSingleByteCommand(INCADR);
        sendSingleByteCommand(DATAWRITEIN);
        for (int i=0; i < sizeof(scrBuffer); i++) {
            sendSingleByteData(scrBuffer[i]);
        }
    }

    // maybe use erase with big character space
    void EraseScreen() {
        DataCommand(RWLSB, 0);
        DataCommand(RWMSB, 0);
        sendSingleByteCommand(INCADR);
        ClearCommand(DATAWRITEIN, sizeof(scrBuffer));
    }
};
