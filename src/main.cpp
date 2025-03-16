#include <Arduino.h>
#include <Ticker.h>
#include "DHTesp.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#ifndef ESP32
#pragma message(THIS WORKS FOR ESP32 ONLY!)
#error Select ESP32 board.
#endif

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); // Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// Address 0x3D for 128x64
// Address 0x3C for 128x32
uint8_t display_addr = 0x3C;
uint8_t textSize = 2;

/** Initialize DHT sensors */
DHTesp dhtSensor1;
DHTesp dhtSensor2;
DHTesp dhtSensor3;

TaskHandle_t tempTaskHandle = NULL; /** Task handle for the light value read task */

int dhtPin1 = 13; /** Pin number for DHT11 1 data pin */
int dhtPin2 = 15; /** Pin number for DHT11 2 data pin */
int dhtPin3 = 12; /** Pin number for DHT11 3 data pin */

/** Ticker for temperature reading */
Ticker tempTicker;

/** Flags for temperature readings finished */
bool gotNewTemperature = false;

/** Data from sensors */
TempAndHumidity sensor1Data;
TempAndHumidity sensor2Data;
TempAndHumidity sensor3Data;

/* Flag if main loop is running */
bool tasksEnabled = false;

/* Function declarations */
String formatSerialData(TempAndHumidity data);
String formatDisplayData(TempAndHumidity data);
void printSerialData(TempAndHumidity sensor1Data, TempAndHumidity sensor2Data, TempAndHumidity sensor3Data);
void printDisplayData(uint8_t testSize, TempAndHumidity sensor1Data, TempAndHumidity sensor2Data, TempAndHumidity sensor3Data);
void triggerGetTemp();
void blinkLED();


/**
 * Task to reads temperature from DHT11 sensor
 * @param pvParameters
 *		pointer to task parameters
 */
void tempTask(void *pvParameters) {
	Serial.println("tempTask loop started");
	while (1) // tempTask loop
	{
		if (tasksEnabled && !gotNewTemperature) { // Read temperature only if old data was processed already
			// Reading temperature for humidity takes about 250 milliseconds!
			// Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
			sensor1Data = dhtSensor1.getTempAndHumidity();	// Read values from sensor 1
			sensor2Data = dhtSensor2.getTempAndHumidity();	// Read values from sensor 1
			sensor3Data = dhtSensor3.getTempAndHumidity();	// Read values from sensor 1
			gotNewTemperature = true;
		}
		vTaskSuspend(NULL);
	}
}

/**
 * triggerGetTemp
 * Sets flag dhtUpdated to true for handling in loop()
 * called by Ticker tempTicker
 */
void triggerGetTemp() {
	if (tempTaskHandle != NULL) {
		xTaskResumeFromISR(tempTaskHandle);
	}
}

void setup() {
	pinMode(LED_BUILTIN, OUTPUT); // Initialize the LED_BUILTIN pin as an output
	// blink led 3 times to indicate start of setup
	for (int i = 0; i < 3; i++) {
		blinkLED();
	}

	// Initialize serial port
	Serial.begin(115200);
	Serial.println("Read data from 3 DHT11/22 sensors");

	if(!display.begin(SSD1306_SWITCHCAPVCC, display_addr)) {
		Serial.println(F("SSD1306 allocation failed"));
		for(;;);
	}

	//delay(2000);
	display.clearDisplay();
	display.setTextSize(2);
	display.setTextColor(WHITE);
	display.setCursor(0, 0); // x, y
	// Display static text
	display.println("Getting\ntemp &\nhumidity\ndata");
	display.display(); 
	Serial.println("Display initialized");

	// Initialize temperature sensors
	dhtSensor1.setup(dhtPin1, DHTesp::DHT11);
	dhtSensor2.setup(dhtPin2, DHTesp::DHT11);
	dhtSensor3.setup(dhtPin3, DHTesp::DHT11);

	// Start task to get temperature
	xTaskCreatePinnedToCore(
		tempTask, 			/* Function to implement the task */
		"tempTask ",		/* Name of the task */
		4000,				/* Stack size in words */
		NULL,				/* Task input parameter */
		5,					/* Priority of the task */
		&tempTaskHandle,	/* Task handle. */
		1					/* Core where the task should run */
	);

	if (tempTaskHandle == NULL) {
		Serial.println("[ERROR] Failed to start task for temperature update");
	} else {
		// Start update of environment data every 10 seconds
		tempTicker.attach(10, triggerGetTemp);
	}

	// Signal end of setup() to tasks
	tasksEnabled = true;
} // End of setup.


void loop() {
	if (gotNewTemperature) {
		blinkLED();
		gotNewTemperature = false;
		printSerialData(sensor1Data, sensor2Data, sensor3Data);
		printDisplayData(textSize, sensor1Data, sensor2Data, sensor3Data);
	}
} // End of loop

String formatDisplayData(uint8_t textSize, TempAndHumidity data) {
	String temperature = String(data.temperature, 1);
	String humidity = String(data.humidity, 1);
	if (temperature == "nan") {
		temperature = "";
	}
	if (humidity == "nan") {
		humidity = "";
	}
	while (temperature.length() < (6 - textSize)) {
		temperature = " " + temperature;
	}
	while (humidity.length() < 5) {
		humidity = " " + humidity;
	}
	if (textSize == 1) {
		temperature += "C";
		humidity += "%";
		return " " + temperature + "    " + humidity;
		//           5 chars                5 chars
		// total 20 chars
	}
	else {
		return temperature + " " + humidity;
		//     4 chars             5 chars
		// total 10 chars
	}
}

void printDisplayData(uint8_t textSize, TempAndHumidity sensor1Data, TempAndHumidity sensor2Data, TempAndHumidity sensor3Data) {
	display.clearDisplay();
	if (textSize > 2) {
		textSize = 2;
	}
	display.setTextSize(textSize);
	display.setTextColor(WHITE);
	display.setCursor(0, 0); // x, y
	if (textSize == 1) {
		display.println("       Temp  Humidity");
		display.println("S1: " + formatDisplayData(1, sensor1Data));
		display.println("S2: " + formatDisplayData(1, sensor2Data));
		display.println("S3: " + formatDisplayData(1, sensor3Data));
	} else {
		display.println("Temp Humid");
		display.println(formatDisplayData(2, sensor1Data));
		display.println(formatDisplayData(2, sensor2Data));
		display.println(formatDisplayData(2, sensor3Data));
	}
	display.display(); 
}

String formatSerialData(TempAndHumidity data) {
	String temperature = String(data.temperature, 1);
	String humidity = String(data.humidity, 1);
	if (temperature == "nan") {
		temperature = "";
	}
	if (humidity == "nan") {
		humidity = "";
	}
	while (temperature.length() < 5) {
		temperature = " " + temperature;
	}
	while (humidity.length() < 5) {
		humidity = " " + humidity;
	}
	temperature += "C";
	humidity += "%";

	return temperature + " " + humidity + "  ";
}

void printSerialData(TempAndHumidity sensor1Data, TempAndHumidity sensor2Data, TempAndHumidity sensor3Data) {
	Serial.println("-- Sensor1 --  -- Sensor2 --  -- Sensor3 --");
	Serial.println("  Temp  Humid    Temp  Humid    Temp  Humid");
	Serial.println(formatSerialData(sensor1Data) + formatSerialData(sensor2Data) + formatSerialData(sensor3Data));
}


void blinkLED() {
	// Blink the built-in LED:
	digitalWrite(LED_BUILTIN, HIGH); // Turn the LED on
	delay(100);                      // Wait for 100 milliseconds
	digitalWrite(LED_BUILTIN, LOW);  // Turn the LED off
	delay(100);                      // Wait for 100 milliseconds
}
