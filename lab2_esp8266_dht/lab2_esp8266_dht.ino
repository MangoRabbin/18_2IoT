#include "DHT.h"
#define D2 4 // GPIO 4
#define DHTPIN D2 //digital pin 
#define DHTTYPE DHT22 //dht module 
DHT dht(DHTPIN, DHTTYPE);
float humidity, temperature;
void setup() {
Serial.begin(9600);
dht.begin();
}
void loop() {
  delay(2000); // Wait for a few seconds between measurements
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
if(!isnan(humidity) && !isnan(temperature)){ 
  Serial.print("온도 = "); 
  Serial.print(temperature);
  Serial.print("습도 = ");
  Serial.println(humidity);
} else{
Serial.println("Failed to read from DHT sensor");
return; }
}
