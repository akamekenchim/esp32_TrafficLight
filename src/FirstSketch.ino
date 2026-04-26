unsigned long lastTime = 0;
const int GREEN_LIGHT = 27;
const int YELLOW_LIGHT = 26;
const int RED_LIGHT = 25;
const int SWITCH_PIN = 33;
typedef struct L{
  uint8_t pin;
  uint16_t duration;
  uint8_t blinkCount;
} Light;
uint8_t blinkCounter = 0;
uint8_t curLight = 0;
bool lastState = LOW;
Light tLights[] = { {GREEN_LIGHT, 3000, 1}, {YELLOW_LIGHT, 400, 3}, {RED_LIGHT, 3000, 1}};


void setup(){
  pinMode(GREEN_LIGHT, OUTPUT);
  pinMode(YELLOW_LIGHT, OUTPUT);
  pinMode(RED_LIGHT, OUTPUT);
  //pinMode(SWITCH_PIN, INPUT_PULLDOWN);
  digitalWrite(tLights[curLight].pin, HIGH);
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
    curLight = 2;
    digitalWrite(tLights[curLight].pin, HIGH);
    blinkCounter = 0;
  } 
  lastState = curState;
  if(curTime - lastTime >= tLights[curLight].duration){
    if(tLights[curLight].blinkCount > 1 && blinkCounter <= tLights[curLight].blinkCount * 2){
      toggle(curLight);
      lastTime = curTime;
      blinkCounter++;
    }
    else{
      digitalWrite(tLights[curLight].pin, LOW);
      curLight++;
      if(curLight > 2) curLight = 0;
      lastTime = curTime;
      digitalWrite(tLights[curLight].pin, HIGH);  
      blinkCounter = 0;
    } 
  }

}