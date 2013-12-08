// http://jeelabs.net/
//http://opensource.org/licenses/mit-license.php
//12/7/13 Todd Miller GreenhouseTX8

#include <JeeLib.h>
#include <avr/sleep.h>
Port tmp (2);
int value;
int radioIsOn=1;
int finalValue=0;
MilliTimer readoutTimer, aliveTimer;

void setup() {
  tmp.mode2(INPUT);
  rf12_initialize(1, RF12_433MHZ, 100);
  rf12_easyInit(15); // every 10 seconds send out pkg
  Serial.begin(57600);
  delay(50);//wait for power to settle
}

void loop() {
  value = tmp.anaRead();
  set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_mode();
  if (radioIsOn && rf12_easyPoll() == 0) {
    rf12_sleep(0); // turn the radio off
    radioIsOn = 0;
  }
  if (readoutTimer.poll(1000)) {
    calcTemp();
  }
  byte sending = rf12_easySend(&finalValue, sizeof value);

  if (aliveTimer.poll(60000)){
    sending = rf12_easySend(0, 0); // always returns 1
    Serial.println(F("Sending 'Alive' Message"));
  }
  if (sending) {
    // make sure the radio is on again
    if (!radioIsOn)
      rf12_sleep(-1); // turn the radio back on
    radioIsOn = 1;
  }
}
void calcTemp(){
  for (int i=0; i <= 3; i++){
    value = value + tmp.anaRead();
    delay(2);
  } 

  value = (value/5);

  float voltage = value * 3.3;
  voltage /= 1024.0; 
  float temperatureC = (voltage - 0.5) * 100 ; 
  float temperatureF = (temperatureC * 9.0 / 5.0) + 32.0;
  finalValue=temperatureF;
  Serial.print(F("Actual temp = "));
  Serial.println(finalValue);
}


