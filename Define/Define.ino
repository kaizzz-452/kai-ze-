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

// Global System Variables
bool isConfigMode = true;
int currentReminderIndex = 0;
bool isCompleted[4] = {false, false, false, false};
bool alertActive = false;
unsigned long lastBeepTime = 0;
int selectedPotValue = 0;
unsigned long activeInterval = 0; // Stores the selected interval from reminderfrequency()

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

// LDR Threshold
const int NORMAL_LDR_THRESHOLD = 900;

// Function Prototypes
unsigned long reminderfrequency(int knobValue);
void overduealert();
void showSetupInstructions();
void printReminderToSerial(int index);
void checkPeriodicBeep();
bool areAllCompleted();
void setLEDs(bool completed, bool alert, bool config, bool periodic);
void updateDisplayNumber(int number);

// =========================================================================
// SETUP
// =========================================================================
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

  showSetupInstructions();
}

// =========================================================================
// MAIN LOOP
// =========================================================================
void loop() {
  int potValue = analogRead(KNOB_PIN);
  int ldrValue = analogRead(LDR_PIN);

  bool isBluePressed = (digitalRead(BUTTON_K1) == LOW);
  bool isYellowPressed = (digitalRead(BUTTON_K2) == LOW);

  // -----------------------------------------------------------------------
  // PHASE 1: CONFIGURATION MODE (KNOB SETUP)
  // -----------------------------------------------------------------------
  if (isConfigMode) {
    updateDisplayNumber(potValue);
    setLEDs(false, false, true, false); // Blue LED ON in config

    if (isBluePressed) {
      isConfigMode = false;
      selectedPotValue = potValue;
      activeInterval = reminderfrequency(selectedPotValue); // Uses custom logic
      lastBeepTime = millis(); // Reset timer start

      tone(BUZZER_PIN, 1000);
      delay(200);
      noTone(BUZZER_PIN);

      Serial.println(F("\n=============================================="));
      Serial.print(F(">>> Confirmed Knob Value: "));
      Serial.println(selectedPotValue);
     
      Serial.print(F(">>> Selected Interval: "));
      Serial.print(activeInterval / 1000);
      Serial.println(F(" seconds"));
      Serial.println(F("==============================================\n"));

      updateDisplayNumber(displayCodes[currentReminderIndex]);
      printReminderToSerial(currentReminderIndex);
      delay(300);
    }
    return;
  }

  // -----------------------------------------------------------------------
  // PHASE 2: MAIN REMINDER OPERATIONS
  // -----------------------------------------------------------------------

  // 1. Check if LDR detects senior left (LDR >= 900)
  if (ldrValue >= NORMAL_LDR_THRESHOLD && !areAllCompleted()) {
    if (!alertActive) {
      alertActive = true;
      Serial.println(F("\n⚠️ [ALERT] Senior absent/left device!"));
      Serial.println(F("⚠️ To silence: Return (LDR < 900) AND press BLUE button (K1)."));
    }
  }

  // 2. Continuous Safety Alert Active State
  if (alertActive) {
    if (ldrValue < NORMAL_LDR_THRESHOLD && isBluePressed) {
      alertActive = false;
      noTone(BUZZER_PIN);
      lastBeepTime = millis(); // Reset timer upon return
     
      Serial.println(F("\n=============================================="));
      Serial.println(F(">>> Alert Cleared: Senior is back!"));
      Serial.println(F("=============================================="));
     
      printReminderToSerial(currentReminderIndex);
      delay(300);
      return; // Stops press from completing reminder simultaneously
    } else {
      tone(BUZZER_PIN, 1000);
      setLEDs(isCompleted[currentReminderIndex], true, false, false);
     
      if (isBluePressed && ldrValue >= NORMAL_LDR_THRESHOLD) {
        Serial.println(F("⚠️ Cannot clear alert! Cover LDR sensor first (LDR < 900)."));
        delay(300);
      }
      return;
    }
  }

  // 3. Handle Yellow Button (K2) - Cycle Reminders & Reset Timer
  if (isYellowPressed) {
    currentReminderIndex = (currentReminderIndex + 1) % 4;
    lastBeepTime = millis(); // Reset interval timer when navigating
   
    updateDisplayNumber(displayCodes[currentReminderIndex]);
    printReminderToSerial(currentReminderIndex);
    delay(300);
  }

  // 4. Handle Blue Button (K1) - Mark Completed
  if (isBluePressed) {
    if (!isCompleted[currentReminderIndex]) {
      isCompleted[currentReminderIndex] = true;
      lastBeepTime = millis(); // Reset timer on completion
     
      Serial.print(F(">>> Marked as COMPLETED: "));
      Serial.println(reminderTitles[currentReminderIndex]);

      if (areAllCompleted()) {
        Serial.println(F("\n🎉 VICTORY! All reminders completed!"));
        for (int i = 0; i < 5; i++) {
          tone(BUZZER_PIN, victoryMelody[i]);
          delay(150);
        }
        noTone(BUZZER_PIN);
      } else {
        for (int i = 0; i < 3; i++) {
          tone(BUZZER_PIN, completionMelody[i]);
          delay(120);
        }
        noTone(BUZZER_PIN);
      }
    } else {
      Serial.println(F(">>> Already marked completed."));
    }
    delay(300);
  }

  // 5. Update Status LEDs
  setLEDs(isCompleted[currentReminderIndex], false, false, false);

  // 6. Handle Periodic Reminder Sound
  checkPeriodicBeep();

  delay(50);
}

// =========================================================================
// HELPER FUNCTIONS
// =========================================================================

// Custom Interval Function
unsigned long reminderfrequency(int knobValue) {
  unsigned long remindertime;
  if (knobValue <= 400) {
    remindertime = 10000; // 10s
  } else if (knobValue > 800) {
    remindertime = 20000; // 20s
  } else {
    remindertime = 15000; // 15s
  }
  return remindertime;
}

// Overdue Alert Function
void overduealert() {
  digitalWrite(LED_RED, HIGH);

  do {
    tone(BUZZER_PIN, 800, 500);
    delay(500);
    tone(BUZZER_PIN, 600, 500);
    delay(500);
    tone(BUZZER_PIN, 400, 500);
    delay(500);
  } while (digitalRead(BUTTON_K1) == HIGH);

  digitalWrite(LED_RED, LOW);

  delay(300);
  while (digitalRead(BUTTON_K1) == LOW); // Wait for release

  digitalWrite(LED_GREEN, HIGH);
  delay(2000);
  digitalWrite(LED_GREEN, LOW);
}

void showSetupInstructions() {
  Serial.println(F("=========================================================="));
  Serial.println(F("   Senior Assistive System Setup Mode                     "));
  Serial.println(F("=========================================================="));
  Serial.println(F("Turn KNOB to set assistance level, then press BLUE (K1):"));
  Serial.println(F("  [ 0 - 400 ]   : Reminder every 10s"));
  Serial.println(F("  [ 401 - 800 ] : Reminder every 15s"));
  Serial.println(F("  [ 801 - 1023 ]: Reminder every 20s\n"));
}

void printReminderToSerial(int index) {
  Serial.println(F("\n------------------------------------------"));
  Serial.print(F("Current Selection: "));
 
  switch (index) {
    case 0: Serial.println(reminderTitles[0]); break;
    case 1: Serial.println(reminderTitles[1]); break;
    case 2: Serial.println(reminderTitles[2]); break;
    case 3: Serial.println(reminderTitles[3]); break;
  }
 
  Serial.println(F("BLUE (K1) = Complete | YELLOW (K2) = Next"));
  Serial.println(F("------------------------------------------"));
}

void checkPeriodicBeep() {
  if (activeInterval > 0 && !areAllCompleted()) {
    if (millis() - lastBeepTime >= activeInterval) {
      Serial.println(F("🔔 [REMINDER] Time to complete your active reminder!"));

      // Play 3s alert
      tone(BUZZER_PIN, 1000);
      setLEDs(isCompleted[currentReminderIndex], false, false, true);
      delay(3000);
     
      // Auto-stop alert without user intervention
      noTone(BUZZER_PIN);
      setLEDs(isCompleted[currentReminderIndex], false, false, false);
     
      // Reset baseline timer AFTER the 3s alert finishes
      lastBeepTime = millis();
    }
  }
}

bool areAllCompleted() {
  for (int i = 0; i < 4; i++) {
    if (!isCompleted[i]) return false;
  }
  return true;
}

void setLEDs(bool completed, bool alert, bool config, bool periodic) {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_BLUE, LOW);
  digitalWrite(LED_YELLOW, LOW);

  if (config) {
    digitalWrite(LED_BLUE, HIGH);
  } else if (alert || periodic) {
    digitalWrite(LED_RED, HIGH);
  } else if (completed) {
    digitalWrite(LED_GREEN, HIGH);
  } else {
    digitalWrite(LED_BLUE, HIGH);
    digitalWrite(LED_YELLOW, HIGH);
  }
}

// Direct Bit-Banging TM1637 Display Driver Function
void updateDisplayNumber(int number) {
  const uint8_t digitMap[10] = {
    0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f
  };

  uint8_t segments[4];
  segments[0] = digitMap[(number / 1000) % 10];
  segments[1] = digitMap[(number / 100) % 10];
  segments[2] = digitMap[(number / 10) % 10];
  segments[3] = digitMap[number % 10];

  auto writeByte = [](uint8_t byte) {
    for (uint8_t i = 0; i < 8; i++) {
      digitalWrite(DISPLAY_CLK, LOW);
      digitalWrite(DISPLAY_DIO, (byte & 0x01) ? HIGH : LOW);
      byte >>= 1;
      digitalWrite(DISPLAY_CLK, HIGH);
    }
    digitalWrite(DISPLAY_CLK, LOW);
    digitalWrite(DISPLAY_DIO, HIGH);
    digitalWrite(DISPLAY_CLK, HIGH);
  };

  digitalWrite(DISPLAY_CLK, HIGH);
  digitalWrite(DISPLAY_DIO, HIGH);
  digitalWrite(DISPLAY_DIO, LOW);
  writeByte(0x40);
  digitalWrite(DISPLAY_CLK, LOW);
  digitalWrite(DISPLAY_DIO, LOW);
  digitalWrite(DISPLAY_CLK, HIGH);
  digitalWrite(DISPLAY_DIO, HIGH);

  digitalWrite(DISPLAY_CLK, HIGH);
  digitalWrite(DISPLAY_DIO, HIGH);
  digitalWrite(DISPLAY_DIO, LOW);
  writeByte(0xc0);
  for (uint8_t i = 0; i < 4; i++) {
    writeByte(segments[i]);
  }
  digitalWrite(DISPLAY_CLK, LOW);
  digitalWrite(DISPLAY_DIO, LOW);
  digitalWrite(DISPLAY_CLK, HIGH);
  digitalWrite(DISPLAY_DIO, HIGH);

  digitalWrite(DISPLAY_CLK, HIGH);
  digitalWrite(DISPLAY_DIO, HIGH);
  digitalWrite(DISPLAY_DIO, LOW);
  writeByte(0x88 | 0x07);
  digitalWrite(DISPLAY_CLK, LOW);
  digitalWrite(DISPLAY_DIO, LOW);
  digitalWrite(DISPLAY_CLK, HIGH);
  digitalWrite(DISPLAY_DIO, HIGH);
}
