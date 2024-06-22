#include <DHT.h>
#include <DHT_U.h>
#include <Wire.h>

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <WiFi.h>
#include <PubSubClient.h>

//WIFI STUFF
char ssid[] = "Plaksha-Events";
char pass[] = "Welcome@2024";

//MQTT STUFF
char *ip= "test.mosquitto.org";
int port = 1883;
char *topic = "yts2024/wise/grp12";

WiFiClient wifiClient;
PubSubClient client(wifiClient);

// #include <Servo.h>                                                                                            // could have to change as we may not use this

#define Width 128
#define Height 64
#define Reset -1
#define Address 0x3C

#define MQ135 35
#define LDR 33
#define LED 14
#define DHTPin 13
#define HallEffectPin 34

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 62, &Wire,Reset);
DHT dht = DHT(DHTPin,DHT22);

// Servo myservo;  // create servo object to control a servo                                                           //May have to change

// volatile byte revolutions;

// int pos = 0;    // variable to store the servo position

float v = 5.0;
float r2 = 10.0;
float r0 = 74.0;
float b = -2.769034857;
float a = 116.6020682;

int x = 0;
int count = 0;
unsigned long start = 0;
unsigned long timeold = 0;
int revolutions = 0;

bool check;
String HallEffectStr;

int time_for_hallEffectSensor_to_break = 5;

void setup() {
  Serial.begin(9600);
  pinMode(HallEffectPin, INPUT);
  dht.begin();

  SSD1306_Setup();

  pinMode(MQ135, INPUT);
  pinMode(LDR,INPUT);
  pinMode(LED,OUTPUT);
  digitalWrite(LED, LOW);
  
  WiFi.begin(ssid, pass);
  Serial.begin(9600);
  client.setServer(ip,port);
  client.connect("group12ESP32Client");

  while(WiFi.status() != WL_CONNECTED){
    Serial.println("WIFI DICONNECTED");
  }
  Serial.println("WIFI CONNECTED");
  
  while (!client.connected()){
    Serial.println("MQTT DISCONNECTED");
    client.connect("group12ESP32Client");
  }
  Serial.println("MQTT CONNECTED");
  Serial.println(WiFi.localIP());
  
  // pinMode(hall_eff, INPUT);
  // attachInterrupt(hall_eff, magnet_detect, RISING);//Initialize the intterrupt pin (Arduino digital pin 2)
  // revolutions = 0;
  // rpm = 0;
  // timeold = 0;
  // start = millis();

  // myservo.attach(9);                                                                                                   //could have to change 
}

void loop() {
  while(WiFi.status() != WL_CONNECTED){
    Serial.println("WIFI DICONNECTED");
  }
  while (!client.connected()){
      Serial.println("MQTT DISCONNECTED");
      client.connect("group12ESP32Client");
      check = false;
  }
  if(client.connected()&& check==false){
    Serial.println("MQTT CONNECTED");
    Serial.println(WiFi.localIP());
    check = true;
  }
  printOnDisplay();
  delay(500);

  if(analogRead(LDR)<1000){
    digitalWrite(LED, HIGH);
    Serial.println("LED is HIGH");
    client.publish(topic, String("LED is HIGH").c_str());
  }
  else{
    digitalWrite(LED, LOW);
    Serial.println("LED is LOW");
    client.publish(topic, String("LED is LOW").c_str());
  }
  // delay(500);
  
}
long findRPM(){
  unsigned long TimeInnitially = millis()/1000;
  // int SameTimeCount = 0;
  int BeforeTime =millis();
  // int timeRepeated = (TimeInnitially + time_for_hallEffectSensor_to_break) -(BeforeTime/1000); // finaltime -secondlastfinaltime == 0
  // Serial.println(timeRightNow + "    " + millis()/1000);
  while((millis()/1000)<=(TimeInnitially + time_for_hallEffectSensor_to_break)){ // the rhs defines a set time until the function will return a -1/null
    if( ((millis()/1000) - (BeforeTime/1000)) != 0){
      String to_be_printed = ("magnet needs to be scanned before "+ String((TimeInnitially + time_for_hallEffectSensor_to_break)-(BeforeTime/1000))+" seconds");
      Serial.println(to_be_printed);
      client.publish(topic, to_be_printed.c_str());
      BeforeTime = millis();
    }
    // if(timeRepeated -((TimeInnitially + time_for_hallEffectSensor_to_break)-(BeforeTime/1000))==0){
    //   Serial.print("you have ");
    //   Serial.print((timeRTimeInnitiallyightNow + time_for_hallEffectSensor_to_break)-(BeforeTime/1000));
    //   Serial.println( " seconds left");
    //   BeforeTime = millis();
    //   timeRepeated = (TimeInnitially + time_for_hallEffectSensor_to_break)-(BeforeTime/1000);
    // }
    x = magnet_detect(HallEffectPin);
    if(x==0){
      if(revolutions == 1){
        unsigned long Initial_time_for_magnet_for_second_time = millis()/1000;
        bool count_for_second_time = true;
        if(((millis()/1000) - Initial_time_for_magnet_for_second_time) >1 && !count_for_second_time){
          client.publish(topic,String("waiting for magnet for the second time").c_str());
          Serial.println("waiting for magnet for the second time");
          count_for_second_time = false;
        }
        TimeInnitially = millis()/1000;
        BeforeTime = millis();
      }
      if( ((millis()/1000) - (BeforeTime/1000)) != 0 && revolutions == 1){
      String to_be_printed = ("magnet needs to be rescanned before "+ String((TimeInnitially + time_for_hallEffectSensor_to_break)-(BeforeTime/1000))+" seconds");
      Serial.println(to_be_printed);
      client.publish(topic, to_be_printed.c_str());
      BeforeTime = millis();
      }
      if(revolutions==2){
        long RPMValue = calculateRPM();
        return RPMValue;
      }
    }
  }
  return -1;
}
long calculateRPM() {
  long rpm = (60000/(millis() - timeold));
  timeold = millis();
  revolutions = 0;
  return rpm;
}
int magnet_detect(int pin){
  int a = digitalRead(pin);
  // Serial.println("before if statement");
  if (a == 1) {
    // Serial.println("in if statement a=1 we dont want");
  }
  else {
    // Serial.println("in else statement a=0");
    unsigned long Initial_time_for_magnet_in_static = millis()/1000;
    revolutions = revolutions + 1;
    // Serial.println("Magnet present");
    bool count = false;
    while(digitalRead(HallEffectPin)==0){
      if(((millis()/1000)-Initial_time_for_magnet_in_static) > 1 && !count){
        Serial.println("magnet is stuck");
        client.publish(topic, String("magnet is stuck").c_str());
        count = true;
      }
      // Serial.println("in while loop a=0, magnet always present");
    }
    if(revolutions == 1){
      timeold = millis();
    }
    // Serial.println(millis());
  }
  return a;
}
void SSD1306_Setup(){
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
void printOnDisplay(){
  display.clearDisplay();
  display.setCursor(0,0);
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  double v2 = analogRead(MQ135) * (v / 4095.0);  
  double r1 = (r2 * (v - v2)) / v2;
  double ratio = r1 / r0;
  double ppm = a*pow(ratio, b);

  String humStr = "humidity: "+ String(hum);
  display.print(humStr);
  // display.print("Humidity: ");
  // display.print(hum);
  client.publish(topic, humStr.c_str());
  Serial.println(humStr);
  // Serial.print("Humidity: ");
  // Serial.println(hum);
  display.println(" g/m^3");

  String tempStr = "Temperature: "+String(temp);
  display.print(tempStr);
  // display.print("Temperature: ");
  // display.print(temp);
  client.publish(topic, tempStr.c_str());
  Serial.println(tempStr);
  // Serial.print("Temperature: ");
  // Serial.println(temp);
  display.println(" C");

  String ppmStr = "MQ135 value: "+String(ppm);
  display.println(ppmStr);
  // display.print("MQ135 value: ");
  // display.println(ppm);
  client.publish(topic,ppmStr.c_str());
  Serial.println(ppmStr);
  // Serial.print("MQ135 value: ");
  // Serial.println(ppm);
  
  String LDRStr = "LDR: "+String(analogRead(LDR));
  Serial.println(LDRStr);
  display.println(LDRStr);
  client.publish(topic, LDRStr.c_str());

  long RPM_Calculated = findRPM();
  if(RPM_Calculated == -1){
    HallEffectStr = "RPM: "+String("magnet took too long");
  }
  else{
    HallEffectStr = "RPM: "+String(RPM_Calculated);
  }
  // display.print("RPM: ");
  display.println(HallEffectStr);
  client.publish(topic, HallEffectStr.c_str());
  // Serial.print("RPM: ");
  Serial.println(HallEffectStr);
  
  //blank line
  display.println("-----------------");
  client.publish(topic,String("-------------------------").c_str());
  Serial.println("-----------------");

  display.display();
}

