
#ifndef _VFDM12_h
#define _VFDM12_h

extern "C" {
#include <../Wire/Wire.h>

}

class VFDM12
{
private:

public:
	char curadr;
	int vfdaddr;
	VFDM12();
	void init();
	void init(int adr);
	void send_segments(char adress, char c, char d, char e); // e is always empty
	void send_led(char c);
	void clear();
	void send_char(char c);
	void write_string(char *str);
};

#endif //_VFDM12_h
