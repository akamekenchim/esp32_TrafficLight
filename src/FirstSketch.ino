#include "soc/gpio_struct.h"
#include "soc/gpio_reg.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
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
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

unsigned long lastTime = 0; //dùng cho đèn LED 
unsigned long lastLDRRead = 0; // dùng để set interval cho cảm biến ánh sáng LDR 
unsigned long lastButtonCheck = 0; // dùng để set interval cho nút bấm người đi bộ - debounce
unsigned long lastClock = 0; // dùng cho màn hình SSD1306

uint16_t prevLightValue = 0;
uint8_t blinkCounter = 0;
bool lastState = LOW;

uint16_t currentTimer = 100;
typedef struct L{
  uint8_t pin;
  uint16_t duration;
  uint8_t blinkCount;
} Light;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

Light tLights[] = { {GREEN_LIGHT, 4000, 1}, {YELLOW_LIGHT, 300, 6}, {RED_LIGHT, 4000, 1}};
typedef enum TS {GREEN, YELLOW, RED, NIGHT_MODE} TrafficState;
TrafficState curLight = GREEN;

void setup(){
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  currentTimer = tLights[GREEN].duration;
  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(46,21);
  display.printf("%02d", currentTimer/1000);
  display.display();
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

inline bool readPin(uint8_t pin);
void toggle(uint8_t index);
inline void turnLight(uint8_t pin, bool state);
inline void displayTime(uint16_t thoigian);
inline void clearScreen();

void loop(){
  uint16_t lightValue = prevLightValue; // biến kiểm tra mức ánh sáng đọc được của LDR trước đó
  unsigned long curTime = millis();
  if(curTime - lastButtonCheck > BUTTON_CHECK_INTERVAL){
    bool curState = readPin(SWITCH_PIN);
    if(curState == HIGH && lastState == LOW && curLight != NIGHT_MODE){
      lastTime = curTime;
      turnLight(tLights[curLight].pin, LOW);
      curLight = RED;
      currentTimer = tLights[RED].duration;
      displayTime(currentTimer/1000);
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
    clearScreen();
  }
  else if(lightValue <= LIGHT_THRESHOLD && prevLightValue > LIGHT_THRESHOLD){
    turnLight(YELLOW_LIGHT, LOW);
    turnLight(GREEN_LIGHT, HIGH);
    currentTimer = tLights[GREEN].duration;
    displayTime(currentTimer/1000);
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
    else if(curLight == YELLOW && blinkCounter >= tLights[YELLOW].blinkCount){
      blinkCounter = 0;
      curLight = RED;
    }
    else if(curLight == RED) curLight = GREEN;
    if(curLight != YELLOW) {
        currentTimer = tLights[curLight].duration;
    }
    switch(curLight){
      case RED:
        turnLight(RED_LIGHT, HIGH); // bật đèn đỏ
        turnLight(YELLOW_LIGHT, LOW);
        turnLight(GREEN_LIGHT, LOW);
        break;
      case YELLOW:
        turnLight(RED_LIGHT, LOW);
        turnLight(GREEN_LIGHT, LOW);
        toggle(YELLOW);
        blinkCounter++;
        break;
      case GREEN:
        turnLight(RED_LIGHT, LOW); // tắt đèn đỏ
        turnLight(YELLOW_LIGHT, LOW);
        turnLight(GREEN_LIGHT, HIGH); // bật đèn xanh
        break;
    }
    
    turnLight(PED_RED, (curLight == GREEN) || (curLight == YELLOW));
    turnLight(PED_GREEN, (curLight == RED));
  }
  if((curLight != NIGHT_MODE && curLight != YELLOW) && ((curTime - lastClock >= 1000) || lastClock == 0)){
    lastClock = curTime;
    displayTime(currentTimer / 1000);
    currentTimer -= 1000;
    if(currentTimer < 0) currentTimer = 0;
  }
}

inline bool readPin(uint8_t pin){
  // GPIO.in trả về chuỗi 32 bit thể hiện trạng thái của 32 thanh ghi trên ESP32 - ứng với 32 chân.
  // (1 << pin) : sẽ gán bit thứ (pin) là 1, còn lại là 0. Nghĩa là nếu chuỗi bit kia có bit vị trí pin là 1, kết quả
  // GPIO.in & (1 << pin) sẽ lớn hơn 0.
  if(pin < 32) return (GPIO.in & (1 << pin)); 
  else{
    return (GPIO.in1.val & (1 << (pin-32))); //in1.val là các vị trí lớn hơn 32.
  }
}
void toggle(uint8_t index){
  if(readPin(tLights[index].pin) == HIGH){
    GPIO.out_w1tc = (1 << tLights[index].pin); //clear bit vị trí pin
  }
  else{
    GPIO.out_w1ts = (1 << tLights[index].pin); // bật bit vị trí pin
  }

}
inline void turnLight(uint8_t pin, bool State){
  if(State == HIGH){
    GPIO.out_w1ts = (1 << pin);
  }
  else{
    GPIO.out_w1tc = (1 << pin);
  }
}
inline void displayTime(uint16_t thoigian){
  display.clearDisplay();
    display.setCursor(46,21);
    display.printf("%02d", thoigian);
    display.display();
}
inline void clearScreen(){
  display.clearDisplay();
  display.display();
}