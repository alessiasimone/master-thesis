#include "arduino_secrets.h"
#include <MAX30105.h>
#include <heartRate.h>
#include <DFRobot_MLX90614.h>
#include "Scaler.h"
#include "Classifier.h"
#include <Arduino.h>
#include <U8x8lib.h>
#include <Adafruit_SleepyDog.h>
U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);
#include "thingProperties.h"

MAX30105 particleSensor;
Eloquent::ML::Port::LogisticRegression clf;
DFRobot_MLX90614_I2C tempSensor;

const byte RATE_SIZE = 3;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;
int beatsPerMinute;

byte st_values[RATE_SIZE];
byte st_spot = 0;
int degree;

void setup() {

  Serial.begin(9600);
  particleSensor.begin(Wire, I2C_SPEED_FAST, 0x57);
  tempSensor.begin();
  u8x8.begin();
  u8x8.setBusClock(100000);
  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0);
  particleSensor.setPulseAmplitudeIR(0);
  particleSensor.setPulseAmplitudeGreen(0x19);
  particleSensor.setSampleRate(50);

  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.drawString(0, 4, "Initializing...");
  delay(2000);
  u8x8.clearDisplay();
  u8x8.drawString(0, 0, "Smart HealthCare");

  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
}

void hrCollection() {
  long irValue = particleSensor.getGreen();
  if (checkForBeat(irValue)) {
    unsigned long delta = millis() - lastBeat;
    lastBeat = millis();
    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20) {
      rates[rateSpot++] = (byte)beatsPerMinute;
      rateSpot %= RATE_SIZE;

      byte sum = 0;
      for (byte j = 0; j < RATE_SIZE; j++)
        sum += rates[j];
      beatsPerMinute = sum / RATE_SIZE;
    }
  }
}

void stCollection() {
  degree = tempSensor.getObjectTempCelsius();
  if (degree > 10 && degree < 50) {
    st_values[st_spot++] = (byte)degree;
    st_spot %= RATE_SIZE;
    byte sum = 0;
    for (byte z = 0; z < RATE_SIZE; z++)
      sum += st_values[z];
    degree = sum / RATE_SIZE;
  }
}

void loop() {
  ArduinoCloud.update();
  hrCollection();
  stCollection();

  int features[] = {beatsPerMinute, degree};
  if (!processor.transform(features))
    return;

  response = clf.predict(processor.X);

  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.setCursor(8, 4);
  u8x8.print(beatsPerMinute);
  u8x8.setCursor(8, 6);
  u8x8.print(degree);
  u8x8.setFont(u8x8_font_open_iconic_embedded_1x1);
  u8x8.drawGlyph(6, 4, 0x46);
  u8x8.drawGlyph(6, 6, 0x45);
  if (WiFi.status() == WL_CONNECTED) {
    u8x8.setFont(u8x8_font_open_iconic_embedded_1x1);
    u8x8.drawGlyph(8, 2, 0x50);
  }
}
