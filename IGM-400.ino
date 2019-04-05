#include <Adafruit_GFX.h>    // Core graphics library

#include "Adafruit_ST7735.h" // Hardware-specific library

#define TFT_CS     PA2
#define TFT_RST    PA4
#define TFT_DC     PA3

#include <Fonts/FreeSans24pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);

float pressure = 0;
float last_pressure = 0;
uint8_t exponent = 0;
uint8_t last_exponent = 0;
uint8_t stat = 0;

uint8_t pressure_history[136];
uint8_t color_history[136];

void setup(void) {
  pinMode(PA8, OUTPUT);
  pinMode(PA9, OUTPUT);

  pinMode(PB3, OUTPUT);
  pinMode(PB4, OUTPUT);
  pinMode(PB7, OUTPUT);
  pinMode(PB8, OUTPUT);
  pinMode(PB9, OUTPUT);

  digitalWrite(PA8, LOW);
  digitalWrite(PA9, LOW);

  digitalWrite(PB3, LOW);
  digitalWrite(PB4, LOW);
  digitalWrite(PB7, LOW);
  digitalWrite(PB8, LOW);
  digitalWrite(PB9, LOW);


  Serial.begin();
  Serial.print("Hello! ST7735 TFT Test");

  // Use this initializer if you're using a 1.8" TFT
  tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab

  tft.setRotation(3);
  tft.fillScreen(ST7735_BLACK);

  Serial.println("Initialized");

  tft.setCursor(35, 2);
  tft.setFont();
  tft.print("IGM-400 Display");
}

void updatePressure() {
  tft.setTextColor(ST7735_WHITE);

  if (last_pressure * 100 != pressure * 100) {
    tft.fillRect(7, 47, 90, 34, ST7735_BLACK);
    last_pressure = pressure;
  }

  if (last_exponent != exponent) {
    tft.fillRect(122, 38, 20, 25, ST7735_BLACK);
    tft.fillRect(150, 89, 7, 34, ST7735_BLACK);
    last_exponent = exponent;
  }
  
  tft.setCursor(7, 73);
  tft.setFont(&FreeSans24pt7b);
  tft.print(pressure);

  tft.setCursor(97, 79);
  tft.setFont(&FreeSans9pt7b);
  tft.print("x10");

  tft.setCursor(112, 62);
  tft.setFont(&FreeSans18pt7b);
  tft.print("-");
  tft.print(exponent);

  tft.setCursor(130, 79);
  tft.setFont();
  tft.print("mbar");  
}

void updateGraph() {
  pressure_history[0] = pressure * 3.2;
  color_history[0] = (exponent * 25) + (pressure * 2.5);

  for (int i = sizeof(pressure_history); i > 0; i--) {
    tft.drawFastVLine(7+(137-i), 87 + (33 - pressure_history[i]), pressure_history[i], ST7735_BLACK);
    pressure_history[i] = pressure_history[i-1];
    color_history[i] = color_history[i-1];
    tft.drawFastVLine(7+(137-i), 87 + (33 - pressure_history[i-1]), pressure_history[i-1], ((color_history[i-1] & 0xF8) << 8) | (((255 - color_history[i-1]) & 0xFC) << 3));
  }

  tft.drawFastVLine(144, 89, 31, ST7735_WHITE);

  tft.drawFastHLine(146, 92, 3, ST7735_WHITE);
  tft.drawFastHLine(146, 114, 3, ST7735_WHITE);

  tft.drawFastHLine(8, 119, 137, ST7735_WHITE);

  tft.setTextColor(ST7735_WHITE);
  tft.setFont();
  tft.setCursor(150, 89);
  tft.print(exponent-1); 
  tft.setCursor(150, 111);
  tft.print((exponent)); 


  tft.setCursor(8, 121);
  tft.print("-10s");

  tft.setCursor(73, 121);
  tft.print("-5s");

  tft.setCursor(139, 121);
  tft.print("0s");
}

uint8_t powerbtn_state = 0;


void loop() {
  if (digitalRead(PB5) == LOW && powerbtn_state == 1) {
    stat = !stat;
    tft.fillRect(7, 24, 80, 12, ST7735_BLACK);
  }
  powerbtn_state = digitalRead(PB5);

  digitalWrite(PA8, (stat == 1));
  digitalWrite(PB9, (stat == 1));

  
  tft.setFont();
  tft.setTextColor(ST7735_WHITE);
  tft.setCursor(8, 15);
  tft.print("Status:");
  
  tft.setCursor(8, 25);
  if (stat == 1) {
    tft.setTextColor(ST7735_GREEN);
    tft.print("active");
  } else if (stat == 0) {
    tft.setTextColor(ST7735_RED);
    tft.print("off");  
  }

  tft.setTextColor(ST7735_WHITE);
  tft.setCursor(80, 15);
  tft.print("Mode:");
  tft.setCursor(80, 25);
  tft.setTextColor(ST7735_WHITE);
  tft.print("100uA");  
  
  updatePressure();
  updateGraph();

  // 3,115264798V = 10V
  // 3866 = 10V

  float analog = ((float)analogRead(PA1) / 3866.0f) - ((float)analogRead(PA0) / 12410.0f);
  exponent = 10 - (analog * 10.0f);

  for (int i = 0; i < 35; i++) {
    pressure = pressure * 0.99f + pow(10.0, (analog * 10.0) - (int)(analog * 10.0)) * 0.01f;
    delay(2);
  }

  if (exponent >= 10) {
    exponent = 9;
  }

  if (pressure >= 9.99) {
    pressure = 0;
  } 
}

