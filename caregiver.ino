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
#define RED_LED_PIN 7

// I2C
#define MY_I2C_ADDRESS 0x08

volatile bool alertActive = false;
volatile byte lastCommand = 0;

unsigned long lastBuzzerToggle = 0;
bool buzzerOn = false;

const unsigned long BEEP_ON = 150UL;
const unsigned long BEEP_OFF = 150UL;

void onReceive(int numBytes) {
  while (Wire.available()) {
    lastCommand = Wire.read();
  }
  alertActive = (lastCommand == 1);
}

void setup() {
  Serial.begin(9600);
  
  Serial.println("CAREGIVER UNIT RUNNING");

  Wire.begin(MY_I2C_ADDRESS);
  Wire.onReceive(onReceive);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);

  showOLED("MedAlert", "Caregiver Unit", "Monitoring...", "");

  delay(200); // FIX: I2C stability
}

void loop() {
  unsigned long now = millis();
  static bool oledUpdated = false;

  if (alertActive) {
    digitalWrite(RED_LED_PIN, HIGH);

    runBuzzerPattern(now, BEEP_ON, BEEP_OFF, 1000);

    if (!oledUpdated) {
      showOLED("!! ALERT !!",
               "Patient missed",
               "medication dose.",
               "Please check!");
      oledUpdated = true;
    }

  } else {
    if (buzzerOn) {
      noTone(BUZZER_PIN);
      buzzerOn = false;
    }

    digitalWrite(RED_LED_PIN, LOW);

    if (oledUpdated) {
      showOLED("MedAlert",
               "Caregiver Unit",
               "All clear.",
               "");
      oledUpdated = false;
    }
  }
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