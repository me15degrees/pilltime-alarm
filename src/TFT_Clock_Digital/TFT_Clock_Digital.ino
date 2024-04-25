#include <TFT_eSPI.h>
#include <SPI.h>

#define TFT_GREY 0x5AEB

TFT_eSPI tft = TFT_eSPI();       
uint32_t targetTime = 0;                    
static uint8_t conv2d(const char* p); 

uint8_t hh = conv2d(__TIME__), mm = conv2d(__TIME__ + 3), ss = conv2d(__TIME__ + 6); 

byte omm = 99, oss = 99;
byte xcolon = 0, xsecs = 0;
unsigned int colour = 0;

void setup(void) {

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  tft.setTextSize(1);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);

  targetTime = millis() + 1000;
}

void loop() {
  if (targetTime < millis()) {
    // Set next update for 1 second later
    targetTime = millis() + 1000;


    ss++;              
    if (ss == 60) {   
      ss = 0;        
      omm = mm;   
      mm++;       
      if (mm > 59) {   
        mm = 0;
        hh++;       
        if (hh > 23) { 
          hh = 0;     
        }
      }
    }

    int xpos = 0;
    int ypos = 85; 
    int ysecs = ypos + 24;

    if (omm != mm) { 
      omm = mm;
    
      if (hh < 10) xpos += tft.drawChar('0', xpos, ypos, 8);
      xpos += tft.drawNumber(hh, xpos, ypos, 8);            
      xcolon = xpos;
      xpos += tft.drawChar(':', xpos, ypos - 8, 8);
      if (mm < 10) xpos += tft.drawChar('0', xpos, ypos, 8); 
      xpos += tft.drawNumber(mm, xpos, ypos, 8);            
      xsecs = xpos;
    }
    if (oss != ss) {
      oss = ss;
      xpos = xsecs;

      if (ss % 2) {
        tft.setTextColor(0x39C4, TFT_BLACK);      
        tft.drawChar(':', xcolon, ypos - 8, 8);    
        xpos += tft.drawChar(':', xsecs, ysecs, 6);
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);    
      }
      else {
        tft.drawChar(':', xcolon, ypos - 8, 8);    
        xpos += tft.drawChar(':', xsecs, ysecs, 6); 
      }

      //Draw seconds
      if (ss < 10) xpos += tft.drawChar('0', xpos, ysecs, 6); 
      tft.drawNumber(ss, xpos, ysecs, 6);                    
    }
  }
}


static uint8_t conv2d(const char* p) {
  uint8_t v = 0;
  if ('0' <= *p && *p <= '9')
    v = *p - '0';
  return 10 * v + *++p - '0';
}



