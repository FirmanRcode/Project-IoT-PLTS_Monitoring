#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Arduino_JSON.h>



const char* ssid = "OPPO A16";
const char* pass = "harun1234";
String host = "103.105.55.180:7779";
const int port = 80;

WiFiClient client;
HTTPClient http;

String pln = "0";
String batt1 = "0";
String batt2 = "0";
String suhu = "0";
String dataNano;
String relay1, relay2,dataGet, dataSendNano;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;

void setup() {
  Serial.begin(115200);
  Wire.begin(0x42);
  wifiConnectHandler = WiFi.onStationModeGotIP(wifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(wifiDisconnect);
  delay(500);
  initWifi();
}

void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim(); // Remove any leading or trailing whitespace
    if (input.length() > 0) {
      host = input;
      Serial.print("Host updated to: ");
      Serial.println(host);
    }
  }
  getData();
  sendSensorData();
  delay(500); // Adjusted delay for more realistic testing
}

void initWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.print("Connected to: ");
  Serial.println(WiFi.localIP());
}

void wifiConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("Successfully connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void wifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("Disconnected from WiFi, attempting to reconnect...");
  WiFi.disconnect();
  initWifi();
}
String httpGETRequest() {
  HTTPClient http;
  http.begin("http://"+host+"/DMP/public/get-relay-data");

  int httpCode = http.GET();
  String payload = "{}";

  if (httpCode > 0) {
    Serial.println("Sukses Get");
    payload = http.getString();
  } else {
    Serial.println("Gagal get");
  }
  http.end();
  return payload;
}

void getData() {
   dataGet = httpGETRequest();

  JSONVar myObject = JSON.parse(dataGet);

  if (JSON.typeof(myObject) == "undefined") {
    Serial.println("Parsing input failed!");
    return;
  }


  if (myObject.hasOwnProperty("relay1")) {
    relay1 = JSON.stringify(myObject["relay1"]);
  }
  if (myObject.hasOwnProperty("relay2")) {
    relay2 = JSON.stringify(myObject["relay2"]);
  }

  Serial.print("relay 1 = ");
  Serial.println(relay1);
  Serial.print("relay 2 = ");
  Serial.println(relay2);
  
    dataSendNano = relay1+relay2;
    Wire.beginTransmission(0x09); // transmit to device #8
    Wire.write(dataSendNano.c_str());              // sends one byte
    Wire.endTransmission();
    delay(500);
}

void sendSensorData() {
  Wire.requestFrom(0x9, 24);  
    dataNano="";
    while(Wire.available()){
      char c = Wire.read();
      dataNano=dataNano+c;
    }
    Serial.print("Data Nano : ");
    Serial.println(dataNano);
    pln = dataNano.substring(0, 6);
    batt1 = dataNano.substring(7, 12);
    batt2 = dataNano.substring(13, 18);
    suhu = dataNano.substring(19, 24);
    
  if (WiFi.status() == WL_CONNECTED) { // Check the current connection status
    HTTPClient http;
    
    // Define your Laravel endpoint URL
    String url = "http://"+host+"/DMP/public/simpan/" + pln + "/" + batt1 + "/" + batt2 + "/" + suhu;
    http.begin(url);

    int httpCode = http.GET(); // Make the GET request
    if (httpCode > 0) { // Check for the returning code
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);
      } else {
        Serial.printf("HTTP GET failed with error code: %d\n", httpCode);
      }
    } else {
      Serial.printf("HTTP GET failed with error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end(); // Free the resources
  } else {
    Serial.println("WiFi not connected");
  }
}
