#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h>

Servo cold, hot;
const int servoCold = 14; // D5 pin
//const int servoHot = 2;

// Configuration parameters for server ("base station")
char *ssid_ap = "ED_59";
char *password_ap = "ED_59_ADMIN";
IPAddress ip(192, 168, 11, 4); // arbitrary IP address
IPAddress gateway(192, 168, 11, 1);
IPAddress subnet(255, 255, 255, 0);

ESP8266WebServer server;

// Sensor data that's going to be recieved
float motion_value = 0.0;

// define pinouts
const int led_board = 16; // D0 pin

void setup()
{
  pinMode(D0, OUTPUT); // LED On the board
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
  //  hot.attach(servoHot);
}

void loop()
{
  // put your main code here, to run repeatedly:
  server.handleClient();
  // turn the LED on the board on if motion is detected by client
  if (motion_value > 0.5)
  {
    digitalWrite(D0, LOW);
    //    hot_open();
    cold_open();
  }
  else
  {
    cold_close();
    digitalWrite(D0, HIGH);
  }
}

// these functions are causing a lot of issues, must fix since they take way too long
void cold_open()
{
  for (int pos = 0; pos <= 180; pos += 1)
  {
    // goes from 0 degrees to 180 degrees in steps of 1 degree
    cold.write(pos); // tell servo to turn 'pos' degrees (I think)
    delay(15);       // waits 15ms for the servo to reach the position
  }
}

void cold_close()
{
  for (int pos = 0; pos <= 180; pos += 1)
  {
    // goes from 0 degrees to 180 degrees in steps of 1 degree
    cold.write(-pos); // tell servo to turn 'pos' degrees (I think)
    delay(15);        // waits 15ms for the servo to reach the position
  }
}

// debugging purposes
void handleIndex()
{
  server.send(200, "text/plain", String(motion_value)); // refresh the page for getting the latest value (for debugging purposes)
}

void handleUpdate()
{
  // The value will be passed as a URL argument
  motion_value = server.arg("value").toFloat();
  Serial.println(motion_value);
  server.send(200, "text/plain", "Updated");
}
