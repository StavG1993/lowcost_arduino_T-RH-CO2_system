#include <Wire.h>  //communication between Arduino boards and I2C (Inter-Integrated Circuit) devices. I2C is a protocol that allows multiple 
                  //devices (like sensors, displays, and other peripherals) to communicate with a microcontroller using just two data lines: 
                  //SCL (clock line) and SDA (data line).
#include <Adafruit_SHTC3.h> //package for the sensor SHTC3 TEMP&RH
#include <SparkFun_SCD4x_Arduino_Library.h> //PACKAGE FOR THE SENSOR SCD40 (temp,RH, CO2)
#include <SD.h> // package for SD card 

// Defining the sensors
Adafruit_SHTC3 shtc3;
SCD4x scd4x; //representing the SCD40 sensor

const int ledPin = 13; //Built in pin number for arduino uno. That is also connected to the SD card box and so the LED doesn't turn on&off as defined in the code
const int chipSelect = 8; // the CS wire - card selection
const char* logFile = "data_mid.txt"; // naming the folder that the data will be saved in
unsigned long startTime; // that it will know the milliseconds that has passed since starting the loop

void setup() {
  Serial.begin(115200); // recomended baud - fast and efficient
  delay(1000); // delay for connecting and initiallizing

  Serial.println("[BOOT] Starting environmental logger...");

  pinMode(ledPin, OUTPUT); // so that the pin (num13) will provide voltage for the LED

  // Initialize sensors (do NOT start measurements yet)
  Serial.println("Initializing sensors...");
  // a check - in case the sensor doesn't work it will print that it failed and won't start the loop of data collecting
  if (!shtc3.begin()) {
    Serial.println("Failed to find SHTC3 sensor");
    while (1) delay(10);
  }
  Serial.println("SHTC3 sensor found"); // if it finds the sensor, it also informs me

  // same as the other sensor
  if (!scd4x.begin()) {
    Serial.println("Failed to find SCD4x sensor");
    while (1) delay(10);
  }
  Serial.println("SCD4x sensor found");

  delay(1000); // Pause before SD card access

  // Initialize SD - as the sensors' first initializing the SD card in the void setup and making sure im informed if it doesn't work
  Serial.print("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("SD card initialization failed");
    while (1) delay(10);
  }
  Serial.println("SD card initialized");

  delay(2000); // Let SD card fully settle before file access

  // Skipping file header to avoid brownout
  Serial.println("Skipping file header write");

  Serial.println("Setup complete");

  scd4x.startPeriodicMeasurement();
  startTime = millis(); // defining the start time in milli seconds as required in the assignment instructions
}

// in the loop the LED will light, than it will read the 2 sensors data and print it in serial print and than save it in the SD card in a folder, the LED
// TURNS OFF and the loop starts again. it's a continuence data measurment for at least 3 days so the time frame isn't defined
void loop() {
  digitalWrite(ledPin, HIGH); // at this point the LED should open, it didn't happen for me.. Since the 13 PIN is used for the SD card as well as the built in PIN for the LED in arduino UNO.
  delay(500); // LED ON visibly before reading/logging

  // --- Sensor Reading ---
  sensors_event_t shtc3_humidity_event, shtc3_temp_event; // event is how the data is stored in this sensor. starting it basically calls the data
  float shtc3_temperature = 0; // float - the data is saved in float type, 0 - before the first reading, giving it a number so I know when the readings have started after that
                                // since it's May in the negev desert I don't think it will match other temprature measured
  float shtc3_humidity = 0; // same goes here
  
  // if the sensor can read the temprature, it prints it on serial print
  if (shtc3.getEvent(&shtc3_humidity_event, &shtc3_temp_event)) { //get event calls the readings of the sensor - temprature and humidity in this case
    shtc3_temperature = shtc3_temp_event.temperature; 
    shtc3_humidity = shtc3_humidity_event.relative_humidity;

    //printing the readings
    Serial.print("SHTC3 - Temp: ");
    Serial.print(shtc3_temperature);
    Serial.print(" °C, Humidity: ");
    Serial.print(shtc3_humidity);
    Serial.println(" %");
    // if there are no readings - informs me
  } else {
    Serial.println("SHTC3 read error");
  }

  // same for the other sensor
  uint16_t scd4x_co2 = 0;
  float scd4x_temperature = 0;
  float scd4x_humidity = 0;

  if (scd4x.readMeasurement()) {
    scd4x_co2 = scd4x.getCO2();
    scd4x_temperature = scd4x.getTemperature();
    scd4x_humidity = scd4x.getHumidity();

    Serial.print("SCD4x - CO2: ");
    Serial.print(scd4x_co2);
    Serial.print(" ppm, Temp: ");
    Serial.print(scd4x_temperature);
    Serial.print(" °C, Humidity: ");
    Serial.print(scd4x_humidity);
    Serial.println(" %");
  } else {
    Serial.println("SCD4x read error");
  }

  // --- Save to SD ---
  File dataFile = SD.open(logFile, FILE_WRITE); // open the folder I defined to save the data and getting ready to write the data in the folder
  if (dataFile) {
    unsigned long currentTime = millis(); // saving the time in milli seconds as reauired and as defined in the begining of the loop

    // printing with "," so it creates a csv file witch is later easy to work on and produce graphs
    dataFile.print(currentTime);
    dataFile.print(",");
    dataFile.print(shtc3_temperature);
    dataFile.print(",");
    dataFile.print(shtc3_humidity);
    dataFile.print(",");
    dataFile.print(scd4x_temperature);
    dataFile.print(",");
    dataFile.print(scd4x_humidity);
    dataFile.print(",");
    dataFile.println(scd4x_co2);

    dataFile.flush();
    dataFile.close();
    Serial.println("Data saved to SD card."); // informing me that it's working and data is being saved
  } else {
    Serial.println("Failed to open data_mid.txt in loop"); // also informing me if it doesn't work..
  }

  // --- Plotting Output ---

  // so I can also see the data in serial prints from the 2 sensors together and make sure it make since
  Serial.print("<DATA>");
  Serial.print(shtc3_temperature); Serial.print(",");
  Serial.print(scd4x_temperature); Serial.print(",");
  Serial.print(shtc3_humidity); Serial.print(",");
  Serial.print(scd4x_humidity); Serial.print(",");
  Serial.print(scd4x_co2);
  Serial.println("</DATA>");

  digitalWrite(ledPin, LOW);     // Turn LED OFF after logging, again, in my case it never turned on.. 
  delay(14500);                  // Remaining delay to make full ~15 sec loop
}