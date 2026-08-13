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


//Function Prototypes
int reminderfrequency(int knobValue); //Ji Wunn
void overduealert(); //Ji Wunn

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

// Arrays
const String reminderTitles[4] = {
  "0001 = Check Healthy365 app",
  "0002 = Check Singpass for any notifications",
  "0003 = Check for any outgoing bills notifications from gov.sg",
  "0004 = Check for whether you have taken your medicine"
};


const int displayCodes[4] = {1, 2, 3, 4};
const int completionMelody[3] = {523, 659, 784};
const int victoryMelody[5] = {523, 659, 784, 880, 1047};





void loop() {
  
}

int reminderfrequency(int knobValue) //Function, Ji Wunn
{
    int remindertime;
    if (knobValue <= 400) //knob value less than or equal 400?
    {
        remindertime = 20000;
    }
    else
        if (knobValue > 800) //knob value greater than 800?
        {
            remindertime = 60000;
        }
        else
        {
            remindertime = 40000;
        }
    return remindertime; 
} 

void overduealert() //Function, Ji Wunn
{
    digitalWrite(LED_RED, HIGH);

    do  {
    tone(BUZZER_PIN, 800, 500); //Digital Buzzer
    tone(BUZZER_PIN, 600, 500);
    tone(BUZZER_PIN, 400, 500);
    } while (digitalRead(BUTTON_K1) == 1);

    digitalWrite(LED_RED, LOW);

    delay(300);
    while(digitalRead(BUTTON_K1) == 0);

    digitalWrite(LED_GREEN, HIGH);
    delay(2000);
    digitalWrite(LED_GREEN, LOW);
}
