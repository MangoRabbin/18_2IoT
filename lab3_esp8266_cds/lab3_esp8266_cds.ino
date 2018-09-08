#define LIGHT_PIN A0
int light_val;
void setup() {
  // put your setup code here, to run once:
  pinMode(LIGHT_PIN,INPUT);
  Serial.begin(9600);

}

void loop() {
  // put your main code here, to run repeatedly:
  light_val = analogRead(LIGHT_PIN);
  Serial.print("Light intensity = ");
  Serial.println(light_val);
  delay(1000);
}
