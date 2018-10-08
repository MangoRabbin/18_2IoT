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
//Pir
int calibrationTime = 30;
long unsigned int lowIn;
long unsigned int pause = 10000;
long unsigned int indexNum = 0;
boolean lockLow = true;
boolean takeLowTime;
int PIRValue = 0;


int led_state, usbled_state;
bool dhtBool, dht_tBool, dht_hBool, cdsBool;
bool usbledBool, checkLedBool, checkUsbLedBool, checkPirBool;
unsigned long prevTime = 0;
unsigned long currentTime =0;

WiFiClient wifiClient; //클라이언트로 작동 // 온습도 센서 instance 선언
DHT dht(dhtPin, DHTTYPE);
float temperature, tempf, humidity;
int lightValue;
char pub_data[80]; //publishing 할때 사용

// WiFi
const char *wifi_ssid = "YM";
const char *wifi_password = "godisgood";


//MQTT
#define mqtt_broker "192.168.2.4"
#define mqtt_user "iot"
#define mqtt_pwd ".csee"
#define TOPIC_GREETINGS "Hello from nodeMCU of YM"

//PubSub
const char *mqtt_nodeName = "nodeMCU.21400670.ece30003";
const char *myTopic = "iot/21400670";
const char *sub_topicEce30003 = "iot/ece30003";
const char *pub_dht = "iot/21400670/sensor/dht22";
const char *pub_dht_t = "iot/21400670/sensor/dht22_t"; // 
const char *pub_dht_h = "iot/21400670/sensor/dht22_h"; // 
const char *pub_cds = "iot/21400670/sensor/cds"; // 
const char *pub_pir = "iot/21400670/sensor/pir"; // 
const char *sub_mynodemcu = "iot/21400670"; //

const char *pub_led = "iot/21400670/check/led";
const char *pub_usbled = "iot/21400670/check/usbled";
void callback(char *topic, byte *payload, unsigned int length) //Web 으로 부터 수신한 Message에 따른 동작 제어 함수
  {
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
      Serial.println(String(topic) + msgString);
      Serial.println("Payload: "+ msgString);
        //전송된 메시가 "led"이면 LED를 받을 때마다 켜고 끈다.(토글) 
      if (msgString == "light/led"){
         led_state = !led_state;
         digitalWrite(ledPin, led_state);
      }
      else if(msgString =="light/ledon"){
          digitalWrite(ledPin, LED_ON);
      }
      else if(msgString =="light/ledoff"){
          digitalWrite(ledPin, LED_OFF);
      }
      else if(msgString =="light/usbled"){
          usbled_state = !usbled_state;
          digitalWrite(relay, !usbled_state);
      }
      else if(msgString =="light/usbledon"){
          //digitalWrite(relay, RELAY_ON);
          usbledBool = true;
          prevTime = millis(); // check button click time.
      }
      else if(msgString =="light/usbledoff"){ 
          //digitalWrite(relay, RELAY_OFF);
          usbledBool = false;
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

    
}
PubSubClient mqttc(mqtt_broker, 1883, callback, wifiClient);

void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600); 
  delay(10);

  pinMode(ledPin, OUTPUT); // Initialize the LED_BUILTIN pin as an output 
  digitalWrite(ledPin, LED_OFF); //LED off
  pinMode(relay, OUTPUT); 
  digitalWrite(relay, RELAY_OFF);
  pinMode(cdsPin,INPUT);
  pinMode(pirPin, INPUT);
  InitWiFi();
  dhtBool = true;
  cdsBool = true;
  dht_tBool = false;
  dht_hBool = false;
  usbledBool = false;
  checkLedBool = false;
  checkUsbLedBool = false;
  checkPirBool = true;
  mqttc.setServer(mqtt_broker, 1883); 
  mqttc.setCallback(callback);
//Connection to MQTT broker
  if (mqttc.connect(mqtt_nodeName, mqtt_user, mqtt_pwd))
  {
      mqttc.publish(pub_dht, TOPIC_GREETINGS); 
      mqttc.publish(pub_cds, TOPIC_GREETINGS);
      mqttc.subscribe(sub_mynodemcu);//Subscribe 할 Topic 
      mqttc.subscribe(sub_topicEce30003);
  }
  dht.begin(); 
}

void loop() {
  // put your main code here, to run repeatedly:

  if(!wifiClient.connected()){
      reconnect(); 
  }
  Dht22Sensor();
  LightSensor();
  PIRSensor();
  USBLEDControl();
  //CheckLED();
  //CheckUSBLED();
  mqttc.loop();
  
}

void InitWiFi(){
  Serial.println();
  Serial.println("Connecting to WiFi..");
  // attempt to connect ot WiFi network
  WiFi.begin(wifi_ssid,wifi_password);
  while(WiFi.status() != WL_CONNECTED){
    delay(5);
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
    delay(5);
  }// end while
}// Network 접속 확인 함수
//void CheckLED(){
//    delay(100);
//    int ledPresentState = digitalRead(ledPin);
//    if(ledPresentState){
//      sprintf(pub_data,"%s","On");
//    }
//    else{
//      sprintf(pub_data,"%s","Off");
//    }
//    //Serial.println("pub_led");
//    mqttc.publish(pub_led,pub_data,1);
//}
//void CheckUSBLED(){
//
//    delay(100);
//    int usbledPresentState = digitalRead(relay);
//
//    if(!usbledPresentState){
//      sprintf(pub_data,"%s","On");
//    }
//    else{
//      sprintf(pub_data,"%s","Off");
//    }  
//    mqttc.publish(pub_usbled,pub_data,1);
//
//}
void Dht22Sensor(){
    humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  if(isnan(humidity) || isnan(temperature))
    Serial.print("Failed to read From DHT sensor!");
    String payload_dht;
    payload_dht = temperature;
    payload_dht += " ";
    payload_dht += humidity;
    payload_dht += " ";
    payload_dht += indexNum++;
    payload_dht.toCharArray(pub_data, (payload_dht.length() +1));
  if (dhtBool)
  {
    delay(10);
    mqttc.publish(pub_dht,pub_data, 1);
    dhtBool = false;
  }
  
  if (dht_tBool)
  {
    String payload_dht_t;
    payload_dht_t = temperature;
    payload_dht_t += " ";
    payload_dht_t += indexNum++;
    payload_dht_t.toCharArray(pub_data, (payload_dht_t.length() +1));
    delay(10);
    mqttc.publish(pub_dht_t, pub_data, 1);
    dht_tBool = false;
  }
    if (dht_hBool)
  {
    String payload_dht_h;
    payload_dht_h = humidity;
    payload_dht_h += " ";
    payload_dht_h += indexNum++;
    payload_dht_h.toCharArray(pub_data, (payload_dht_h.length() +1));
    delay(10);
    //sprintf(pub_data, "%3.1f", humidity);
    mqttc.publish(pub_dht_h, pub_data, 1);
    dht_hBool = false;
  }
}


void LightSensor(){
    lightValue = analogRead(cdsPin);
  if (cdsBool)
  {
    String payload_light;
    payload_light = lightValue;
    payload_light += " ";
    payload_light += indexNum++;
    payload_light.toCharArray(pub_data, (payload_light.length() +1));
    delay(10);
    mqttc.publish(pub_cds, pub_data, 1);
    cdsBool = false;
  }
}



void PIRSensor() {
  if((digitalRead(pirPin) ==HIGH) && checkPirBool){
        usbledBool = true;
        PIRValue = 1;
        //Serial.println("Motion detected");
        String payload_pir;
        payload_pir = "Detected ";
        payload_pir += indexNum++;
        payload_pir.toCharArray(pub_data, (payload_pir.length() +1));
        delay(50);
        //sprintf(pub_data, "%s", "Detected"); 
        mqttc.publish(pub_pir, pub_data, 1);
        prevTime = millis();
        checkPirBool = false;


  }
  if ( digitalRead(pirPin) == LOW){
     if((currentTime - prevTime >= 10000)&& !checkPirBool){
        PIRValue = 0;
        String payload_pir;
        payload_pir = "Undetected ";
        payload_pir += indexNum++;
        payload_pir.toCharArray(pub_data, (payload_pir.length() +1));
        //Serial.println("Motion ended");
        delay(50);
        mqttc.publish(pub_pir, pub_data, 1);
        checkPirBool = true;
      }
  }
  
}

void USBLEDControl()
{
    if(millis() < 10000){
      currentTime = 10001;
    }
    else{
      currentTime = millis();
    }
   // USBLED control
    if (currentTime - prevTime >= 10000){
      if (usbled_state){
       usbledBool = true;
     }
    else{
        if(PIRValue)
        {
          Serial.println("detected");
          usbledBool = true;
        }
         else
        {
          usbledBool = false;
          if(lightValue > 800){
            Serial.println("light high");
            usbledBool = false;
          }
          else if (lightValue < 400)
          {
            Serial.println("light low");
            usbledBool = true;
          }
          
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

  delay(10);
}
