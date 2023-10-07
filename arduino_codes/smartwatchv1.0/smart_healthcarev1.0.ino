#include <MAX30105.h>
#include <heartRate.h>
#include <DFRobot_MLX90614.h>
#include "Scaler.h"
#include "Classifier.h"
#include <Arduino.h>
#include <U8x8lib.h>
U8X8_SSD1306_64X48_ER_SW_I2C u8x8(5,4);

MAX30105 particleSensor;
Eloquent::ML::Port::LogisticRegression clf;
DFRobot_MLX90614_I2C tempSensor;

const int vect_size = 3;

int bpm_vector[vect_size];
int bpm_vector_index = 0;
int total_bpm = 0;
int smoothed_bpm = 0;
long lastBeat;

int temp_vector[vect_size];
int temp_vector_index = 0;
int total_temp = 0;
int smoothed_temp = 0;

int response;

int printing_interval = 10000;
unsigned long time_now = 0;

void setup() {

  Serial.begin(9600);
  particleSensor.begin(Wire, I2C_SPEED_STANDARD, 0x57);
  tempSensor.begin();
  u8x8.begin();

  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0);
  particleSensor.setPulseAmplitudeIR(0);
  particleSensor.setPulseAmplitudeGreen(0x19);

  u8x8.setBusClock(100000);
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.drawString(0, 2, "Wait..");
  delay(2000);
  u8x8.clearDisplay();
  u8x8.drawString(1, 0, "Hello");

  // initialize all the readings to 0:
  for (int thisReading = 0; thisReading < vect_size; thisReading++) {
    bpm_vector[thisReading] = 0;
    temp_vector[thisReading] = 0;
  }

  u8x8.setFont(u8x8_font_open_iconic_embedded_2x2);
        u8x8.drawGlyph(1, 2, 0x46);
        u8x8.drawGlyph(1, 4, 0x45);

}

void hrCollection() {
  long irValue = particleSensor.getGreen();
  if (checkForBeat(irValue)) {
    unsigned long delta = millis() - lastBeat;
    lastBeat = millis();
    int beatsPerMinute = 60 / (delta / 1000.0);
    digitalWrite(LED_BUILTIN, LOW);

    if (beatsPerMinute < 255 && beatsPerMinute > 20) {
      //Smoothing
      total_bpm = total_bpm - bpm_vector[bpm_vector_index]; //substract the last reading
      bpm_vector[bpm_vector_index] = beatsPerMinute; //filling the vector with readings
      total_bpm = total_bpm + bpm_vector[bpm_vector_index]; //add readings to the total
      bpm_vector_index = bpm_vector_index + 1; //advance to the next position

      if (bpm_vector_index >= vect_size) { //if we are at the end of the array..
        bpm_vector_index = 0; //..go back to the beginning
      }

      smoothed_bpm = total_bpm / vect_size;

    }
  } else {
    digitalWrite(LED_BUILTIN, HIGH);
  }
}

void stCollection() {
    
  int degree = tempSensor.getObjectTempCelsius();

  //Smoothing Average
  total_temp = total_temp - temp_vector[temp_vector_index]; //substract the last reading
  temp_vector[temp_vector_index] = degree; //filling the vector with readings
  total_temp = total_temp + temp_vector[temp_vector_index]; //add readings to the total
  temp_vector_index = temp_vector_index + 1; //advance to the next position

  if (temp_vector_index >= vect_size) { //if we are at the end of the array..
  temp_vector_index = 0; //..go back to the beginning
  }

  smoothed_temp = total_temp / vect_size;
  
}

void loop() {

  hrCollection();

  stCollection();

  int features[] = {smoothed_bpm, smoothed_temp};
  if (!processor.transform(features))
    return;

  response = clf.predict(processor.X);

  if(millis() >= time_now + printing_interval){
        time_now += printing_interval;
        //Serial.print("BPM:"); Serial.println(smoothed_bpm);
        //Serial.print("Temp: "); Serial.println(smoothed_temp);
        //Serial.print("Resp: "); Serial.println(response);
        u8x8.setFont(u8x8_font_chroma48medium8_r);
        u8x8.setCursor(4, 3);
        u8x8.print(smoothed_bpm);
        u8x8.setCursor(4, 5);
        u8x8.print(smoothed_temp);
  
        if (response == 0) {
          u8x8.setFont(u8x8_font_open_iconic_check_2x2);
          u8x8.drawGlyph(6, 3, 0x40);
        }
        else {
          u8x8.setFont(u8x8_font_open_iconic_embedded_2x2);
          u8x8.drawGlyph(6, 3, 0x47);
        }
    }

}
