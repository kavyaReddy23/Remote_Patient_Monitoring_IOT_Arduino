#include <DHT.h>
#include <ESP8266WebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "MAX30100_PulseOximeter.h"
#define BLYNK_PRINT Serial
#include "Blynk.h"
#include <BlynkSimpleEsp8266.h>

#define DHTPIN D4   // Pin connected to the DHT sensor
#define DHTTYPE DHT22   // DHT 22 (AM2302)

DHT dht(DHTPIN, DHTTYPE);//Initialize DHT sensor
#define REPORTING_PERIOD_MS     1000

const char* auth="KrYX60VvZmgJd_rXE36Eq52IcmQObdUm";
const char* ssid = "kavyareddy";  // Enter SSID here
const char* password = "kavyareddy"; 
float humidity, temperature, bodyTemperatureC = 0.0, bodyTemperatureF = 0.0, BPM, SpO2;
PulseOximeter pox;//pulse oximeter object
uint32_t tsLastReport = 0;
ESP8266WebServer server(80);//webserver connected to port 80

int oneWireBus = 12;
OneWire oneWire(oneWireBus);//onewire bus pin for dallas temperature sensor
DallasTemperature sensors(&oneWire);//dallas temperture sensor object
 

void setup() {
    Serial.begin(115200);//initialize serial communication
    pinMode(D2, OUTPUT);//set pin D2 as output for the pulse oximeter
    dht.begin(); 

    Serial.println("Connecting to ");
    Serial.println(ssid);
  
    // Connect to your local Wi-Fi network
    WiFi.begin(ssid, password);
  
    // Check if Wi-Fi is connected to Wi-Fi network
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Blynk.begin(auth, ssid, password);//connect to blynk platform
    Serial.println("");
    Serial.println("WiFi connected..!");
    Serial.print("Got IP: ");  
    Serial.println(WiFi.localIP());   

    server.on("/", handle_OnConnect);
    server.onNotFound(handle_NotFound);
    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
  Blynk.run();
    server.handleClient();
   
    if (millis() - tsLastReport > REPORTING_PERIOD_MS)
    {
        Serial.print("Room Humidity: ");
        humidity = dht.readHumidity();
        Serial.print(humidity);
        Serial.println("%");

        temperature = dht.readTemperature();
        Serial.print("Room Temperature: ");
        Serial.print(temperature);
        Serial.println("°C");
        
        if (isnan(humidity) || isnan(temperature)) {
            Serial.println("Failed to read from DHT sensor!");
            return;
        }
        
        sensors.requestTemperatures();
        bodyTemperatureC = sensors.getTempCByIndex(0);
        bodyTemperatureF = sensors.getTempFByIndex(0);
      
        Serial.print("Body Temperature in Celsius: ");
        Serial.print(bodyTemperatureC);
        Serial.println("°C");
        Serial.print("Body Temperature in Fahrenheit: ");
        Serial.print(bodyTemperatureF);
        Serial.println("°F");
        BPM = pox.getHeartRate();
       SpO2 = pox.getSpO2();

        Serial.print("BPM: ");
        Serial.println(BPM);

       Serial.print("SpO2: ");
       Serial.print(SpO2);
       Serial.println("%");

//used for updating Blynk app
       Blynk.virtualWrite(V1, temperature);
      Blynk.virtualWrite(V0, humidity);
      Blynk.virtualWrite(V4, BPM);
      Blynk.virtualWrite(V3, SpO2);
      Blynk.virtualWrite(V2, bodyTemperatureC);
        Serial.println("*********************************");
        Serial.println();
        delay(10000); 
    }
}

void handle_OnConnect() {
    server.send(200, "text/html", SendHTML(humidity, temperature, bodyTemperatureC, bodyTemperatureF, BPM, SpO2));
}

void handle_NotFound() {
    server.send(404, "text/plain", "Not found");
}
//send html response with sensor readings to http client
String SendHTML(float humidity, float temperature, float bodyTemperatureC, float bodyTemperatureF,float BPM,float SpO2) {
    String ptr = "<!DOCTYPE html>";
    ptr += "<html>";
    ptr += "<head>";
    ptr += "<title>REMOTE PATIENT MONITORING</title>";
    ptr += "<meta charset='UTF-8'>";
    ptr += "<style>";
    ptr += "body {";
    ptr += "    background-color: #e6f2ff;";  // Light blue background color
    ptr += "    font-family: Arial, sans-serif;";
    ptr += "    text-align: center;";         // Center align text
    ptr += "}";
    ptr += ".container {";
    ptr += "    max-width: 800px;";
    ptr += "    margin: 0 auto;";
    ptr += "    display: grid;";
    ptr += "    grid-template-columns: 1fr 1fr;";
    ptr += "    grid-gap: 20px;";
    ptr += "}";
    ptr += ".sensor {";
    ptr += "    background-color: #ffffff;";
    ptr += "    padding: 20px;";
    ptr += "    border-radius: 5px;";
    ptr += "}";
    ptr += ".sensor label {";
    ptr += "    font-weight: bold;";
    ptr += "}";
    ptr += "</style>";
    ptr += "</head>";
    ptr += "<body>";
    ptr += "<h1>REMOTE PATIENT MONITORING</h1>";  // Title centered on the page
    
    ptr += "<div class='container'>";
    ptr += "<div class='sensor'>";
    ptr += "<label>Room Humidity:</label>";
    ptr += "<span>" + String(humidity) + "%</span>";
    ptr += "</div>";
    ptr += "<div class='sensor'>";
    ptr += "<label>Room Temperature:</label>";
    ptr += "<span>" + String(temperature) + "°C</span>";
    ptr += "</div>";
    ptr += "<div class='sensor'>";
    ptr += "<label>Body Temperature in Celsius:</label>";
    ptr += "<span>" + String(bodyTemperatureC) + "°C</span>";
    ptr += "</div>";
    ptr += "<div class='sensor'>";
    ptr += "<label>Body Temperature in Fahrenheit:</label>";
    ptr += "<span>" +  String(bodyTemperatureF) + "°F</span>";
    ptr += "</div>";
    ptr += "<div class='sensor'>";
    ptr += "<label>BPM:</label>";
    ptr += "<span>" +  String(BPM)+"</span>";
    ptr += "</div>";
   
    ptr += "<div class='sensor'>";
    ptr += "<label>SpO2:</label>";
    ptr += "<span>" + String(SpO2) + "%</span>";
    ptr += "</div>";
    ptr += "</div>";
    ptr += "</body>";
    ptr += "</html>";
    return ptr;
}

