#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define Width 128
#define Height 64
#define Reset -1
#define Address 0x3C

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire,Reset);

void setup() {
  Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  display.begin(SSD1306_SWITCHCAPVCC, Address); // Address 0x3C for 128x32
  display.clearDisplay();
  
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("connected!");
  display.setCursor(0,0);
  display.display(); // actually display all of the above
  delay(1000);
  display.clearDisplay();
  display.display();

}

void loop() {
  Serial.println("Enter your phrase");
  if (Serial.available() > 0) {
    String i = Serial.readString();
    if (i.compareTo("clear")==10){
      display.clearDisplay();
      Serial.println(i);
      display.setCursor(0,0);
    }
    else{
      Serial.println(i);
      display.println(i);
    }
  }
  display.display();
}
