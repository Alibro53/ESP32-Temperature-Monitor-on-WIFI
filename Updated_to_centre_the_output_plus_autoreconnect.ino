#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPAsyncWebServer.h>

// Replace with your network credentials
const char* ssid = "XXXXXXX";
const char* password = "XXXXXXX";

// Create a OneWire object to communicate with the DS18B20 sensors
OneWire oneWire(15); // DS18B20 signal pin connected to GPIO5
DallasTemperature sensors(&oneWire);

// Define the number of sensors and their addresses
const int numSensors = 3;
DeviceAddress sensorAddresses[numSensors] = {
  {0x28, 0xAF, 0x88, 0x95, 0xF0, 0x1, 0x3C, 0xA7},
  {0x28, 0xFF, 0x64, 0x1, 0xB7, 0x5A, 0x8A, 0x6D},
  {0x28, 0xEF, 0x52, 0x95, 0xF0, 0x1, 0x3C, 0x31}
};

// Create a web server object
AsyncWebServer server(80);

float temperatures[numSensors];

void setup() {
  // Start serial communication for debugging purposes
  Serial.begin(115200);

  // Connect to Wi-Fi network
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected to WiFi");

  // Initialize the DS18B20 sensors
  sensors.begin();
  for (int i = 0; i < numSensors; i++) {
    sensors.setResolution(sensorAddresses[i], 9);
  }

  // Define web pages
  server.on("/", HTTP_GET, handleRoot);

  // Start the server
  server.begin();
  Serial.println("Server started");
}

void loop() {
  // If WiFi is not connected, attempt to reconnect
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection lost. Attempting to reconnect...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("Connected to WiFi");
  }

  // Read temperatures from the DS18B20 sensors
  sensors.requestTemperatures();
  for (int i = 0; i < numSensors; i++) {
    temperatures[i] = sensors.getTempC(sensorAddresses[i]);
    Serial.print("Sensor ");
    Serial.print(i);
    Serial.print(" temperature: ");
    Serial.println(temperatures[i]);
  }

  // Wait a few seconds before updating the web page
  delay(5000);
}

void handleRoot(AsyncWebServerRequest *request){
  // Create a HTML page with the temperatures
  String html = "<!DOCTYPE html>\n";
  html += "<html lang=\"en\">\n";
  html += "<head>\n";
  html += "<meta charset=\"UTF-8\">\n";
  html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
  html += "<title>Temperature</title>\n";
  html += "<style>\n";
  html += "  body {\n";
  html += "    font-family: Arial, sans-serif;\n";
  html += "    background-color: #f4f4f4;\n";
  html += "    margin: 0;\n";
  html += "  }\n";
  html += "  #container {\n";
  html += "    background-color: white;\n";
  html += "    border: 1px solid #ddd;\n";
  html += "    border-radius: 5px;\n";
  html += "    padding: 20px;\n";
  html += "    position: absolute;\n";
  html += "    top: 50%;\n";
  html += "    left: 50%;\n";
  html += "    transform: translate(-50%, -50%);\n";
  html += "    max-width: 600px;\n";
  html += "  }\n";
  html += "  h1 {\n";
  html += "    text-align: center;\n";
  html += "  }\n";
  html += "  table {\n";
  html += "    width: 100%;\n";
  html += "  }\n";
  html += "  td {\n";
  html += "    padding: 5px;\n";
  html += "  }\n";
  html += "  th {\n";
  html += "    text-align: left;\n";
  html += "    padding: 5px;\n";
  html += "  }\n";
  html += "</style>\n";
  html += "</head>\n";
  html += "<body>\n";
  html += "<div id=\"container\">\n";
  html += "<h1>Temperatures</h1>\n";
  html += "<table>\n";
  html += "<tr><th>Location</th><th>Temperature</th></tr>\n";
  html += "<tr><td>Top of Tank</td><td>" + String(temperatures[0]) + " &deg;C</td></tr>\n";
  html += "<tr><td>Middle of Tank</td><td>" + String(temperatures[1]) + " &deg;C</td></tr>\n";
  html += "<tr><td>Bottom of Tank</td><td>" + String(temperatures[2]) + " &deg;C</td></tr>\n";
  html += "</table>\n";
  html += "</div>\n";
  html += "</body>\n";
  html += "</html>\n";

 // Send the HTML page with CSS to the client
  request->send(200, "text/html", html);
}