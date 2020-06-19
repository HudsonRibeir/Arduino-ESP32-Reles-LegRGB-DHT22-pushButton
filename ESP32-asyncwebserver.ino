/*********
  Hudson Ribeiro - https://github.com/HudsonRibeir
*********/

// Import required libraries
#include "WiFi.h" //https://www.arduino.cc/en/Reference/WiFi
#include "ESPAsyncWebServer.h" //https://github.com/me-no-dev/ESPAsyncWebServer/
#include <Adafruit_Sensor.h> //https://github.com/adafruit/Adafruit_Sensor
#include <DHT.h> //https://github.com/adafruit/DHT-sensor-library
#include <ArduinoJson.h> //https://arduinojson.org/

// Replace with your network credentials
const char* ssid     = "SSID";
const char* password = "PASSWORD";

#define DHTPIN 14 // Digital pin connected to the DHT sensor

const int redPin = 22;
const int greenPin = 4;
const int bluePin = 18;

String stateRed = "off";
String stateBlue = "off";
String stateGreen = "off";

const int redPushButton = 32;
const int greenPushButton = 33;
const int bluePushButton = 27;

// Uncomment the type of sensor in use:
//#define DHTTYPE    DHT11     // DHT 11
#define DHTTYPE DHT22 // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

String readDHTTemperature()
{
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  //float t = dht.readTemperature(true);
  // Check if any reads failed and exit early (to try again).
  if (isnan(t))
  {
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else
  {
    Serial.println(t);
    return String(t);
  }
}

String readDHTHumidity()
{
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  if (isnan(h))
  {
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else
  {
    Serial.println(h);
    return String(h);
  }
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>ESP32 DHT Server</h2>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="dht-labels">Temperature</span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span class="dht-labels">Humidity</span>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">%</sup>
  </p>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 10000 ) ;
</script>
</html>)rawliteral";

// Replaces placeholder with DHT values
String processor(const String &var)
{
  //Serial.println(var);
  if (var == "TEMPERATURE")
  {
    return readDHTTemperature();
  }
  else if (var == "HUMIDITY")
  {
    return readDHTHumidity();
  }
  return String();
}

void setup()
{
  // Serial port for debugging purposes
  Serial.begin(115200);

  //Start read temperature and humidity
  dht.begin();

  // Initialize the output variables as outputs
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  // Set outputs to LOW
  digitalWrite(redPin, LOW);
  digitalWrite(greenPin, LOW);
  digitalWrite(bluePin, LOW);

  // Set PushButton pins
  pinMode(redPushButton, INPUT);
  pinMode(greenPushButton, INPUT);
  pinMode(bluePushButton, INPUT);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html, processor);
  });

  //state all
  server.on("/state", HTTP_GET, [](AsyncWebServerRequest *request) {
    DynamicJsonDocument doc(200);

    doc["temperature"] = readDHTTemperature();
    doc["humidity"] = readDHTHumidity();
    doc["led"]["red"] = stateRed;
    doc["led"]["blue"] = stateBlue;
    doc["led"]["green"] = stateGreen;

    String json;
    serializeJson(doc, json);

    request->send_P(200, "application/json", json.c_str());
  });

  //state temperature
  server.on("/temperature/state", HTTP_GET, [](AsyncWebServerRequest *request) {
    DynamicJsonDocument doc(200);

    doc["temperature"] = readDHTTemperature();
    String json;
    serializeJson(doc, json);

    request->send_P(200, "application/json", json.c_str());
  });

  //state humidity
  server.on("/humidity/state", HTTP_GET, [](AsyncWebServerRequest *request) {
    DynamicJsonDocument doc(200);

    doc["humidity"] = readDHTHumidity();
    String json;
    serializeJson(doc, json);

    request->send_P(200, "application/json", json.c_str());
  });

  //state led all
  server.on("/led/state", HTTP_GET, [](AsyncWebServerRequest *request) {
    DynamicJsonDocument doc(200);

    doc["red"] = stateRed;
    doc["blue"] = stateBlue;
    doc["green"] = stateGreen;

    String json;
    serializeJson(doc, json);

    request->send_P(200, "application/json", json.c_str());
  });

  server.on("/led/turn", HTTP_GET, [](AsyncWebServerRequest *request) {
    //get specific header by name
    //Serial.println("led turn is execute");

    if (request->hasHeader("red"))
    {
      AsyncWebHeader *h = request->getHeader("red");
      String redHeader = h->value().c_str();
      Serial.printf("Red Header: %s\n", redHeader);

      if (redHeader == "on")
      {
        digitalWrite(redPin, HIGH);
        stateRed = "on";
        Serial.println("Red turn ON");
      }
      else if (redHeader == "off")
      {
        digitalWrite(redPin, LOW);
        stateRed = "off";
        Serial.println("Red turn OFF");
      }
    }

    if (request->hasHeader("green"))
    {
      AsyncWebHeader *h = request->getHeader("green");
      String greenHeader = h->value().c_str();
      Serial.printf("Green Header: %s\n", greenHeader);

      if (greenHeader == "on")
      {
        digitalWrite(greenPin, HIGH);
        stateGreen = "on";
        Serial.println("Green turn ON");
      }
      else if (greenHeader == "off")
      {
        digitalWrite(greenPin, LOW);
        stateGreen = "off";
        Serial.println("Green turn OFF");
      }
    }

    if (request->hasHeader("blue"))
    {
      AsyncWebHeader *h = request->getHeader("blue");
      String blueHeader = h->value().c_str();
      Serial.printf("Blue Header: %s\n", blueHeader);

      if (blueHeader == "on")
      {
        digitalWrite(bluePin, HIGH);
        stateBlue = "on";
        Serial.println("Blue turn ON");
      }
      else if (blueHeader == "off")
      {
        digitalWrite(bluePin, LOW);
        stateBlue = "off";
        Serial.println("Blue turn OFF");
      }
    }

    DynamicJsonDocument doc(200);

    doc["red"] = stateRed;
    doc["blue"] = stateBlue;
    doc["green"] = stateGreen;

    String json;
    serializeJson(doc, json);

    request->send_P(200, "application/json", json.c_str());
  });

  // Start server
  server.begin();
}

void loop()
{
  // digitalRead function stores the Push button state
  // in variable push_button_state
  int redPushButtonState = digitalRead(redPushButton);
  if (redPushButtonState == HIGH)
  {
    Serial.println("Red button was pressed");
    if (stateRed == "on")
    {
      digitalWrite(redPin, LOW);
      stateRed = "off";
    }
    else
    {
      digitalWrite(redPin, HIGH);
      stateRed = "on";
    }
  }

  int greenPushButtonState = digitalRead(greenPushButton);
  if (greenPushButtonState == HIGH)
  {
    Serial.println("Green button was pressed");
    if (stateGreen == "on")
    {
      digitalWrite(greenPin, LOW);
      stateGreen = "off";
    }
    else
    {
      digitalWrite(greenPin, HIGH);
      stateGreen = "on";
    }
  }

  int bluePushButtonState = digitalRead(bluePushButton);
  if (bluePushButtonState == HIGH)
  {
    Serial.println("Blue button was pressed");
    if (stateBlue == "on")
    {
      digitalWrite(bluePin, LOW);
      stateBlue = "off";
    }
    else
    {
      digitalWrite(bluePin, HIGH);
      stateBlue = "on";
    }
  }
}

/*********
 References:
  - https://randomnerdtutorials.com
  - https://techtutorialsx.com/2018/02/03/esp32-arduino-async-server-controlling-http-methods-allowed/
  - https://learn.adafruit.com/dht
*********/
