#include <Arduino.h>
#include <ArduinoJson.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include "time.h"
#include "sntp.h"

const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
const long  gmtOffset_sec = 18000;
const int   daylightOffset_sec = 0;
const char* time_zone = "Asia/Karachi";  // TimeZone rule for Europe/Rome including daylight adjustment rules (optional)
char dateString[30];
char timeString[20];

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define WIFI_SSID "openwifi" //
#define WIFI_PASSWORD "44445555" //

#define API_KEY "API KEY HERE"
#define DATABASE_URL "DATABASE LINK HERE" 

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool signupOK = false;

float pressure;
float humidity;
float temperature; // Celsius
float sunlight;
float smoke; 
String rain;

void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("No time available (yet)");
    return;
  }
  strftime(dateString, sizeof(dateString), "%d %B %Y, %A", &timeinfo);
  strftime(timeString, sizeof(timeString), "%H:%M:%S", &timeinfo);
  Serial.println(dateString);
  Serial.println(timeString);
}

void timeavailable(struct timeval *t)
{
  Serial.println("Got time adjustment from NTP!");
  printLocalTime();
}

void setup() {
  Serial.begin(9600);
  Serial2.begin(9600);

  sntp_set_time_sync_notification_cb( timeavailable );
  //sntp_servermode_dhcp(1);    // (optional)
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  }
  else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Serial.println("Done Setup");
}

void loop() 
{
  Receive();
}

void Receive()
{
  if (Serial2.available())
  {
    StaticJsonDocument<150> doc;
    DeserializationError err = deserializeJson(doc, Serial2);
    if (err == DeserializationError::Ok)
    {
      pressure = doc["p"].as<float>();
      humidity = doc["h"].as<float>();
      temperature = doc["t"].as<float>();
      sunlight = doc["l"].as<float>();
      smoke = doc["s"].as<float>();
      rain = doc["r"].as<String>();

if(rain!="null"){
      Serial.println("pressure:"+String(pressure));
      Serial.println("humidity:"+String(humidity));
      Serial.println("temperature:"+String(temperature));
      Serial.println("sunlight:"+String(sunlight));
      Serial.println("smokeValue:"+String(smoke));
      Serial.println("rainAmount:"+rain);
      printLocalTime();     // it will take some time to sync time :)
      setfirebase();
}
    }
  }
  else {while (Serial2.available()>0){Serial2.read();}}
}

void setfirebase()
{
    if (Firebase.RTDB.setFloat(&fbdo, "weatherData/" + String(dateString) + "/" + String(timeString) + "/pressure", pressure/100)){Serial.println("PASSED");
    Serial.println("PATH: " + fbdo.dataPath());Serial.println("TYPE: " + fbdo.dataType());}
    else {Serial.println("FAILED");Serial.println("REASON: " + fbdo.errorReason());}

    if (Firebase.RTDB.setFloat(&fbdo, "weatherData/" + String(dateString) + "/" + String(timeString) + "/humidity", humidity)){Serial.println("PASSED");
    Serial.println("PATH: " + fbdo.dataPath());Serial.println("TYPE: " + fbdo.dataType());}
    else {Serial.println("FAILED");Serial.println("REASON: " + fbdo.errorReason());}

    if (Firebase.RTDB.setFloat(&fbdo, "weatherData/" + String(dateString) + "/" + String(timeString) + "/temperature", temperature)){Serial.println("PASSED");
    Serial.println("PATH: " + fbdo.dataPath());Serial.println("TYPE: " + fbdo.dataType());}
    else {Serial.println("FAILED");Serial.println("REASON: " + fbdo.errorReason());}

    if (Firebase.RTDB.setFloat(&fbdo, "weatherData/" + String(dateString) + "/" + String(timeString) + "/sunlight", sunlight)){Serial.println("PASSED");
    Serial.println("PATH: " + fbdo.dataPath());Serial.println("TYPE: " + fbdo.dataType());}
    else {Serial.println("FAILED");Serial.println("REASON: " + fbdo.errorReason());}
    
    if (Firebase.RTDB.setFloat(&fbdo, "weatherData/" + String(dateString) + "/" + String(timeString) + "/smoke", smoke)){Serial.println("PASSED");
    Serial.println("PATH: " + fbdo.dataPath());Serial.println("TYPE: " + fbdo.dataType());}
    else {Serial.println("FAILED");Serial.println("REASON: " + fbdo.errorReason());}
    
    if (Firebase.RTDB.setString(&fbdo, "weatherData/" + String(dateString) + "/" + String(timeString) + "/rain", rain)){Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());Serial.println("TYPE: " + fbdo.dataType());}
    else {Serial.println("FAILED");Serial.println("REASON: " + fbdo.errorReason());}



    if (Firebase.RTDB.setFloat(&fbdo, "weather/pressure", pressure/100)){Serial.println("PASSED");
    Serial.println("PATH: " + fbdo.dataPath());Serial.println("TYPE: " + fbdo.dataType());}
    else {Serial.println("FAILED");Serial.println("REASON: " + fbdo.errorReason());}

    if (Firebase.RTDB.setFloat(&fbdo, "weather/humidity", humidity)){Serial.println("PASSED");
    Serial.println("PATH: " + fbdo.dataPath());Serial.println("TYPE: " + fbdo.dataType());}
    else {Serial.println("FAILED");Serial.println("REASON: " + fbdo.errorReason());}

    if (Firebase.RTDB.setFloat(&fbdo, "weather/temperature", temperature)){Serial.println("PASSED");
    Serial.println("PATH: " + fbdo.dataPath());Serial.println("TYPE: " + fbdo.dataType());}
    else {Serial.println("FAILED");Serial.println("REASON: " + fbdo.errorReason());}

    if (Firebase.RTDB.setFloat(&fbdo, "weather/sunlight", sunlight)){Serial.println("PASSED");
    Serial.println("PATH: " + fbdo.dataPath());Serial.println("TYPE: " + fbdo.dataType());}
    else {Serial.println("FAILED");Serial.println("REASON: " + fbdo.errorReason());}
    
    if (Firebase.RTDB.setFloat(&fbdo, "weather/smoke", smoke)){Serial.println("PASSED");
    Serial.println("PATH: " + fbdo.dataPath());Serial.println("TYPE: " + fbdo.dataType());}
    else {Serial.println("FAILED");Serial.println("REASON: " + fbdo.errorReason());}
    
    if (Firebase.RTDB.setString(&fbdo, "weather/rain", rain)){Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());Serial.println("TYPE: " + fbdo.dataType());}
    else {Serial.println("FAILED");Serial.println("REASON: " + fbdo.errorReason());}
    

    if (Firebase.RTDB.setFloat(&fbdo, "Weather/Pressure (hPa)", pressure/100)){Serial.println("PASSED");
    Serial.println("PATH: " + fbdo.dataPath());Serial.println("TYPE: " + fbdo.dataType());}
    else {Serial.println("FAILED");Serial.println("REASON: " + fbdo.errorReason());}

    if (Firebase.RTDB.setFloat(&fbdo, "Weather/Humidity (percent)", humidity)){Serial.println("PASSED");
    Serial.println("PATH: " + fbdo.dataPath());Serial.println("TYPE: " + fbdo.dataType());}
    else {Serial.println("FAILED");Serial.println("REASON: " + fbdo.errorReason());}

    if (Firebase.RTDB.setFloat(&fbdo, "Weather/Temperature (°C)", temperature)){Serial.println("PASSED");
    Serial.println("PATH: " + fbdo.dataPath());Serial.println("TYPE: " + fbdo.dataType());}
    else {Serial.println("FAILED");Serial.println("REASON: " + fbdo.errorReason());}

    if (Firebase.RTDB.setFloat(&fbdo, "Weather/Sunlight (cd)", sunlight)){Serial.println("PASSED");
    Serial.println("PATH: " + fbdo.dataPath());Serial.println("TYPE: " + fbdo.dataType());}
    else {Serial.println("FAILED");Serial.println("REASON: " + fbdo.errorReason());}
    
    if (Firebase.RTDB.setFloat(&fbdo, "Weather/Smoke (ppm)", smoke)){Serial.println("PASSED");
    Serial.println("PATH: " + fbdo.dataPath());Serial.println("TYPE: " + fbdo.dataType());}
    else {Serial.println("FAILED");Serial.println("REASON: " + fbdo.errorReason());}
    
    if (Firebase.RTDB.setString(&fbdo, "Weather/Rain", rain)){Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());Serial.println("TYPE: " + fbdo.dataType());}
    else {Serial.println("FAILED");Serial.println("REASON: " + fbdo.errorReason());}
}
