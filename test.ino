#define LED_GREEN 5
#define LED_RED 4
#define LED_BLUE 6
#define LED_YELLOW 7
#define KNOB_PIN A0



void setup() {
  // put your setup code here, to run once:
  pinMode(LED_RED,OUTPUT);   
  pinMode(LED_GREEN,OUTPUT); 
  pinMode(LED_BLUE,OUTPUT); 
  pinMode(LED_YELLOW,OUTPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:

}
