#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>


#define D0 16 // LED
#define D1 5 // Relay
#define D2 4 // DHT22
#define D3 0 // PIR
#define D4 2 
#define D5 14 
#define D6 12 
#define D7 13 
#define D8 15


#define DHTTYPE DHT22
#define LED_ON HIGH
#define LED_OFF LOW
#define RELAY_ON LOW
#define RELAY_OFF HIGH


int ledPin = D0;
int relay = D1;
int dhtPin = D2;
int cdsPin = A0;
int pirPin = D3;
int led_state, usbled_state;
bool dhtBool, dht_tBool, dht_hBool, cdsBool, usbledBool, pirBool;
unsigned long prevTime =0;
unsigned long currentTime =0;

//Pir
int calibrationTime = 30;
long unsigned int lowIn;
long unsigned int pause = 5000;
boolean lockLow = true;
boolean takeLowTime;
int PIRValue = 0;

// WiFi
const char *wifi_ssid = "YM";
const char *wifi_password = "godisgood";


//MQTT
#define mqtt_broker "192.168.2.4"
#define mqtt_user "iot"
#define mqtt_pwd ".csee"
#define TOPIC_GREETINGS "Hello from nodeMCU of YM"


const char *mqtt_nodeName = "nodeMCU.21400670";
const char *myTopic = "iot/21400670";
const char *sub_topicEce30003 = "iot/ece30003";
const char *pub_dht = "iot/21400670/sensor/dht22";
const char *pub_dht_t = "iot/21400670/sensor/dht22_t"; // 
const char *pub_dht_h = "iot/21400670/sensor/dht22_h"; // 
const char *pub_cds = "iot/21400670/sensor/cds"; // 
const char *pub_pir = "iot/21400670/sensor/pir"; // 
const char *sub_mynodemcu = "iot/21400670"; // 

WiFiClient wifiClient; //클라이언트로 작동 // 온습도 센서 instance 선언
DHT dht(dhtPin, DHTTYPE);
float temperature, tempf, humidity;
int lightValue;
char pub_data[80]; //publishing 할때 사용
void callback(char *topic, byte *payload, unsigned int length) //Web 으로 부터 수신한 Message에 따른 동작 제어 함수
  {
    if( topic != myTopic || topic != sub_topicEce30003)
    {
      return ;
    }
    char message_buff[100]; //initialise storage buffer
    String msgString; 
    int i = 0;
    Serial.println("Message arrived: topic: " + String(topic)); 
    Serial.println("Length: "+ String(length,DEC));
    //create character buffer with ending null terminator (string) 
    for(i=0; i<length; i++)
    {
      message_buff[i] = payload[i]; 
    }
    message_buff[i]= '\0';
    msgString = String(message_buff); 
    Serial.println("Payload: "+ msgString);
    led_state = digitalRead(ledPin); 
    usbled_state = digitalRead(relay);
      //전송된 메시가 "led"이면 LED를 받을 때마다 켜고 끈다.(토글) 
    if (msgString == "light/led"){
       digitalWrite(ledPin, !led_state);
       Serial.println("Switching LED"); 
    }
    else if(msgString =="light/ledon"){
        digitalWrite(ledPin, LED_ON);
    }
    else if(msgString =="light/ledoff"){
        digitalWrite(ledPin, LED_OFF); 
    }
    else if(msgString =="light/usbled"){
        digitalWrite(relay, !usbled_state);
    }
    else if(msgString =="light/usbledon"){
        digitalWrite(relay, RELAY_ON);
        prevTime = millis(); // check button click time.
    }
    else if(msgString =="light/usbledoff"){ 
        digitalWrite(relay, RELAY_OFF);
        prevTime = millis(); // check button click time.
     }
    else if(msgString =="sensor/dht22"){
        dhtBool = true;
    }
    else if(msgString =="sensor/dht22_t"){
        dht_tBool = true;
    }
    else if(msgString =="sensor/dht22_h"){
        dht_hBool = true;
    }
    else if(msgString =="sensor/cds"){
        cdsBool = true;
    }
    else if(msgString =="sensor/pir"){
        pirBool = true;
    }
}
 /* end of callback() for MQTT */
PubSubClient mqttc(mqtt_broker, 1883, callback, wifiClient);

void setup() 
{
  Serial.begin(9600); 
  delay(10);

  pinMode(ledPin, OUTPUT); // Initialize the LED_BUILTIN pin as an output 
  digitalWrite(ledPin, LED_OFF); //LED off
  pinMode(relay, OUTPUT); 
  digitalWrite(relay, RELAY_OFF);
  pinMode(cdsPin,INPUT);
  pinMode(pirPin, INPUT);
  InitWiFi();
  dhtBool = false;
  cdsBool = false;
  dht_tBool = false;
  dht_hBool = false;
  usbledBool = false;
  usbledState = false;
  mqttc.setServer(mqtt_broker, 1883); 
  mqttc.setCallback(callback);
//Connection to MQTT broker
  if (mqttc.connect(mqtt_nodeName, mqtt_user, mqtt_pwd))
  {
      mqttc.publish(pub_dht, TOPIC_GREETINGS); 
      mqttc.publish(pub_cds, TOPIC_GREETINGS);
      mqttc.subscribe(sub_mynodemcu);//Subscribe 할 Topic 
  }
  dht.begin(); 
}

void loop() 
{ 
  if(!wifiClient.connected()){
      reconnect(); 
  }
  delay(5); // wait a few seconds between measurements 
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
//화씨로 온도 측정
  tempf = dht.readTemperature(true);
  if(isnan(humidity) || isnan(temperature) || isnan(tempf)) 
     Serial.println("Failed to read from DHT sensor!");
     //Serial.println(temperature); 
     //Serial.println(humidity);
  String payload_dht;
//  payload_dht += "Humidity: ";

//  payload_dht += ", ";

//  payload_dht += "Temperature: ";
    payload_dht = temperature;
    payload_dht += " ";
    payload_dht += humidity;
//  payload_dht += "'C";
//  payload_dht += "}";
  payload_dht.toCharArray(pub_data, (payload_dht.length() + 1)); 
  if( dhtBool ){
    mqttc.publish(pub_dht, pub_data, 1);
    dhtBool = false;
  }
  if (dht_tBool){
    Serial.print("true");
    sprintf(pub_data, "%3.1f", temperature); 
    mqttc.publish(pub_dht_t, pub_data, 1);
    dht_tBool = false;
  }
  if(dht_hBool){
    Serial.print("true");
    sprintf(pub_data, "%3.1f", humidity); 
    mqttc.publish(pub_dht_h, pub_data, 1);
    dht_hBool = false;
  }
  

  lightValue = analogRead(cdsPin);
//  String payload_light = "{";
//  payload_light += "Light : ";
//  payload_light += lightValue;
//  payload_light += "}";
//  payload_light.toCharArray(pub_data, (payload_light.length() + 1));
    if (cdsBool){
      Serial.print("true");
      sprintf(pub_data, "%d", lightValue);
      Serial.print(lightValue);
      mqttc.publish(pub_cds,pub_data,1);
      cdsBool = false;
    }
    
    currentTime = millis(); // check current time
    if (currentTime - prevTime >= 10000){
      if(usbled_state){
        usbledBool = true;
//        digitalWrite(relay, RELAY_ON);
      }
      else 
        if(PIRValue){
          Serial.println("pirvalue high");
          usbledBool = true;
//        digitalWrite(relay, RELAY_ON);
        }
        else
        {
          usbledBool = false;
     //  Serial.println("pirvalue low");
//        digitalWrite(relay, RELAY_OFF);
          if(lightValue > 800)
          {
            Serial.println("800");
            usbledBool = false;
//          digitalWrite(relay,RELAY_ON);
          }
          else if (lightValue < 400)
          {
            Serial.println("400");
            usbledBool = true;
//          digitalWrite(relay, RELAY_ON);
          }
      }
    }
      
    
    if(usbledBool)
    {
      digitalWrite(relay, RELAY_ON);
    }
    else
    {
      digitalWrite(relay, RELAY_OFF);
    }

    //Match the request for USBLED
  PIRSensor();
  delay(10); // delay

  mqttc.loop();
} // end of loop()

void InitWiFi(){
  Serial.println();
  Serial.println("Connecting to WiFi..");
  // attempt to connect ot WiFi network
  WiFi.begin(wifi_ssid,wifi_password);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected to AP:");
  //접속 정보를 출력
  Serial.println(WiFi.localIP());
}

void reconnect()
{
  int status;

  while (!wifiClient.connected()){
    status = WiFi.status();
    if(status != WL_CONNECTED){
      WiFi.begin(wifi_ssid, wifi_password);
      while(WiFi.status() != WL_CONNECTED){
        delay(5);
        Serial.println(".");
      }
      Serial.println("Connected to AP again");
    }
    delay(50);
  }// end while
}// Network 접속 확인 함수

void PIRSensor() {
  if(digitalRead(pirPin) ==HIGH){
    if(lockLow){
      PIRValue = 1;
      lockLow = false;
      Serial.println("Motion detected");
      sprintf(pub_data, "%d", PIRValue); 
      mqttc.publish(pub_pir, pub_data, 1);
      delay(5);
    }
  }
  if ( digitalRead(pirPin) == LOW){
    if(takeLowTime){
      lowIn =millis();
      takeLowTime = false;      
    }
    if( !lockLow && millis() - lowIn > pause){
      PIRValue = 0;
      lockLow = true;
      Serial.println("Motion ended");
      sprintf(pub_data, "%d", PIRValue); 
      mqttc.publish(pub_pir, pub_data, 1);
      delay(5);
    }
  }
}
