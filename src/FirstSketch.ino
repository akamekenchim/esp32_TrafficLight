unsigned long lastTime = 0;
const int GREEN_LIGHT = 27;
const int YELLOW_LIGHT = 26;
const int RED_LIGHT = 25;
const int SWITCH_PIN = 33;
const int PED_GREEN = 16;
const int PED_RED = 17;
typedef struct L{
  uint8_t pin;
  uint16_t duration;
  uint8_t blinkCount;
} Light;
uint8_t blinkCounter = 0;
bool lastState = LOW;
Light tLights[] = { {GREEN_LIGHT, 3000, 1}, {YELLOW_LIGHT, 400, 3}, {RED_LIGHT, 3000, 1}};
typedef enum TS {GREEN, YELLOW, RED} TrafficState;
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
  unsigned long curTime = millis();
  bool curState = digitalRead(SWITCH_PIN);
  if(curState == HIGH && lastState == LOW){
    lastTime = curTime;
    digitalWrite(tLights[curLight].pin, LOW);
    curLight = RED;
    digitalWrite(tLights[curLight].pin, HIGH);
    blinkCounter = 0;
    digitalWrite(PED_RED, LOW);
    digitalWrite(PED_GREEN, HIGH);
  } 
  lastState = curState;

  if(curTime - lastTime >= tLights[curLight].duration){
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