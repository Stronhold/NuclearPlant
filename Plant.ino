#include <Servo.h> 
#include <EEPROM.h>
#include <SPI.h>
#include <HttpClient.h>
#include <WiFi.h>
#include <aJSON.h>

char ssid[] = "MCU";
char pass[] = "";   // your network password

// Name of the server we want to connect to
const char kHostname[] = "json.internetdelascosas.es";
// Prefix of the path to use
const char path_prefixTemp[] = "/arduino/add.php?device_id=125&data_name=temp&data_value=";
const char path_prefixLight[] = "/arduino/add.php?device_id=125&data_name=light&data_value=";
// Variable of the max size expected to store the complete path after adding
char kPath[200] = "";

int MAX_VALUE_LIGHT = 1000;
int MIN_VALUE_LIGHT = 0;
int MAX_VALUE_TEMP = 40;
int MIN_VALUE_TEMP = 20;
#define LIGHT_SENSOR A0
#define TEMPERATURE_SENSOR A1
#define SERVO 9
#define RELEASE 120
#define LOCKED 30
#define RED_COLOR 2
#define GREEN_COLOR 3
#define BLUE_COLOR 8
#define BUZZER 5
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
  Serial.println(MIN_VALUE_LIGHT);
  Serial.println(MAX_VALUE_LIGHT);
  Serial.println(MIN_VALUE_TEMP);
  pinMode(BUZZER, OUTPUT);   // sets the pin as output
  
  Serial.begin(9600);

  connectToWiFi();
}

void loop() {
  // put your main code here, to run repeatedly:
  getMeditions();
  timer++;
  Serial.println(temperature);
  setState();
  state = EVERYTHING_OK;
  doActions();
  connectToWiFi();
  //analogWrite(BUZZER,128);
  delay(1000);
  digitalWrite(BUZZER, LOW);
  doPost();
}

void doPost(){
  int err;
  Serial.println("ENVIAMOS DATOS");
  WiFiClient c;
  HttpClient http(c);
  Serial.println(temperature);
  Serial.println(light);
  //sprintf(kPath,"%s%f",path_prefixTemp,temperature);
  //err = http.get(kHostname, kPath);
  //http.stop();
  sprintf(kPath,"%s%d",path_prefixLight,light);
  err = http.get(kHostname, kPath);
  http.stop();
}

void getMeditions(){
  light = readInput(LIGHT_SENSOR);
  temperature = getVoltage(TEMPERATURE_SENSOR);
  //Serial.println(light);
  //Serial.println(temperature);
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
    for(int i = 0; i < 30000; i++){
      bright += fadeAmount;
      //Serial.println(bright);
        if(bright == 0 ||bright == 255)
        {
          fadeAmount = -fadeAmount; 
        }
        analogWrite(GREEN_COLOR,bright);
        delay(1);
    }
    servo.write(LOCKED);
          Serial.println("lock");
  }
}

void connectToWiFi(){
  int status = WiFi.status();
  // attempt to connect to Wifi network:
  while ( WiFi.status() != WL_CONNECTED) { 
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:    
    status = WiFi.begin(ssid);
    // wait 10 seconds for connection:
    delay(5000);
  } 
  Serial.println("connected");
  // you're connected now, so print out the status:
  printWifiStatus();
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
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


