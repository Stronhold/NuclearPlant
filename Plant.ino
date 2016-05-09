#include <Servo.h> 

#define MAX_VALUE_LIGHT 1000
#define MIN_VALUE_LIGHT 0
#define MAX_VALUE_TEMP 40
#define MIN_VALUE_TEMP 20
#define LIGHT_SENSOR A0
#define TEMPERATURE_SENSOR 1
#define SERVO 9
#define RELEASE 120
#define LOCKED 30
#define TIMER_TO_WAIT 28000
float temperature;
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
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  getMeditions();
  timer++;
  Serial.println(temperature);
  setState();
  doActions();
  delay(TIMER_TO_WAIT);
}

void getMeditions(){
  light = readInput(LIGHT_SENSOR);
  temperature = getVoltage(TEMPERATURE_SENSOR);
  Serial.println(light);
  Serial.println(temperature);
}

void doActions(){
  if(state == CRITICAL ){
      servo.write(RELEASE);
      Serial.println("crit");
  }
  else if(state == HIGH_LIGHT ){
    Serial.println("High light");
  }
  else if(state == LOW_TEMP){
    Serial.println("low temp");
  }
  else if(state == HIGH_TEMP){
      servo.write(RELEASE);
            Serial.println("high temp");
  }
  else if(state == LOW_LIGHT){
    Serial.println("low light");
  }
  else{
    servo.write(LOCKED);
          Serial.println("lock");
  }
  // no comprobamos mas, sino esta OK
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
    setCritical();
    if(state != CRITICAL)
      state = LOW_TEMP;
    
  }
  else{
    if(temperature > MAX_VALUE_TEMP){
       setCritical();
       if(state != CRITICAL)
        state = HIGH_TEMP;
    }
  }
}

void setCritical(){
  if(state > 0){
    state = CRITICAL;
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


