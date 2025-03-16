#include <Arduino.h>
#include <DHT.h>

// function declarations
int myFunction(int, int);
void blink_led();

// DHT sensor settings
#define DHTTYPE DHT22 // DHT 22 (AM2302)
int sensor1 = 13;
int sensor2 = 15;
DHT dht1(sensor1, DHTTYPE);
DHT dht2(sensor2, DHTTYPE);

int counter = 0;

void setup() {
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
  Serial.begin(115200);
  Serial.println("Starting sensor monitoring...");
  Serial.println(result);
  pinMode(LED_BUILTIN, OUTPUT); // Initialize the LED_BUILTIN pin as an output
  dht1.begin();
  dht2.begin();
}

void loop() {
  blink_led();
  // Read temperature and humidity from sensor 1
  float humidity1 = dht1.readHumidity();
  float temperature1 = dht1.readTemperature();

  // Read temperature and humidity from sensor 2
  float humidity2 = dht2.readHumidity();
  float temperature2 = dht2.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(humidity1) || isnan(temperature1)) {
    Serial.println("Failed to read from DHT sensor 1!");
  } else {
    Serial.print("Sensor 1 - Humidity: ");
    Serial.print(humidity1);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(temperature1);
    Serial.println(" *C");
  }

  if (isnan(humidity2) || isnan(temperature2)) {
    Serial.println("Failed to read from DHT sensor 2!");
  } else {
    Serial.print("Sensor 2 - Humidity: ");
    Serial.print(humidity2);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(temperature2);
    Serial.println(" *C");
  }

  //Serial.println(counter);
  //counter += 1;
}

int myFunction(int x, int y) {
  return x + y;
}

void blink_led() {
  // Blink the built-in LED:
  digitalWrite(LED_BUILTIN, HIGH); // Turn the LED on
  delay(100);                      // Wait for 100 milliseconds
  digitalWrite(LED_BUILTIN, LOW);  // Turn the LED off
  delay(900);                      // Wait for 900 milliseconds
}
