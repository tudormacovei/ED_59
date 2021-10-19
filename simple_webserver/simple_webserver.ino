#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h>

// NOTE: CLOCKWISE = OPEN, ANTI = CLOSE
#define SPEED_CLOCKWISE 20
#define SPEED_ANTICLOCK 160
#define SPEED_STOP 90

Servo cold, hot;

// Configuration parameters for server ("base station")
char *ssid_ap = "ED_59";
char *password_ap = "ED_59_ADMIN";
IPAddress ip(192, 168, 11, 4); // arbitrary IP address
IPAddress gateway(192, 168, 11, 1);
IPAddress subnet(255, 255, 255, 0);

ESP8266WebServer server;

// Sensor data that's going to be recieved
float sensors_value = 0.0;

// servo handling variables
// !IMPORTANT: ball valles are closed at the start
int should_hot_close = 1;
int should_cold_close = 1;
int is_hot_close = 1;
int is_cold_close = 1;

// define pinouts
const int led_board = 16; // D0 pin, debug purposes
const int servoCold = 14; // D5 pin
const int servoHot = 0;   // D3 pin

void setup()
{
//  pinMode(led_board, OUTPUT);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(ip, gateway, subnet);
  WiFi.softAP(ssid_ap, password_ap);
  // Print IP Address as a sanity check
  Serial.begin(115200);
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Configure the server's routes
  server.on("/", handleIndex);        // use the top root path to report the last sensor value
  server.on("/update", handleUpdate); // use this route to update the sensor value
  server.begin();

  cold.attach(servoCold);
  hot.attach(servoHot);
}

void loop()
{
  // update servo control variables
  server.handleClient();
  // respond to the update
  servo_control();
  Serial.println(sensors_value);
}

void servo_control() {
  if (should_hot_close && !is_hot_close) {
    is_hot_close = 1;
    hot_close();
  }
  if (!should_hot_close && is_hot_close) {
    is_hot_close = 0;
    hot_open();
  }
  if (should_cold_close && !is_cold_close) {
    is_cold_close = 1;
    cold_close();
  }
  if (!should_cold_close && is_cold_close) {
    is_cold_close = 0;
    cold_open();
  }
}

void cold_open()
{
  cold.write(SPEED_CLOCKWISE);
  delay(900);
  cold.write(SPEED_STOP);
  delay(100);
}

void cold_close()
{
  cold.write(SPEED_ANTICLOCK);
  delay(900);
  cold.write(SPEED_STOP);
  delay(100);
}

void hot_open()
{
  hot.write(SPEED_CLOCKWISE);
  delay(900);
  hot.write(SPEED_STOP);
  delay(100);
}

void hot_close()
{
  hot.write(SPEED_ANTICLOCK);
  delay(900);
  hot.write(SPEED_STOP);
  delay(100);
}

// debugging purposes
void handleIndex()
{
  server.send(200, "text/plain", String(sensors_value)); // refresh the page for getting the latest value (for debugging purposes)
}

void handleUpdate()
{
  // The value will be passed as a URL argument
  sensors_value = server.arg("value").toFloat();
  server.send(200, "text/plain", "Updated!");
  if (sensors_value > 0.5 && sensors_value < 1.5) {
    Serial.println("I want to open both valves");
    should_hot_close = 0;
    should_cold_close = 0;
  }
  if (sensors_value > 1.5 && sensors_value < 2.5) {
    Serial.println("I want to close the hot valve");
    should_hot_close = 1;
    should_cold_close = 0;
  }
  if (sensors_value > -0.5 && sensors_value < 0.5) {
    Serial.println("I want to close both valves");
    should_hot_close = 1;
    should_cold_close = 1;
  }
  Serial.println(sensors_value);
}
