#include <../SPI/SPI.h> 

//#define _cspin 10
//#define _auxpin 9
/*
00H ─ 1st & 2nd Screens are Displayed off ○
01H ─ 1st Screen is Displayed on
02H ─ 2nd Screen is Displayed on
04H ─ Read/Write address is automatically incremented ○
05H ─ Read/Write address is held
06H ─ Clear Screen
07H D0～D7 Control Power supply 00H
08H D0～D7 Display data write-in
09H D0～D7 Display data read-out
0AH D0～D7 Setting lower address for 1st Screen started 00H
0BH D0～D3 Setting upper address for 1st Screen started 00H
0CH D0～D7 Setting lower address for 2nd Screen started 00H
0DH D0～D3 Setting upper address for 2nd Screen started 00H
0EH D0～D7 Setting lower address of Read/Write 00H
0FH D0～D3 Setting upper address of Read/Write 00H
10H ─ Display OR of 1st & 2nd Screens
11H ─ Display EX-OR of 1st & 2nd Screens
12H ─ Display AND of 1st & 2nd Screens
13H D0～D7 Luminance Adjustment 00H
14H D0～D7 Display Mode 00H
15H D0～D7 INT Signal Mode 00H
20H D0～D7 Display Character
21H D0～D7 Setting the Character Starting Location of RAM (X, Y)=(00H,00H)
22H D0～D7 Setting Size of Character (X,Y) =(00H,00H)
24H D0～D7 Setting Brightness of Character 00H
*/

// double buffered screenoutput

class VFD128x64BW
{
    public:
    
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
    byte curDisplay; // display to draw too, when finished drawing, activate with SwapDisplay
    bool swapMode;
    int _cspin, _auxpin;
    byte pos[3];
    byte charsize[2];
public:
    VFD128x64BW(int cspin, int auxpin)
    {
    	charsize[0]=0;
    	charsize[1]=0;
    	pos[0]=0;
    	pos[1]=0;
    	pos[1]=0;
    	_cspin=cspin;
    	_auxpin=auxpin;
        curDisplay=0;
        //init();
        swapMode=false;
    }
    
    int isSecondScreen()
    {
        if(swapMode)
        {
            if (curDisplay==1)
            {
                return 1;
            }
        }
        return 0;
    }
    
    void writeStrAt(int x, int y, char *str)
    {
    	pos[0]=x;
    	pos[2]=y;
    	Command(INCADR);
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
        DataCommand(DISPLAYMODE,BWSCREEN);// display mode
        Command(FIRSTDISPLAYON); // 1st screen
        DataCommand(LUMINANCE,0x10);    
        Command(CLEARSCREEN);// clear screen;
        Command(INCADR);
	    noSwap();
        EraseScreen();
        delay(500);
        writeStrAt(0,0,"Ready (nearly)!");
        writeStrAt(0,8,"and wait!");

/*	    ByteCommand(CHARACTERXY,3,pos);
	    ByteCommand(CHARACTERSIZE,2,charsize);
	    StrCommand(SHOWCHARACTER,"Ready (nearly)!");
	    */
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
    void noSwap()
    {
        swapMode=false;
    }
    
    void ByteCommand(int cmd, int size, byte* data)
    {
        digitalWrite(_auxpin, COMMANDOUT);
        digitalWrite(_cspin, LOW);
        SPI.transfer(cmd);
        digitalWrite(_cspin, HIGH);
        while (size--)
        {
           sendSingleByteData(*data++);
        }
        digitalWrite(_cspin, HIGH);    
        digitalWrite(_auxpin, COMMANDOUT);
    }
    
    void ClearCommand(int cmd, int size)
    {
        digitalWrite(_auxpin, COMMANDOUT);
        digitalWrite(_cspin, LOW);
        SPI.transfer(cmd);
        digitalWrite(_cspin, HIGH);
        while (size--)
        {
           sendSingleByteData(0x00);
        }
        digitalWrite(_auxpin, COMMANDOUT);
    }
    
    void DataCommand(int cmd, int data)
    {
        digitalWrite(_auxpin, COMMANDOUT);
        digitalWrite(_cspin, LOW);
        SPI.transfer(cmd);
        digitalWrite(_cspin, HIGH);
        digitalWrite(_auxpin, DATAOUT);
        digitalWrite(_cspin, LOW);
        SPI.transfer(data);
        digitalWrite(_cspin, HIGH);    
        digitalWrite(_auxpin, COMMANDOUT);
    }
    
    
    unsigned ReadCommand(int cmd, int data)
    {
        digitalWrite(_auxpin, COMMANDOUT);
        digitalWrite(_cspin, LOW);
        SPI.transfer(cmd);
        digitalWrite(_cspin, HIGH);
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

    void SetPixel(int x, int y, bool noRead)
    {
        int adr=(y/8)+(x*8);
        //adr=0;
        int bitset=1<<(7-(y&7));
        // bitset=255;
        if(swapMode)
        {
            if (curDisplay==1)
            {
                adr += 0x400;
            }
        }
        DataCommand(0x0e,adr&0xff);
        DataCommand(0x0f,(adr>>8)&0x0f);
        int val=0;
        if (!noRead)
        {
            val=ReadCommand(0x9,0);
            DataCommand(0x0e,adr&0xff); // should not be necessary, but the bitch needs it
            DataCommand(0x0f,(adr>>8)&0x0f);
            DataCommand(DATAWRITEIN,val|bitset);
        }
        else
           DataCommand(DATAWRITEIN,bitset);

        
    }
    
    void ClrPixel(int x, int y)
    {
        int adr=(y/8)+(x*8);
        int bitset=1<<(7-(y&7));
       
        DataCommand(0x0e,adr&0xff);
        DataCommand(0x0f,(adr>>8)&0x0f);
        int val=ReadCommand(0x9,0);
        DataCommand(DATAWRITEIN,val&~bitset);
    }
    
    void HorLine(int x, int y, int h)
    {
        while (h--)
        {
            SetPixel(x, y++,false);
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
            SetPixel(x, y++,false);
        }
    }

    
    
    unsigned char vertLineArray[8];
    
    void EraseLine()
    {
        memset(vertLineArray,0,sizeof(vertLineArray));
    }
    
    
    void FlushLine(int x)
    {
        int adr=(x*8);
        if(swapMode)
        {
            if (curDisplay==1)
            {
                adr += 0x400;
            }
        }
        DataCommand(0x0e,adr&0xff);
        DataCommand(0x0f,(adr>>8)&0x0f);
        ByteCommand(DATAWRITEIN, 8, vertLineArray);
        
    }
    
    void VLine(int x, int y, int h)
    {
        if (h<0)
        {
            y+=h;
            h=-h;
        }
        int bit=1<<(7-(y & 0x07));
        int pos=y/8;
        while (h--)
        {
            vertLineArray[pos]|=bit;
            if (bit & 0x1)
            {
                bit=0x80;
                pos++;
                if (pos > 7)
                    break;
            }
            else
                bit >>= 1;
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
    
        SetPixel(x0,y0,false);
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
	        SetPixel(x0,y0,false);
        }
    }
        


    void Command(int cmd)
    {
        digitalWrite(_auxpin, COMMANDOUT);
        digitalWrite(_cspin, LOW);
        SPI.transfer(cmd);
        digitalWrite(_cspin, HIGH);    
        digitalWrite(_auxpin, COMMANDOUT);
    }
    
    
    // maybe use erase with big character space
    void EraseScreen()
    {
        unsigned int adr=0;
        if(swapMode)
        {
            if (curDisplay==1)
            {
                adr += 0x400;
            }
        }
        DataCommand(0x0e,adr&0xff);
        DataCommand(0x0f,(adr>>8)&0x0f);
        Command(INCADR);
        
        ClearCommand(0x08, 0x400);   
    }
    
    void SwapDisplay()
    {
        swapMode=true;
        curDisplay=curDisplay?0:1;
        unsigned int adr=0;
        if (curDisplay==0)
        {
            adr += 0x400;
        }
        DataCommand(0x0a, adr&0xff);
        DataCommand(0x0b, (adr>>8)&0x0f);
        //EraseScreen();
    }
};

// VFD128x64BW vfd=VFD128x64BW();

/*
void loop()
{
    if (newData)
    {
        newData=false;
        byte curPos=curDataWrite&0x7f;
        while (curDataRead!=curPos)
        {
            data[x++]=srdata[curDataRead];
            curDataRead++;
            curDataRead&=0x7f;
            if(x==256)
            {
                x=0;
                long sum=0;
                for (int i=0; i < 256; i++)
                {
                    sum+=abs(data[i]);
                }
                if (sum < 1000)
                    break;
               
               char outb[12];
        
               for (int i=0; i < 256; i++)
               {    
                   if ((i & 1)==0)
                       vfd.EraseLine();
                   if ((data[i])/4==0)
                   {
                       if (i%8==0)
                           vfd.VLine(i/2, 32, 1);
                   }
                   else
                       vfd.VLine(i/2, 32, (data[i])/4);
                   if ((i & 1)==1)
                       vfd.FlushLine(i/2);
               }
               sprintf(outb,"%ld %ld %ld",irCount, millis(), irCount/(millis()/1000));
               if (vfd.isSecondScreen())
               {
                   pos[2]=64;
               }
               else
                   pos[2]=0;
               vfd.ByteCommand(0x21,3,pos);
               vfd.ByteCommand(0x22,2,dblSize);
               vfd.StrCommand(0x20,outb);
               x=0;
            }
        }
    }
}
*/