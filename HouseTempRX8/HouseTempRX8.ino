// http://jeelabs.net/
// http://opensource.org/licenses/mit-license.php
// digiRead() & digiWrite() - ports D pin
// anaRead() & digiWrite2() - Ports A pin 
// mode sets Digital pin mode2 sets ports Analog pin
// 12/9/13 Todd Miller based on GreenhouseRX8

#include <JeeLib.h>
#include <PortsLCD.h>
#include <avr/sleep.h> 

Port ledOne (1);
Port ledTwo (2);
Port ledThree (3);
PortI2C myI2C (4);
LiquidCrystalI2C lcd (myI2C);
int temp;
int data;
byte y =0;
byte r = 0;
byte x = 0;
byte threshold= 34;
byte threshold2= 95;
int backlightSwitch;
int tmpHigh;
int tmpLow;
byte tmp=0;
int pkg1=0;
int pkg2=0;
int pkg3=0;
int yCount = 0;

typedef struct {
byte light;
byte wind :1;
byte humi :7;
int temp :10;
byte lobat :1;
} Payload;

Payload measurement;

unsigned long interval = 150000;
unsigned long previousMillis;

#define NOTE_G5  784
#define NOTE_A3  220
int melody2[] = {
  NOTE_G5, NOTE_A3};
int noteDurations[] = {
  6, 6 };

void setup () {
  Serial.begin(57600);
  Serial.print("Starting up....");
  lcd.begin(20, 4);
  homeScreen();
  ledOne.mode(OUTPUT); // digital pin for green led
  ledTwo.mode(INPUT); // digital pin for backlight switch
  ledTwo.mode2(OUTPUT); // analog pin for yellow led
  ledThree.mode(OUTPUT); //digital pin for speaker
  ledThree.mode2(OUTPUT); // analog pin for red led

  rf12_initialize('R', RF12_433MHZ, 75);
  ledOne.digiWrite(1);
  ledTwo.digiWrite(1);
  ledTwo.digiWrite2(0);
  ledThree.digiWrite2(0);
}

void loop () {
  set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_mode();
  unsigned long currentMillis = millis();
  backlightSwitch=ledTwo.digiRead();
  if (backlightSwitch == LOW)
    lcd.noBacklight();
  else
    lcd.backlight();

  if (currentMillis-previousMillis >interval){
    yellowLed();
    previousMillis=currentMillis;
  }
if (rf12_crc == 0) {
    activityLed(1);
}
  if (rf12_recvDone() && rf12_crc == 0 && rf12_len == sizeof (Payload)) {
    previousMillis=currentMillis;
    Serial.println("Rcvd data");
    if (tmp==0)
    {
      tmp=1;
      tmpLow=rf12_data[0];
      tmpHigh=rf12_data[0];
    }
    
    measurement= *(Payload*) rf12_data;

Serial.print("Room ");
Serial.print(measurement.light, DEC);
lcd.setCursor(2,1);
lcd.print(measurement.light);
Serial.print (" ");
Serial.print(measurement.wind, DEC);
lcd.setCursor(8,1);
lcd.print(measurement.wind);
Serial.print (" ");
Serial.print(measurement.humi, DEC);
lcd.setCursor(2,2);
lcd.print(measurement.humi);
Serial.print (" ");
Serial.print(measurement.temp, DEC);
lcd.setCursor(8, 2);
lcd.print(measurement.temp);
Serial.print (" ");
Serial.print(measurement.lobat, DEC);
Serial.println();

    int* data = (int*) rf12_data;
    pkg1++;
    if (pkg1>999){
      pkg1=0;
      pkg2++;
      if(pkg2>999){
        pkg2=0;
        pkg3++;
      }
    }

    if(rf12_data[0] <tmpLow && rf12_data[0]>tmpLow-20)
      tmpLow=rf12_data[0];

    if(rf12_data[0] >tmpHigh)
      tmpHigh=rf12_data[0];

   
    // process data here

    else {
      greenLed();
     
      lcd.setCursor(12, 3);
      lcd.print(pkg1);
      //lcd.print((int) rf12_hdr);
    }

    if (RF12_WANTS_ACK)
      rf12_sendStart(RF12_ACK_REPLY,0,0);
  }
}

void greenLed()
{
  ledOne.digiWrite(1);
  ledTwo.digiWrite2(0);
  ledThree.digiWrite2(0); 
  if (y==1 || r==1){
    homeScreen(); 
    y=0; 
    r=0;
    x=0;
  }
}

void yellowLed()
{
  ledOne.digiWrite(0);
  ledTwo.digiWrite2(1);
  ledThree.digiWrite2(0);
  if(y==0 || r==1)
    yCount++;
    
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Check sensor!"));
  lcd.setCursor(0, 1);
  lcd.print(F("Battery may need"));
  lcd.setCursor(0, 2);
  lcd.print(F("recharging."));
  y=1;
  r=0;
  x=0;
}

void redLed()
{
  ledOne.digiWrite(0);
  ledTwo.digiWrite2(0);
  ledThree.digiWrite2(1);
  if(r==0 || y==1){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Check temp!"));
    lcd.setCursor(0, 1);
    lcd.print(F("Current Temp:"));
    lcd.setCursor(14, 1);
    lcd.print(rf12_data[0]);
    lcd.setCursor(17, 1);
    lcd.print(F("F"));
    r=1;
    y=0;
  }
  
   if (x==0){
      x++;
      for (int thisNote = 0; thisNote < 2; thisNote++) {
        int noteDuration = 1000/noteDurations[thisNote];
        tone(6, melody2[thisNote],noteDuration);//pin 6 or port 3 digital pin
        int pauseBetweenNotes = noteDuration * 1.30;
        delay(pauseBetweenNotes);
        noTone(6);        
      }
  }
}

void homeScreen()
{
 lcd.clear();
lcd.print("Miller House");
lcd.setCursor(0,1);
lcd.print("L ");
lcd.setCursor(6,1);
lcd.print("W ");
lcd.setCursor(0,2);
lcd.print("H ");
lcd.setCursor(6,2);
lcd.print("T ");
lcd.setCursor(0,3);
lcd.print("Pkgs Rcvd: ");
}


static void activityLed (byte on) {
#ifdef LED_PIN
pinMode(LED_PIN, OUTPUT);
digitalWrite(LED_PIN, !on);
#endif

}

