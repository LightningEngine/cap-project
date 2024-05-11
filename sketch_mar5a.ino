#include <DHT.h>
#include <DHT_U.h>
#include <Arduino.h>
#include <ESP32Servo.h>
#include <WiFi.h>    
#include <HTTPClient.h>
#include <UrlEncode.h>
#include <MQ135.h>

#define DHTPIN 32
#define DHTPIN2 33
#define DHTTYPE DHT11
const int photosensorpin = 35;
int servopin = 18;
int fanpin = 19;
int gaspin = 34;
float photosensor;

bool DHTbroke = false;
bool DHT2broke = false;
bool Photobroke = false;
bool MQbroke = false;
bool TempERR = true;
bool PhotoERR = true;
bool CO2ERR = false;

DHT dht(DHTPIN, DHTTYPE);
DHT dht2(DHTPIN2, DHTTYPE);
Servo Servo1;

const char* ssid = "OPPO Reno6";
const char* password = "12345678";

String phoneNumber = "+201270964567";
String apiKey = "8420344";

void sendMessage(String message){

  // Data to send with HTTP POST
  String url = "https://api.callmebot.com/whatsapp.php?phone=" + phoneNumber + "&apikey=" + apiKey + "&text=" + urlEncode(message);    
  HTTPClient http;
  http.begin(url);

  // Specify content-type header
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  
  // Send HTTP POST request
  int httpResponseCode = http.POST(url);
  if (httpResponseCode == 200){
    Serial.println("Message sent successfully");
  }
  else{
    Serial.println("Error sending the message");
    Serial.print("HTTP response code: ");
    Serial.println(httpResponseCode);
  }

  // Free resources
  http.end();
}

void setup() {
  Serial.begin(115200);
  Serial.println("DHT11 test!");
  pinMode(fanpin, OUTPUT);
  pinMode(servopin, OUTPUT);
  dht.begin();
  dht2.begin();
  Servo1.attach(servopin);
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

}

void loop() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  float humidity2 = dht2.readHumidity();
  float temperature2 = dht2.readTemperature();
  photosensor = analogRead(photosensorpin); //Get the value of light
  MQ135 gasSensor = MQ135(gaspin);
  float ppm = gasSensor.getResistance();


  if ((isnan(humidity) || isnan(temperature))&& DHTbroke == false) {
    Serial.println("Failed to read from DHT sensor!"); //Check for DHT error
    DHTbroke == true;
    sendMessage("Failed to read from DHT sensor!");
    return;
  }
  else{
    DHTbroke == false;
  }
  if ((isnan(humidity2) || isnan(temperature2))&& DHT2broke == false) {
    Serial.println("Failed to read from DHT sensor 2!"); //Check for DHT error
    DHT2broke == true;
    sendMessage("Failed to read from DHT sensor 2!");
    return;
  }
  else{
    DHT2broke == false;
  }
  
  if (isnan(photosensor) && Photobroke == false) {
    Serial.println("Photoresistor Error!");
    sendMessage("Photoresistor Error!");
    return;
  }
  else{
    Photobroke == false;
  }

  if (isnan(ppm) && MQbroke == false) {
    Serial.println("Failed to read from MQ-135 sensor");
    sendMessage("Failed to read from MQ-135 sensor");
    return;
  }
  else{
    MQbroke == false;
  }

  if(temperature2 < temperature-3 && TempERR == true){
    Serial.println("The temperature inside the building is succesfully less than 3*C than the outside");
    sendMessage("The temperature inside the building is succesfully less than 3*C than the outside");
    TempERR = false;
  }
  else if (temperature2 > temperature-3 && TempERR == false){
    Serial.println("The temperature inside the building not less than 3*C than the outside");
    sendMessage("The temperature inside the building not less than 3*C than the outside");
    TempERR = true;
  }

  if(photosensor>1000 && PhotoERR == true){
    Servo1.write(180);
    Serial.println("It is bright outside");
    sendMessage("It is bright outside");
    PhotoERR = false;
    delay(500);  
  }
  else if (photosensor<1000 && PhotoERR == false){
    Servo1.write(0);
    Serial.println("It is dark outside");
    sendMessage("It is dark outside");
    PhotoERR = true;
    delay(500);  
  }
  if(ppm>700 && CO2ERR == true){
    digitalWrite(fanpin, LOW);
    Serial.println("CO2 concentration is above 700ppm");
    sendMessage("CO2 concentration is above 700ppm");
    CO2ERR = false;
  }
  else if (ppm<700 && CO2ERR == false){
    digitalWrite(fanpin, HIGH);
    Serial.println("CO2 concentration is below 700ppm");
    sendMessage("CO2 concentration is below 700ppm");
    CO2ERR = true;
  }
  
  Serial.print ("ppm: ");
  Serial.println (ppm);

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" *C");
  Serial.print("Temperature2: ");
  Serial.print(temperature2);
  Serial.println(" *C");
  Serial.print("photo sensor value: ");
  Serial.println(photosensor);
  Serial.println(" ");

  delay(500);
}
