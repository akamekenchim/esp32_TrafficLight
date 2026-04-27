#define GREEN_LIGHT 27
#define YELLOW_LIGHT 26
#define RED_LIGHT 25
#define SWITCH_PIN 33
#define PED_GREEN 16
#define PED_RED 17
#define LDR_PIN 34
#define LIGHT_THRESHOLD 300
#define BUTTON_CHECK_INTERVAL 300
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
  digitalWrite(tLights[curLight].pin, HIGH);
  digitalWrite(PED_RED, HIGH);
  analogReadResolution(10);
}
void toggle(uint8_t index){
  if(digitalRead(tLights[index].pin) == HIGH){
    digitalWrite(tLights[index].pin, LOW);
  }
  else{
    digitalWrite(tLights[index].pin, HIGH);
  }
}
void loop(){
  uint16_t lightValue = prevLightValue;
  unsigned long curTime = millis();
  if(curTime - lastButtonCheck > BUTTON_CHECK_INTERVAL){
    bool curState = digitalRead(SWITCH_PIN);
    if(curState == HIGH && lastState == LOW && curLight != NIGHT_MODE){
      lastTime = curTime;
      digitalWrite(tLights[curLight].pin, LOW);
      curLight = RED;
      digitalWrite(tLights[curLight].pin, HIGH);
      blinkCounter = 0;
      digitalWrite(PED_RED, LOW);
      digitalWrite(PED_GREEN, HIGH);
    } 
    lastButtonCheck = curTime;
    lastState = curState;
  }
  if(curTime - lastLDRRead > LDR_CHECK_INTERVAL){
    lightValue = analogRead(LDR_PIN);
    lastLDRRead = curTime;
  }
  if(lightValue > LIGHT_THRESHOLD && prevLightValue <= LIGHT_THRESHOLD){
    digitalWrite(PED_RED, LOW);
    digitalWrite(PED_GREEN, HIGH);
    digitalWrite(tLights[curLight].pin, LOW);
    curLight = NIGHT_MODE;
    prevLightValue = lightValue;
  }
  else if(lightValue <= LIGHT_THRESHOLD && prevLightValue > LIGHT_THRESHOLD){
    digitalWrite(YELLOW_LIGHT, LOW);
    digitalWrite(GREEN_LIGHT, HIGH);
    digitalWrite(PED_RED, HIGH);
    digitalWrite(PED_GREEN, LOW);
    lastTime = curTime;
    curLight = GREEN;
    prevLightValue = lightValue;
  }
  if(curLight == NIGHT_MODE && curTime - lastTime >= tLights[YELLOW].duration){
    toggle(YELLOW);
    lastTime = curTime;
  }

  if(curTime - lastTime >= tLights[curLight].duration && curLight != NIGHT_MODE){
    lastTime = curTime;
    if(curLight != YELLOW) digitalWrite(tLights[curLight].pin, LOW);
    if(curLight == GREEN) curLight = YELLOW;
    else if(curLight == YELLOW && blinkCounter == 6){
      blinkCounter = 0;
      curLight = RED;
    }
    else if(curLight == RED) curLight = GREEN;
    switch(curLight){
      case RED:
        digitalWrite(RED_LIGHT, HIGH);
        break;
      case YELLOW:
        toggle(YELLOW);
        blinkCounter++;
        break;
      case GREEN:
        digitalWrite(GREEN_LIGHT, HIGH);
        break;
    }
    
    digitalWrite(PED_RED, (curLight == GREEN) || (curLight == YELLOW));
    digitalWrite(PED_GREEN, (curLight == RED));
  }
}