#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>
#include <DHT.h>

#define D0 16 //LED
#define D1 5 //USBLED
#define DHT_PIN 4 //온습도
#define D3 0 
#define D4 2 
#define D5 14 
#define D6 12 
#define D7 13 
#define D8 15
#define LEDOFF LOW
#define LEDON HIGH
#define DHTTYPE DHT22
#define RELAY_OFF HIGH
#define RELAY_ON LOW


DHT dht(DHT_PIN, DHTTYPE);
float temperature, humidity;

const char* ssid = "YM";
const char* password = "godisgood";
const char* mymqtt_client_id = "iot_nodeMCU_21400670";

char* topic = "iot";
char* server ="203.252.106.154";
char message_buff[100]; // initialize storage buffer

WiFiClient wifiClient;
int ledPin = D0; // GPIO 16
int RELAY_PIN = D1; //GPIO 5
int relay_state = RELAY_OFF;


void callback(char* topic, byte* payload, unsigned int length)
{
  int i = 0;
  Serial.println("Message arrived: topic: " + String(topic));
  Serial.println("Length: "+ String(length,DEC));//create character buffer with ending null terminator (string)
  
 for(i=0; i<length; i++)
 {
  message_buff[i] = payload[i];
 }
 message_buff[i]= '\0';
 String msgString = String(message_buff);
 Serial.println("Payload: "+ msgString);
 int LED_state = digitalRead(ledPin);
 int RELAY_state = digitalRead(RELAY_PIN);
 //전송된 메시가 "led"이면 LED를 받을 때마다 켜고 끈다.(토글) 
 if (msgString == "ledon"){
 digitalWrite(ledPin, LEDON);
 LED_state = LEDON;
 Serial.println("Switching LED");
  }
 else if (msgString == "ledoff"){
 digitalWrite(ledPin, LEDOFF);
 LED_state = LEDOFF;
 Serial.println("Switching LED");
  }
  if(msgString == "usbled"){
    relay_state = RELAY_ON;
    digitalWrite(RELAY_PIN,!relay_state)
  }
  if(msgString == "usbledon")
 if(msgString == "dht22"){
      humidity = dht.readHumidity();
      temperature = dht.readTemperature();
    while((isnan(humidity) || isnan(temperature))){
      delay(1);
      Serial.print(".");
      humidity = dht.readHumidity();
      temperature = dht.readTemperature();
    }
    Serial.println(humidity);
    Serial.println(temperature);
  }
}

PubSubClient client(server, 1883, callback, wifiClient);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(10);
  pinMode(ledPin, OUTPUT); // Initialize the LED_BUILTIN pin as an output
  digitalWrite(ledPin, LEDOFF); //LED off
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, RELAY_OFF);
  Serial.println();
  Serial.println();
  Serial.println("Connecting to");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.println(".");
  }
  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.println(WiFi.localIP());

//  if(client.connect(mymqtt_client_id)){
//    client.publish(topic,"Publishing message form my nodeMCU");
//    client.subscribe(topic);
//  }
  if(client.connect(mymqtt_clinent_id,mqtt_user, mqtt_pwd)){
    client.publish(mqtt_topic,"Publishing message from my nodeMCU");
    client.subscribe(mqtt_topic);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  client.loop();
}
