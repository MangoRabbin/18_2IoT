#include "DHT.h"
#define D2 4 // GPIO 4
#define DHTPIN D2 //digital pin 
#define DHTTYPE DHT22 //dht module 
#define LIGHT_PIN A0

DHT dht(DHTPIN, DHTTYPE);
float humidity, temperature;
int light_val ;
void setup() {
Serial.begin(9600);
pinMode(LIGHT_PIN,INPUT);
dht.begin();
}
void loop() {
  delay(2000); // Wait for a few seconds between measurements
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  light_val = analogRead(LIGHT_PIN);
if(!isnan(humidity) && !isnan(temperature) && !isnan(light_val)){ 
  Serial.print("온도 = "); 
  Serial.print(temperature);
  Serial.print("  습도 = ");
  Serial.print(humidity);
  Serial.print("  LUX = ");
  Serial.println(light_val);
  
} else{
Serial.println("Failed to read from DHT sensor");
return; }
}
