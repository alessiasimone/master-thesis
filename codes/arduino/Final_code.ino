//__________________________________________________________________________________________________________________________________________________________________________
//IMPORTING LIBRARIES
#include <PlaquetteLib.h> // --> Smoothing and scaling operations [https://sofapirate.github.io/Plaquette/]
using namespace pq;
Smoother smoother3(3.0f); // --> third moving mean of 3 seconds for ST values
Smoother smoother2(3.0f); // --> second moving mean of 3 seconds for HR values
#include "Wire.h" // --> I2C connection
#include "MAX30105.h" // --> Sparkfun library for MAX30101 sensor [https://github.com/sparkfun/SparkFun_MAX3010x_Sensor_Library]
TwoWire particleConnection = TwoWire(1);
TwoWire tempConnection = TwoWire(2);
MAX30105 particleSensor; // --> used for BPM
#include <DFRobot_MLX90614.h> // --> DFRobot library for MLX90614 sensor [https://github.com/DFRobot/DFRobot_MLX90614]
DFRobot_MLX90614_I2C tempSensor(0x5A, &tempConnection); // --> used for peripheral skin temperature
#include "Scaler.h" // --> Feature extraction library [https://github.com/eloquentarduino/everywhereml]
#include "Classifier.h" // --> Classification library containing Logistic Regression algorithm [https://github.com/eloquentarduino/micromlgen]
Eloquent::ML::Port::LogisticRegression clf;
#include "heartRate.h" // --> Optical Heart Rate Detection (PBA Algorithm) [https://github.com/sparkfun/SparkFun_MAX3010x_Sensor_Library/blob/master/src/heartRate.h]

//____________________________________________________________________________________________________________________________________________________________________________
//DEFINING GLOBAL VARIABLES
long beat; // --> raw green led signal from MAX30101
long lastBeat = 0; // --> count in milliseconds to determine IBI
float BPM; // --> raw BPM values
int beatAvg; // --> averaged BPM values
long unblockedValue; // --> average IR at power up
long startTime; // --> used to calculate measurement rate
float degree; // --> raw peripheral skin temperature in °C
float st; // averaged peripheral skin temperature in °C
unsigned long time_printing = 0; // --> variables for millis delay (1Hz)
unsigned long time_now1 = 0;
unsigned long time_now2 = 0;
unsigned long time_now3 = 0;
bool Presence;
const int WINDOW_SIZE = 60;
int signal1[WINDOW_SIZE];
int signal2[WINDOW_SIZE];
int signal1Index = 0;
int signal2Index = 0;
const byte RATE_SIZE = 3; 
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
float beatsPerMinute;

//_________________________________________________________________________________________________________________________________________________________________________
void setup() {

  //instrumental initialization
  Serial.begin(9600);
  Plaquette.begin();
  particleConnection.begin(41, 40, 100000);
  tempConnection.begin(34, 33, 100000);

  //st sensor
  tempSensor.begin();  
  tempSensor.setEmissivityCorrectionCoefficient(0.98); // each surface has a different grade of IR emissivity: human skin has an emissivity of 0.98.

  //hr sensor
  particleSensor.begin(particleConnection);
  byte ledBrightness = 0x1F;
  byte sampleAverage = 1; 
  byte ledMode = 3; 
  int sampleRate = 100;
  int pulseWidth = 411;
  int adcRange = 4096; 
  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); 
  particleSensor.setPulseAmplitudeRed(0); //0 mA
  particleSensor.setPulseAmplitudeIR(0);  // 0 mA
  particleSensor.setPulseAmplitudeGreen(0x19); // 1 mA
  
  }

//_________________________________________________________________________________________________________________________________________________________________________
// BPM values with MAX30101 sensor 1Hz
void hrCollection() {
/**
* The particle sensor capture the green led emission from the inner wrist. When a blood pulse happen, the photodiode capture a pulse of light which corresponds to a heart beat.
* In order to be able to capture peaks throw the wrist, the green led has been accurately set up in the setup() section, following the instructions published by the authors
* on page 22 of the paper. [https://pdfserv.maximintegrated.com/en/an/AN6409.pdf]
* The peaks, corresponding to the BPM, are computed throw a PBA algorithm, then, they are averaged with a moving window of size 3. 
*/
  unsigned long currentMillis2 = millis();
  beat = particleSensor.getGreen();

  if (checkForBeat(beat) == true) { // --> we sensed a beat, hence we compute the BPM at 1Hz
    if (currentMillis2 - time_now2 >= 1000) {
      time_now2 = currentMillis2;
      long delta = millis() - lastBeat;
      lastBeat = millis();
      BPM = 60 / (delta / 1000.0);
    }      

    if (BPM < 255 && BPM > 20) // motion artefact remover
    {
      rates[rateSpot++] = (byte)BPM; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
      // Add beatAvg to signal1
      signal1[signal1Index] = beatAvg;
      signal1Index = (signal1Index + 1) % WINDOW_SIZE;
    }
  }
}
//_________________________________________________________________________________________________________________________________________________________________-
// ST values with MLX90614 sensor 4Hz.
void stCollection() {
/**
* Peripheral skin Temperature has been measured in degree Celsius thanks to an InfraRed thermophile that capture the emissivity of the human skin.
*/
  unsigned long currentMillis3 = millis();

  if(currentMillis3 - time_now3 >= 1000){
    time_now3 = currentMillis3;
    degree = tempSensor.getObjectTempCelsius();
  }

  st = degree >> smoother3;
  // Add st to signal2
  signal2[signal2Index] = st;
  signal2Index = (signal2Index + 1) % WINDOW_SIZE;

}
//____________________________________________________________________________________________________________________________________________________________________________
// Feature Extraction
void extraction(float* features) {
  // Calculate minimum, maximum, mean, median, standard deviation, variance, skewness, and kurtosis for signal1 (beatAvg)
  int beatMin = INT_MAX;
  int beatMax = INT_MIN;
  float beatSum = 0.0;
  float beatSumSquares = 0.0;
  float beatSkewness = 0.0;
  float beatKurtosis = 0.0;
  int sortedSignal1[WINDOW_SIZE];

  for (int i = 0; i < WINDOW_SIZE; i++) {
    if (signal1[i] < beatMin) {
      beatMin = signal1[i];
    }
    if (signal1[i] > beatMax) {
      beatMax = signal1[i];
    }
    beatSum += signal1[i];
    beatSumSquares += pow(signal1[i], 2);
    sortedSignal1[i] = signal1[i];
  }

  float beatMean = beatSum / WINDOW_SIZE;

  std::sort(sortedSignal1, sortedSignal1 + WINDOW_SIZE);

  int middle = WINDOW_SIZE / 2;
  int beatMedian;
  if (WINDOW_SIZE % 2 == 0) {
    beatMedian = (sortedSignal1[middle - 1] + sortedSignal1[middle]) / 2;
  } else {
    beatMedian = sortedSignal1[middle];
  }

  float beatStdDeviation = sqrt((beatSumSquares / WINDOW_SIZE) - pow(beatMean, 2));
  float beatVariance = pow(beatStdDeviation, 2);

  for (int i = 0; i < WINDOW_SIZE; i++) {
    float deviation = signal1[i] - beatMean;
    beatSkewness += pow(deviation, 3);
    beatKurtosis += pow(deviation, 4);
  }
  beatSkewness /= (WINDOW_SIZE * pow(beatStdDeviation, 3));
  beatKurtosis /= (WINDOW_SIZE * pow(beatStdDeviation, 4));

  // Calculate minimum, maximum, mean, median, standard deviation, variance, skewness, and kurtosis for signal2 (st)
  float stMin = FLT_MAX;
  float stMax = FLT_MIN;
  float stSum = 0.0;
  float stSumSquares = 0.0;
  float stSkewness = 0.0;
  float stKurtosis = 0.0;
  float sortedSignal2[WINDOW_SIZE];

  for (int i = 0; i < WINDOW_SIZE; i++) {
    if (signal2[i] < stMin) {
      stMin = signal2[i];
    }
    if (signal2[i] > stMax) {
      stMax = signal2[i];
    }
    stSum += signal2[i];
    stSumSquares += pow(signal2[i], 2);
    sortedSignal2[i] = signal2[i];
  }

  float stMean = stSum / WINDOW_SIZE;

  std::sort(sortedSignal2, sortedSignal2 + WINDOW_SIZE);

  middle = WINDOW_SIZE / 2;
  int stMedian;
  if (WINDOW_SIZE % 2 == 0) {
    stMedian = (sortedSignal2[middle - 1] + sortedSignal2[middle]) / 2;
  } else {
    stMedian = sortedSignal2[middle];
  }

  float stStdDeviation = sqrt((stSumSquares / WINDOW_SIZE) - pow(stMean, 2));
  float stVariance = pow(stStdDeviation, 2);

  for (int i = 0; i < WINDOW_SIZE; i++) {
    float deviation = signal2[i] - stMean;
    stSkewness += pow(deviation, 3);
    stKurtosis += pow(deviation, 4);
  }
  stSkewness /= (WINDOW_SIZE * pow(stStdDeviation, 3));
  stKurtosis /= (WINDOW_SIZE * pow(stStdDeviation, 4));

  // Store the calculated features in the features array
  features[0] = beatMin;
  features[1] = beatMax;
  features[2] = beatMean;
  features[3] = beatMedian;
  features[4] = beatStdDeviation;
  features[5] = beatVariance;
  features[6] = beatSkewness;
  features[7] = beatKurtosis;
  features[8] = stMin;
  features[9] = stMax;
  features[10] = stMean;
  features[11] = stMedian;
  features[12] = stStdDeviation;
  features[13] = stVariance;
  features[14] = stSkewness;
  features[15] = stKurtosis;
}
//_____________________________________________________________________________________________________________________________________________________________________________

//_____________________________________________________________________________________________________________________________________________________________________________
void loop() {

  Plaquette.step();

  hrCollection();
  stCollection();

  float features[16]; // Create the features array
  extraction(features); 

  if (!processor.transform(features))
  return;

  if((millis() - time_printing) >= 1000){
    time_printing = millis();
    Serial.print(beatAvg); Serial.print(","); Serial.print(st); Serial.print(","); Serial.println(clf.predictLabel(processor.X));
  }
  
}
