// Pin Definitions
#define LED_RED 4
#define LED_GREEN 5
#define LED_BLUE 6
#define LED_YELLOW 7
#define BUTTON_K1 8   // Blue Button
#define BUTTON_K2 9   // Yellow Button
#define KNOB_PIN A0
#define LDR_PIN A2
#define BUZZER_PIN 3
#define DISPLAY_CLK 10
#define DISPLAY_DIO 11




void setup() {
  Serial.begin(9600);

  pinMode(LED_RED, OUTPUT);   
  pinMode(LED_GREEN, OUTPUT); 
  pinMode(LED_BLUE, OUTPUT); 
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(DISPLAY_CLK, OUTPUT);
  pinMode(DISPLAY_DIO, OUTPUT);
  
  pinMode(BUTTON_K1, INPUT_PULLUP); // Blue
  pinMode(BUTTON_K2, INPUT_PULLUP); // Yellow
  pinMode(KNOB_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);


}


void loop() {
  
}