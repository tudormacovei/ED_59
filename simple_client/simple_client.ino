#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define MAX_TEMP 29.0
// 29.0 is for testing purposes, should be switched in the final version

// sensor variables
bool isMotion = false;
float temperature = 0.0;

// Information for accessing the server
const char *ssid = "ED_59";
const char *password = "ED_59_ADMIN";
const char *host = "192.168.11.4"; // IP address of server

// define input/output ports 
const int led_board = 16;     // D0 pin
const int led_external_1 = 0; // D3 pin
const int led_external_2 = 4; // D2 pin
const int motion_sensor = 14; // D5 pin (D4 pin is not working for some reason)
const int temp_sensor = 5;    // D1 pin
const int buzzer = 12;        // D6 pin

unsigned long last_movement;

WiFiClient client;
OneWire oneWire(temp_sensor);
DallasTemperature sensors(&oneWire);

void server_connect() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("IP Address (AP): ");
  Serial.println(WiFi.localIP());
}

void setup()
{
  Serial.begin(115200);
  pinMode(led_board, OUTPUT);  // LED On the board
  pinMode(motion_sensor, INPUT);   // PIR motion sensor
  pinMode(led_external_1, OUTPUT);
  pinMode(led_external_2, OUTPUT);
  pinMode(temp_sensor, INPUT);
  pinMode(buzzer, OUTPUT);

  server_connect();
  sensors.begin();
}

void loop()
{
  // Get a measurement from the sensor(s)
  sensors.requestTemperatures();                // Send the command to get temperatures  
  isMotion = digitalRead(motion_sensor);
  temperature = sensors.getTempCByIndex(0);     // index 0 refers to the first IC on the wire (in our case we only have 1)
  Serial.println("Temperature is: ");
  Serial.println(temperature);   

  // Connect to the server and send the data as a URL parameter
  contact_server();
  
  debug_led();
  control_led();  
  control_buzzer();
}

void control_buzzer() {
  if (temperature > MAX_TEMP) {
     tone(buzzer, 2300); // Send 2.3KHz sound signal
  } else {
     noTone(buzzer);
  }
}

void debug_led() {
  if (isMotion) {
    digitalWrite(led_board, LOW);
    last_movement = millis();
  } else {
    digitalWrite(led_board, HIGH);
  }
}

void control_led() {
  if (millis() - last_movement > 10000) {
    digitalWrite(led_external_1, LOW);
  if (millis() - last_movement < 15000) {
      digitalWrite(led_external_2, HIGH);  
    } else {
      digitalWrite(led_external_2, LOW);  
    }
  } else {
    digitalWrite(led_external_1, HIGH);
    digitalWrite(led_external_2, LOW);  
  }
}

void contact_server() {
  if (client.connect(host, 80))
  {
    String url = "/update?value=";
    if (!isMotion && millis() - last_movement > 20000) {
      // close servos / keep closed
      url += "0.0";
      Serial.println("CLOSE");
    } else {
      // open servos
      if (temperature > MAX_TEMP) {
        // close hot servo
        url +="2.0";
        Serial.println("CLOSE HOT");
      } else {
        // open servos / keep open
        url += "1.0"; 
        Serial.println("OPEN");
      }
    }
    client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" +
                 "Connection: keep-alive\r\n\r\n"); // minimum set of required URL headers
    delay(20);
    // Read all the lines of the response
    while (client.available())
    {
      String line = client.readStringUntil('\r');
    }
  }
}
