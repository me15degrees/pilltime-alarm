#include "Adafruit_GFX.h"     
#include "Adafruit_ILI9341.h" 
#include <Wire.h>
#include <SPI.h>

#define TFT_DC 2             
#define TFT_CS 15      
#define TFT_RST 4  
#define TFT_MISO 19         
#define TFT_MOSI 23        
#define TFT_CLK 18     

#define PURPLE_LIGHT 0xC80FF
#define YELLOW_LIGHT 0xFFFF00

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

char compartimento[] = "Compartimento 1";//alterar para varivel com o compartimento
char remedio[] = "Dramin";//alterar para varivel com o nome do remedio

#include <Fonts/FreeSans9pt7b.h>

void setup() {
  Serial.begin(9600);
  tft.begin();                      
  tft.setRotation(1);            
  tft.fillScreen(PURPLE_LIGHT);

  tft.setFont(&FreeSans9pt7b);

  printText(compartimento, YELLOW_LIGHT, 20, 80, 2);
  printText(remedio, YELLOW_LIGHT, 20, 130, 2);
  
}

void loop() {

}

void printText(char *text, uint16_t color, int x, int y, int textSize) {
  tft.setCursor(x, y);
  tft.setTextColor(color);
  tft.setTextSize(textSize);
  tft.setTextWrap(true);
  tft.print(text);
}
