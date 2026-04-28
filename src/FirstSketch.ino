#include "soc/gpio_struct.h"
#include "soc/gpio_reg.h"
#define GREEN_LIGHT 27
#define YELLOW_LIGHT 26
#define RED_LIGHT 25
#define SWITCH_PIN 33
#define PED_GREEN 16
#define PED_RED 17
#define LDR_PIN 34
#define LIGHT_THRESHOLD 300
#define BUTTON_CHECK_INTERVAL 80
#define LDR_CHECK_INTERVAL 1500

unsigned long lastTime = 0;
unsigned long lastLDRRead = 0;
unsigned long lastButtonCheck = 0;

typedef struct L{
  uint8_t pin;
  uint16_t duration;
  uint8_t blinkCount;
} Light;
uint16_t prevLightValue = 0;
uint8_t blinkCounter = 0;
bool lastState = LOW;
Light tLights[] = { {GREEN_LIGHT, 2000, 1}, {YELLOW_LIGHT, 300, 3}, {RED_LIGHT, 2000, 1}};
typedef enum TS {GREEN, YELLOW, RED, NIGHT_MODE} TrafficState;
TrafficState curLight = GREEN;

void setup(){
  pinMode(GREEN_LIGHT, OUTPUT);
  pinMode(YELLOW_LIGHT, OUTPUT);
  pinMode(RED_LIGHT, OUTPUT);
  pinMode(PED_RED, OUTPUT);
  pinMode(PED_GREEN, OUTPUT);
  pinMode(SWITCH_PIN, INPUT_PULLDOWN);
  GPIO.out_w1ts = (1 << GREEN_LIGHT); // turnLight(GREEN_LIGHT, HIGH);
  GPIO.out_w1ts = (1 << PED_RED); // turnLight(PED_RED, HIGH);
  analogReadResolution(10);
}

inline bool readPin(uint8_t pin){
  uint32_t current = GPIO.out;
  current = ((current >> pin) & 1);
  return current > 0 ? HIGH : LOW;
}
void toggle(uint8_t index){
  if(readPin(tLights[index].pin) == HIGH){
    GPIO.out_w1tc = (1 << tLights[index].pin);
  }
  else{
    GPIO.out_w1ts = (1 << tLights[index].pin);
  }
}
inline void turnLight(uint8_t pin, bool state){
  if(state == HIGH){
    GPIO.out_w1ts = (1 << pin);
  }
  else{
    GPIO.out_w1tc = (1 << pin);
  }
}

void loop(){
  uint16_t lightValue = prevLightValue;
  unsigned long curTime = millis();
  if(curTime - lastButtonCheck > BUTTON_CHECK_INTERVAL){
    bool curState = digitalRead(SWITCH_PIN);
    if(curState == HIGH && lastState == LOW && curLight != NIGHT_MODE){
      lastTime = curTime;
      turnLight(tLights[curLight].pin, LOW);
      curLight = RED;
      turnLight(tLights[curLight].pin, HIGH);
      blinkCounter = 0;
      turnLight(PED_RED, LOW);
      turnLight(PED_GREEN, HIGH);
    } 
    lastButtonCheck = curTime;
    lastState = curState;
  }
  if(curTime - lastLDRRead > LDR_CHECK_INTERVAL){
    lightValue = analogRead(LDR_PIN);
    lastLDRRead = curTime;
  }
  if(lightValue > LIGHT_THRESHOLD && prevLightValue <= LIGHT_THRESHOLD){
    turnLight(PED_RED, LOW);
    turnLight(PED_GREEN, HIGH);
    turnLight(tLights[curLight].pin, LOW);
    curLight = NIGHT_MODE;
    prevLightValue = lightValue;
  }
  else if(lightValue <= LIGHT_THRESHOLD && prevLightValue > LIGHT_THRESHOLD){
    turnLight(YELLOW_LIGHT, LOW);
    turnLight(GREEN_LIGHT, HIGH);
    turnLight(PED_RED, HIGH);
    turnLight(PED_GREEN, LOW);
    lastTime = curTime;
    curLight = GREEN;
    prevLightValue = lightValue;
  }
  if(curLight == NIGHT_MODE && curTime - lastTime >= tLights[YELLOW].duration){
    toggle(YELLOW);
    lastTime = curTime;
  }

  if(curLight != NIGHT_MODE && curTime - lastTime >= tLights[curLight].duration){
    lastTime = curTime;
    if(curLight != YELLOW) turnLight(tLights[curLight].pin, LOW);
    if(curLight == GREEN) curLight = YELLOW;
    else if(curLight == YELLOW && blinkCounter == 6){
      blinkCounter = 0;
      curLight = RED;
    }
    else if(curLight == RED) curLight = GREEN;
    switch(curLight){
      case RED:
        turnLight(RED_LIGHT, HIGH); // bật đèn đỏ
        break;
      case YELLOW:
        turnLight(RED_LIGHT, LOW);
        turnLight(GREEN_LIGHT, LOW);
        toggle(YELLOW);
        blinkCounter++;
        break;
      case GREEN:
        turnLight(GREEN_LIGHT, HIGH); // bật đèn xanh
        break;
    }
    
    turnLight(PED_RED, (curLight == GREEN) || (curLight == YELLOW));
    turnLight(PED_GREEN, (curLight == RED));
  }
}