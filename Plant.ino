#include <Servo.h> 
#include <EEPROM.h>


int MAX_VALUE_LIGHT = 1000;
int MIN_VALUE_LIGHT = 0;
int MAX_VALUE_TEMP = 40;
int MIN_VALUE_TEMP = 20;
#define LIGHT_SENSOR A0
#define TEMPERATURE_SENSOR A1
#define SERVO 9
#define RELEASE 120
#define LOCKED 30
#define RED_COLOR 10
#define GREEN_COLOR 11
#define BLUE_COLOR 12
float temperature;
int fadeAmount = 1;
int light;
int timer;
Servo servo;
//temperature has priority
enum State{
  EVERYTHING_OK = 0,
  LOW_LIGHT = 1,
  HIGH_LIGHT = 2,
  LOW_TEMP = 4,
  HIGH_TEMP = 8,
  CRITICAL = 16
};

State state;


void setup() {
  // put your setup code here, to run once:
  state = EVERYTHING_OK;
  temperature = 0;
  light = 0;
  pinMode(LIGHT_SENSOR, INPUT);
  pinMode(TEMPERATURE_SENSOR, INPUT);
  servo.attach(SERVO);
  timer = 0;
  pinMode(RED_COLOR, INPUT);
  pinMode(GREEN_COLOR, INPUT);
  pinMode(BLUE_COLOR, INPUT);
  MIN_VALUE_LIGHT = EEPROM.read(0);
  MAX_VALUE_LIGHT = EEPROM.read(100);
  MIN_VALUE_TEMP = EEPROM.read(200);
  MAX_VALUE_TEMP = EEPROM.read(300);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  getMeditions();
  timer++;
  Serial.println(temperature);
  setState();
  state = EVERYTHING_OK;
  doActions();
}

void getMeditions(){
  light = readInput(LIGHT_SENSOR);
  temperature = getVoltage(TEMPERATURE_SENSOR);
  Serial.println(light);
  Serial.println(temperature);
}

void doActions(){

  if(state == LOW_TEMP){
    analogWrite(RED_COLOR,255);
    Serial.println("low temp");
    delay(30000);
  }
  else if(state == HIGH_TEMP){
      servo.write(RELEASE);
      int brightness = 5;

      for(int i = 0; i < 60; i++){
        analogWrite(RED_COLOR, 255);
        delay(250);
        analogWrite(RED_COLOR, 0);
        delay(250);
    }
  }
  else if(state == HIGH_LIGHT ){
    Serial.println("High light");
      int brightness = 5;

      for(int i = 0; i < 60; i++){
        analogWrite(GREEN_COLOR, 255);
        analogWrite(RED_COLOR, 255);
        delay(250);
        analogWrite(GREEN_COLOR, 0);
        analogWrite(RED_COLOR, 0);
        delay(250);
    }
  }
 
  else if(state == LOW_LIGHT){
    Serial.println("low light");
    analogWrite(GREEN_COLOR, 255);
    analogWrite(RED_COLOR, 255);
    delay(30000);
  }
  else{
    int bright = 1;
    for(int i = 0; i < 1500; i++){
      bright += fadeAmount;
        if(bright == 0 ||bright == 255)
        {
          fadeAmount = -fadeAmount; 
        }
        analogWrite(GREEN_COLOR,bright);
        delay(20);
    }
    delay(30000);
    servo.write(LOCKED);
          Serial.println("lock");
          
  }
}


void setState(){
  state = EVERYTHING_OK;
  if(light < MIN_VALUE_LIGHT){
    state = LOW_LIGHT;
  }
  else{
    if(light > MAX_VALUE_LIGHT){
      state = HIGH_LIGHT;
    }
    else{
      Serial.println("Temp ok");
    }
  }
  
  if(temperature < MIN_VALUE_TEMP){
      state = LOW_TEMP;
  }
  else{
    if(temperature > MAX_VALUE_TEMP){
       state = HIGH_TEMP;
    }
  }
}



int readInput(int pinInput){
  int sum = 0;
  for(int i=0;i<10;i++){
    sum += analogRead(pinInput);
    delay(100);
  }
  return sum/10;
}

float getVoltage(int pin)
{
  int lecture = readInput(pin);
  float voltage = lecture * 5.0;
  voltage /= 1024;
  return (voltage-0.5)*100;
}


