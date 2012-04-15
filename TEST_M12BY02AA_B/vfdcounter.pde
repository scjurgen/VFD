
#include <avr/pgmspace.h>
#include <Wire.h>
#include "VFDM12.h"

VFDM12 vfd;

void setup()
{  
   Wire.begin();
   vfd.init();
  Serial.begin(9600);
  delay(1000);
  Serial.println(vfd.vfdaddr);
}

void loop()
{
    vfd.send_led(7);
    for (int n=0; n < 1500; n++)
    {
        char outb[14];
        sprintf(outb,"%4d HZ ", n);
        vfd.write_string(outb);
        //delay(1);
    }
    vfd.send_led(0);
    vfd.send_char('D');
    vfd.send_char('O');
    vfd.send_char('N');
    vfd.send_char('E');
    delay(1000);
    vfd.clear();
}


