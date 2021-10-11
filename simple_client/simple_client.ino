#include <ESP8266WiFi.h>

bool isMotion = false;

// Information for accessing the server
const char *ssid = "ED_59";
const char *password = "ED_59_ADMIN";
const char *host = "192.168.11.4"; // IP address of server

WiFiClient client;

void setup()
{
  pinMode(D0, OUTPUT);  // LED On the board
  pinMode(D1, INPUT);   // PIR motion sensor (2s approx delay)
  Serial.begin(115200);

  // Connect to the server
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

void loop()
{
  // Get a measurement from the sensor(s)
  isMotion = digitalRead(D1);
  if (isMotion) {
    digitalWrite(D0, LOW);
  }

  // Connect to the server and send the data as a URL parameter
  if (client.connect(host, 80))
  {
    String url = "/update?value=";
    if (isMotion) {
      url += "1.0";
    } else {
      url += "0.0";
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
  delay(100);
  digitalWrite(D0, HIGH);
}
