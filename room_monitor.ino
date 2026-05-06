/*
 *
 *  Room Comfort Monitor with Auto Fan Control
 *  
 *  Hardware used:
 *    - Arduino Uno
 *    - DHT22 temperature & humidity sensor (D2)
 *    - SSD1306 OLED display using I2C (A4=SDA, A5=SCL)
 *    - Servo motor for simulating fan speed (D9)
 *    - RGB LED comfort indicator (R=D10, G=D11)
 *
 *  Simple overview:
 *    The DHT22 is polled every 2 seconds. The reading is used
 *    to place the room into one of 5 comfort zones. Each zone
 *    drives a servo angle (fan speed), a LED colour and an
 *    OLED status message simultaneously.
 *
 * 
 */

#include <Wire.h>               //I2C bus for OLED
#include <Adafruit_GFX.h>       //OLED graphics
#include <Adafruit_SSD1306.h>   //SSD1306 OLED driver included
#include <DHT.h>                //DHT22 sensor library
#include <Servo.h>              //Servo library included for fan simulation

// defining pins
#define DHTPIN        2    
#define DHTTYPE       DHT22
#define SERVO_PIN     9    // Servo wire (has PWM)
#define LED_RED_PIN   10   // RGB LED channels
#define LED_GREEN_PIN 11  

// configuring the OLED display
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1      
#define OLED_ADDRESS  0x3C   

// setting the temperature thresholds in celcius ───────────────────────────────

#define TEMP_COOL     24.0    // Below this means COOL zone
#define TEMP_MEDLOW   26.0    // Above this means MED-LOW zone
#define TEMP_MED      28.0    // Above this means MED zone
#define TEMP_MEDHIGH  30.0    // Above this means MED-HIGH zone
#define TEMP_HOT      33.0    // Above this means HOT zone

//Servo angles to represent fan speed increasing and decreasing
// 0° = stopped, 180° = full speed

#define FAN_OFF       0
#define FAN_MEDLOW    45
#define FAN_MED       90
#define FAN_MEDHIGH   135
#define FAN_FULL      180

//sensor polling interval

#define READ_INTERVAL_MS 2000

//object instances
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Servo fanServo;

//Comfort zone enum 

enum ComfortZone { COOL, MEDLOW, MED, MEDHIGH, HOT };

//timing tracker
unsigned long lastReadTime = 0;

//forward declarations
ComfortZone determineZone(float temp);
void setFanSpeed(ComfortZone zone);
void setComfortLED(ComfortZone zone);
void setRGB(bool red, bool green);
void updateDisplay(float temp, float hum, ComfortZone zone);
void displayError();
const char* zoneLabel(ComfortZone zone);

//processing begins
void setup() {
  Serial.begin(9600);
  Serial.println(F("Room Comfort Monitor starting..."));

  // to initialise DHT22 sensor
  dht.begin();

 //starts at 0
  fanServo.attach(SERVO_PIN);
  fanServo.write(FAN_OFF); 

  //setting up LED pins as outputs
  pinMode(LED_RED_PIN,   OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  setRGB(false, true);

  //initialise OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println(F("ERROR: SSD1306 OLED not found. Check wiring!"));
    while (true);  
  }

  //Startup screen to be shown while DHT22 stabilises
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(10, 10);
  display.println(F("Room Comfort"));
  display.println(F("  Monitor v2.0"));
  display.setCursor(10, 40);
  display.println(F("Warming up sensor..."));
  display.display();

  // DHT22 normaly needs seconds after power-on before first valid read
  delay(2000);
}


void loop() {
  unsigned long now = millis();

  // we only poll the sensor every READ_INTERVAL_MS milliseconds
  // this will prevent DHT22 overload and keeps readings stable
  if (now - lastReadTime >= READ_INTERVAL_MS) {
    lastReadTime = now;

    //Read sensor
    float humidity    = dht.readHumidity();
    float temperature = dht.readTemperature();  // Celsius

    // isnan() method catches failed readings
    if (isnan(humidity) || isnan(temperature)) {
      Serial.println(F("DHT22 read failed — check connections."));
      displayError();
      return;
    }

 
    ComfortZone zone = determineZone(temperature);

    //depending on temp zon se derive all three output systems
    setFanSpeed(zone);              //move servo to matching angle
    setComfortLED(zone);            //set RGB LED colour
    updateDisplay(temperature, humidity, zone);  // Then OLED refresh

    // ── Serial debug output ──────────────────────────────────
    Serial.print(F("Temp: "));     Serial.print(temperature); Serial.print(F("C  "));
    Serial.print(F("Humidity: ")); Serial.print(humidity);    Serial.print(F("%  "));
    Serial.print(F("Zone: "));     Serial.println(zoneLabel(zone));
  }
}


// here the temperature is compared with thresholds from hottest downward
// the method returns the matching ComfortZone enum value.

ComfortZone determineZone(float temp) {
  if (temp >= TEMP_HOT)     return HOT;
  if (temp >= TEMP_MEDHIGH) return MEDHIGH;
  if (temp >= TEMP_MED)     return MED;
  if (temp >= TEMP_MEDLOW)  return MEDLOW;
  return COOL;
}


// setFanSpeed

void setFanSpeed(ComfortZone zone) {
  switch (zone) {
    case COOL:    fanServo.write(FAN_OFF);     break;
    case MEDLOW:  fanServo.write(FAN_MEDLOW);  break;
    case MED:     fanServo.write(FAN_MED);     break;
    case MEDHIGH: fanServo.write(FAN_MEDHIGH); break;
    case HOT:     fanServo.write(FAN_FULL);    break;
  }
}


// setComfortLED

void setComfortLED(ComfortZone zone) {
  switch (zone) {
    case COOL:    setRGB(false, true);  break;  // Green
    case MEDLOW:  setRGB(false, true);  break;  // Green
    case MED:     setRGB(true,  true);  break;  // Yellow
    case MEDHIGH: setRGB(true,  true);  break;  // Yellow
    case HOT:     setRGB(true,  false); break;  // Red
  }
}


// setRGB

void setRGB(bool red, bool green) {
  digitalWrite(LED_RED_PIN,   red   ? HIGH : LOW);
  digitalWrite(LED_GREEN_PIN, green ? HIGH : LOW);
}


// updateDisplay

void updateDisplay(float temp, float hum, ComfortZone zone) {
  display.clearDisplay();

  //header uses inverted colours for visual contrast
  display.fillRect(0, 0, 128, 12, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setTextSize(1);
  display.setCursor(10, 2);
  display.print(F("ROOM COMFORT MONITOR"));

  //the temperature reading in large font for easy reading
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 16);
  display.print(F("Temp:"));
  display.setTextSize(2);          
  display.setCursor(40, 13);
  display.print(temp, 1);          
  display.setTextSize(1);
  display.setCursor(110, 16);
  display.print(F("C"));

  //humidity reading also in large font
  display.setTextSize(1);
  display.setCursor(0, 36);
  display.print(F("Hum: "));
  display.setTextSize(2);
  display.setCursor(40, 33);
  display.print(hum, 1);
  display.setTextSize(1);
  display.setCursor(110, 36);
  display.print(F("%"));

  //horizontal line between readings and status
  display.drawLine(0, 50, 128, 50, SSD1306_WHITE);

  //the status bar, zone label and current servo angle
  display.setTextSize(1);
  display.setCursor(0, 54);
  switch (zone) {
    case COOL:    display.print(F("COOL    Fan:OFF   0deg"));  break;
    case MEDLOW:  display.print(F("MED-LOW Fan:LOW  45deg"));  break;
    case MED:     display.print(F("MED     Fan:MED  90deg"));  break;
    case MEDHIGH: display.print(F("MED-HI  Fan:HIGH135deg"));  break;
    case HOT:     display.print(F("HOT!    Fan:MAX  180deg")); break;
  }

  
  display.display();
}


// zoneLabel

const char* zoneLabel(ComfortZone zone) {
  switch (zone) {
    case COOL:    return "COOL";
    case MEDLOW:  return "MED-LOW";
    case MED:     return "MED";
    case MEDHIGH: return "MED-HIGH";
    case HOT:     return "HOT";
    default:      return "UNKNOWN";
  }
}


// displayError in case nan

void displayError() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(10, 20);
  display.println(F("Sensor Error!"));
  display.setCursor(10, 35);
  display.println(F("Check DHT22 wiring"));
  display.display();

  //brief red blink as a visual alert
  setRGB(true, false);
  delay(300);
  setRGB(false, false);
}