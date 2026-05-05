#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pins
#define BUZZER_PIN 9
#define BUTTON_PIN 2
#define GREEN_LED_PIN 7

// I2C
#define CAREGIVER_ADDR 0x08

// Timing
const unsigned long REMINDER_INTERVAL = 30000UL;
const unsigned long ACK_WINDOW = 15000UL;
const unsigned long BUZZER_BEEP_ON = 300UL;
const unsigned long BUZZER_BEEP_OFF = 700UL;
const unsigned long ESCALATION_BEEP_ON = 100UL;
const unsigned long ESCALATION_BEEP_OFF = 100UL;
const unsigned long LED_CONFIRM_DURATION = 3000UL;
const unsigned long DEBOUNCE_DELAY = 50UL;

// Medications
const char* medications[] = {
  "Metformin",
  "Amlodipine",
  "Atorvastatin"
};
const int MED_COUNT = 3;
int currentMedIndex = 0;

// States
enum SystemState {
  STATE_IDLE,
  STATE_REMINDER,
  STATE_ESCALATION,
  STATE_CONFIRMED
};

SystemState currentState = STATE_IDLE;

// Timing vars
unsigned long lastReminderTime = 0;
unsigned long reminderStartTime = 0;
unsigned long lastBuzzerToggle = 0;
unsigned long confirmStartTime = 0;

bool buzzerOn = false;

// Button
volatile bool buttonPressed = false;
unsigned long lastDebounceTime = 0;

// Interrupt
void handleButtonPress() {
  buttonPressed = true;
}

void setup() {
  Serial.begin(9600);

  Serial.println("PATIENT UNIT RUNNING");
  
  Wire.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonPress, RISING);

  showOLED("MedAlert", "Patient Unit", "Ready", "");
  lastReminderTime = millis();

  delay(200); // FIX: I2C stability
}

void loop() {
  unsigned long now = millis();

  if (buttonPressed) {
    buttonPressed = false;

    if ((now - lastDebounceTime) > DEBOUNCE_DELAY) {
      lastDebounceTime = now;

      if (currentState == STATE_REMINDER || currentState == STATE_ESCALATION) {
        handleAcknowledgment(now);
      }
    }
  }

  switch (currentState) {

    case STATE_IDLE:
      if ((now - lastReminderTime) >= REMINDER_INTERVAL) {
        startReminder(now);
      }
      break;

    case STATE_REMINDER:
      if ((now - reminderStartTime) >= ACK_WINDOW) {
        startEscalation(now);
      } else {
        runBuzzerPattern(now, BUZZER_BEEP_ON, BUZZER_BEEP_OFF, 440);
      }
      break;

    case STATE_ESCALATION:
      runBuzzerPattern(now, ESCALATION_BEEP_ON, ESCALATION_BEEP_OFF, 880);
      break;

    case STATE_CONFIRMED:
      if ((now - confirmStartTime) >= LED_CONFIRM_DURATION) {
        digitalWrite(GREEN_LED_PIN, LOW);
        currentState = STATE_IDLE;
        showOLED("MedAlert", "Next dose in:", "30 seconds", "");
      }
      break;
  }
}

void startReminder(unsigned long now) {
  reminderStartTime = now;
  lastReminderTime = now;

  noTone(BUZZER_PIN); // FIX
  buzzerOn = false;

  currentState = STATE_REMINDER;

  showOLED("TIME TO TAKE:",
           medications[currentMedIndex],
           "Press button",
           "to confirm");
}

void startEscalation(unsigned long now) {
  noTone(BUZZER_PIN); // FIX
  buzzerOn = false;

  currentState = STATE_ESCALATION;

  showOLED("!! MISSED DOSE !!",
           medications[currentMedIndex],
           "Alerting",
           "caregiver...");

  Wire.beginTransmission(CAREGIVER_ADDR);
  Wire.write(1);
  Wire.endTransmission();
}

void handleAcknowledgment(unsigned long now) {
  noTone(BUZZER_PIN);
  buzzerOn = false;

  if (currentState == STATE_ESCALATION) {
    Wire.beginTransmission(CAREGIVER_ADDR);
    Wire.write(0);
    Wire.endTransmission();
  }

  currentMedIndex = (currentMedIndex + 1) % MED_COUNT;

  digitalWrite(GREEN_LED_PIN, HIGH);
  confirmStartTime = now;
  currentState = STATE_CONFIRMED;

  showOLED("Dose Confirmed!",
           medications[(currentMedIndex + MED_COUNT - 1) % MED_COUNT],
           "Thank you.",
           "");
}

void runBuzzerPattern(unsigned long now,
                      unsigned long onDuration,
                      unsigned long offDuration,
                      int frequency) {
  unsigned long elapsed = now - lastBuzzerToggle;

  if (buzzerOn && elapsed >= onDuration) {
    noTone(BUZZER_PIN);
    buzzerOn = false;
    lastBuzzerToggle = now;
  } else if (!buzzerOn && elapsed >= offDuration) {
    tone(BUZZER_PIN, frequency);
    buzzerOn = true;
    lastBuzzerToggle = now;
  }
}

void showOLED(const char* l1, const char* l2,
              const char* l3, const char* l4) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(l1);
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);
  display.setCursor(0, 14); display.println(l2);
  display.setCursor(0, 28); display.println(l3);
  display.setCursor(0, 42); display.println(l4);
  display.display();
}