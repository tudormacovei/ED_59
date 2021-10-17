#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>

bool isMotion = false;

// Information for accessing the server
const char *ssid = "ED_59";
const char *password = "ED_59_ADMIN";
const char *host = "192.168.11.4"; // IP address of server

// define input/output ports 
const int led_board = 16;     // D0 pin
const int led_external_1 = 0; // D3 pin
const int led_external_2 = 4; // D2 pin
const int motion_sensor = 14;  // D5 pin (D4 pin is not working for some reason)
const int temp_sensor = 5;    // D1 pin

unsigned long last_movement;

WiFiClient client;
OneWire oneWire(temp_sensor);

DallasTemperature sensors(&oneWire);

void contact_server();

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
  pinMode(led_board, OUTPUT);  // LED On the board
  pinMode(motion_sensor, INPUT);   // PIR motion sensor
  pinMode(led_external_1, OUTPUT);
  pinMode(led_external_2, OUTPUT);
  pinMode(temp_sensor, INPUT);
  
//  pinMode(D6, INPUT);   // Temp Sensor input
  Serial.begin(115200);

  // Connect to the server
//  server_connect();

  sensors.begin();
}

void loop()
{
  // Get a measurement from the sensor(s)
  isMotion = digitalRead(motion_sensor);
  if (isMotion) {
    digitalWrite(led_board, LOW);
    last_movement = millis();
  }
  if (millis() - last_movement > 5000) {
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
  // Connect to the server and send the data as a URL parameter
  
  sensors.requestTemperatures();                // Send the command to get temperatures  
  Serial.println("Temperature is: ");
  Serial.println(sensors.getTempCByIndex(0));   // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire
  
  contact_server();
  delay(30);
  digitalWrite(led_board, HIGH);
}

void contact_server() {
  if (client.connect(host, 80))
  {
    String url = "/update?value=";
    if (!isMotion && millis() - last_movement > 5000) {
      url += "0.0";
    } else {
      url += "1.0";
    }
    client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" +
                 "Connection: keep-alive\r\n\r\n"); // minimum set of required URL headers
    delay(20);
    // Read all the lines of the response and print them to Serial
    Serial.println("\nResponse: ");
    while (client.available())
    {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
  }
}
