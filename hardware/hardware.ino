//##################################################################################################################
//##                                      ELET2415 DATA ACQUISITION SYSTEM CODE                                   ##
//##                                                                                                              ##
//##################################################################################################################

// LIBRARY IMPORTS
#include <esp_sleep.h>
#include <math.h>  // https://www.tutorialspoint.com/c_standard_library/math_h.htm 
#include <ctype.h>

// ADD YOUR IMPORTS HERE
#include <FastLED.h>          // FastLED by Daniel Garcia  (WS2812 control)
#include <DHT.h>              // DHT sensor library by Adafruit

#ifndef _WIFI_H 
#include <WiFi.h>
#endif

#ifndef STDLIB_H
#include <stdlib.h>
#endif

#ifndef STDIO_H
#include <stdio.h>
#endif

#ifndef ARDUINO_H
#include <Arduino.h>
#endif 
 
#ifndef ARDUINOJSON_H
#include <ArduinoJson.h>
#endif

// DEFINE VARIABLES
#define ARDUINOJSON_USE_DOUBLE      1 

// DEFINE THE CONTROL PINS FOR THE DHT22 (LED ARRAY INCLUDED)
// ---- PIN DEFINITIONS ----
#define DHTPIN     32
#define DHTTYPE    DHT22

// LED ARRAY (WS2812 / “LED module” = 7 addressable LEDs)
#define LED_PIN     33
#define NUM_LEDS    7

// FastLED config
#define LED_TYPE     WS2812B
#define COLOR_ORDER  GRB

// Default brightness (0–255). You can change this.
static uint8_t gBrightness = 120;

// MQTT CLIENT CONFIG  
static const char* pubtopic      = "620171852";                    // Add your ID number here
static const char* subtopic[]    = {"620171852_sub","/elet2415"};  // Array of Topics(Strings) to subscribe to
static const char* mqtt_server   = "www.yanacreations.com";            // Broker IP address or Domain name as a String 
static uint16_t mqtt_port        = 1883;
// publish to pubtopic

// WIFI CREDENTIALS


const char* ssid         = "gadfa’s iPhone"; // Add your Wi-Fi ssid
const char* password     = "12345678"; // Add your Wi-Fi password 
//const char* ssid       = "MonaConnect";     // Add your Wi-Fi ssid
//const char* password   = ""; // Add your Wi-Fi password 


// TASK HANDLES 
TaskHandle_t xMQTT_Connect          = NULL; 
TaskHandle_t xNTPHandle             = NULL;  
TaskHandle_t xLOOPHandle            = NULL;  
TaskHandle_t xUpdateHandle          = NULL;
TaskHandle_t xButtonCheckeHandle    = NULL;  

// FUNCTION DECLARATION   
void checkHEAP(const char* Name);   // RETURN REMAINING HEAP SIZE FOR A TASK
void initMQTT(void);                // CONFIG AND INITIALIZE MQTT PROTOCOL
unsigned long getTimeStamp(void);   // GET 10 DIGIT TIMESTAMP FOR CURRENT TIME
void callback(char* topic, byte* payload, unsigned int length);
void initialize(void);
bool publish(const char *topic, const char *payload); // PUBLISH MQTT MESSAGE(PAYLOAD) TO A TOPIC
void vButtonCheck( void * pvParameters );
void vUpdate( void * pvParameters );  
bool isNumber(double number);
double square(double value);

/* Declare your functions below */ 
double convert_Celsius_to_fahrenheit(double c);
double convert_fahrenheit_to_Celsius(double f);
double calcHeatIndex(double Temp, double Humid);
static void setAll(uint8_t r, uint8_t g, uint8_t b);

/* Init class Instances for the DHT22 etc */
DHT dht(DHTPIN, DHTTYPE);

// FastLED LED buffer
CRGB leds[NUM_LEDS];

//############### IMPORT HEADER FILES ##################
#ifndef NTP_H
#include "NTP.h"
#endif

#ifndef MQTT_H
#include "mqtt.h"
#endif

// Temporary Variables 

void setup() {
  Serial.begin(115200);  // INIT SERIAL  
  
  // INITIALIZE ALL SENSORS AND DEVICES
  dht.begin();

  // FastLED init
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(gBrightness);
  setAll(0, 0, 0); // start OFF

  /* Add all other necessary sensor Initializations and Configurations here */

  initialize();     // INIT WIFI, MQTT & NTP 
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // vButtonCheckFunction(); // UNCOMMENT IF USING BUTTONS INT THIS LAB, THEN ADD LOGIC FOR INTERFACING WITH BUTTONS IN THE vButtonCheck FUNCTION
}

void loop() {

  
    // put your main code here, to run repeatedly:       
    vTaskDelay(1000 / portTICK_PERIOD_MS);    
}

//####################################################################
//#                          UTIL FUNCTIONS                          #       
//####################################################################
void vButtonCheck( void * pvParameters )  {
    configASSERT( ( ( uint32_t ) pvParameters ) == 1 );     
      
    for( ;; ) {
        // Add code here to check if a button(S) is pressed
        // then execute appropriate function if a button is pressed  

        vTaskDelay(200 / portTICK_PERIOD_MS);  
    }
}

void vUpdate( void * pvParameters )  {
    configASSERT( ( ( uint32_t ) pvParameters ) == 1 );    
           
    for( ;; ) {
          // #######################################################
          // ## This function must PUBLISH to topic every second. ##
          // #######################################################
   
          StaticJsonDocument<256> doc; // JSON OBJECT CREATED
          char message[256]  = {0};   // ARRAY MESSAGE TO HOLD DATA FROM JSON IN serializeJson(doc, message)

          // 1. Read Humidity and save in variable below
          double h = dht.readHumidity();
           
          // 2. Read temperature as Celsius and save in variable below
          double t = dht.readTemperature();    
          
          if(isNumber(t) && isNumber(h)){
              // ## Publish update according to:
              // ‘{"id": "student_id", "timestamp": 1702212234, "temperature": 30, "humidity":90, "heatindex": 30}’
              doc["id"] = "620171852";
              doc["timestamp"] = (unsigned long)getTimeStamp();
              doc["temperature"] = t;
              doc["humidity"] = h;
              doc["heatindex"] = calcHeatIndex(t, h);

              serializeJson(doc, message);
              
              if(mqtt.connected()){
                publish(pubtopic, message);
              }

              // 1. Create JSon object
              // 2. Create message buffer/array to store serialized JSON object
              // 3. Add key:value pairs to JSon object based on above schema
              // 4. Serialize / Convert JSon object to JSon string and store in message array
              // 5. Publish message to a topic subscribed to by both backend and frontend                
          }
            
        vTaskDelay(1000 / portTICK_PERIOD_MS);  
    }
}

unsigned long getTimeStamp(void) {
          // RETURNS 10 DIGIT TIMESTAMP REPRESENTING CURRENT TIME
          time_t now;         
          time(&now); // Retrieve time[Timestamp] from system and save to &now variable
          return (unsigned long)now;
}

void callback(char* topic, byte* payload, unsigned int length) {
  // ############## MQTT CALLBACK  ######################################
  // RUNS WHENEVER A MESSAGE IS RECEIVED ON A TOPIC SUBSCRIBED TO
  Serial.println("CALLBACK FIRED");
  Serial.printf("RX topic=%s len=%u\n", topic, length);


  Serial.printf("\nMessage received : ( topic: %s ) \n",topic ); 
  char *received = new char[length + 1] {0}; 
  
  for (unsigned int i = 0; i < length; i++) { 
    received[i] = (char)payload[i];    
  }

  // PRINT RECEIVED MESSAGE
  Serial.printf("Payload : %s \n",received);

  // CONVERT MESSAGE TO JSON
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, received);  

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    delete[] received;
    return;
  }

  // PROCESS MESSAGE
  const char* type = doc["type"]; 

  if (type && strcmp(type, "controls") == 0){
    // 1. EXTRACT ALL PARAMETERS: LEDS, RED, GREEN, BLUE, AND BRIGHTNESS FROM JSON OBJECT
    int ledCount = doc["leds"] | 0;
    int brightness = doc["brightness"] | 0;

    int r = doc["color"]["r"] | 0;
    int g = doc["color"]["g"] | 0;
    int b = doc["color"]["b"] | 0;

    // Clamp values to safe ranges
    if (ledCount < 0) ledCount = 0;
    if (ledCount > NUM_LEDS) ledCount = NUM_LEDS;

    if (brightness < 0) brightness = 0;
    if (brightness > 255) brightness = 255;

    if (r < 0) r = 0; if (r > 255) r = 255;
    if (g < 0) g = 0; if (g > 255) g = 255;
    if (b < 0) b = 0; if (b > 255) b = 255;

    gBrightness = (uint8_t)brightness;
    FastLED.setBrightness(gBrightness);

    // 2. ITERATIVELY, TURN ON LED(s) BASED ON THE VALUE OF LEDS. Ex IF LEDS = 2, TURN ON 2 LED(s)
    // 3. ITERATIVELY, TURN OFF ALL REMAINING LED(s).
    // Lab requires iterating 0..6, using FastLED.show(), and a small vTaskDelay() for smooth execution.
    for(int i = 0; i < NUM_LEDS; i++){
      if(i < ledCount){
        leds[i] = CRGB((uint8_t)r, (uint8_t)g, (uint8_t)b);
      } else {
        leds[i] = CRGB::Black;
      }

      FastLED.show();
      vTaskDelay(30 / portTICK_PERIOD_MS); // small delay for smooth execution
    }
  }

  delete[] received;
}

bool publish(const char *topic, const char *payload){   
     bool res = false;
     try{
        res = mqtt.publish(topic,payload);
        // Serial.printf("\nres : %d\n",res);
        if(!res){
          res = false;
          throw false;
        }
     }
     catch(...){
      Serial.printf("\nError (%d) >> Unable to publish message\n", res);
     }
  return res;
}

//***** Complete the util functions below ******

double convert_Celsius_to_fahrenheit(double c){    
    // CONVERTS INPUT FROM °C TO °F. RETURN RESULTS   
    return (c*9.0/5.0)+32.0;  
}

double convert_fahrenheit_to_Celsius(double f){    
    // CONVERTS INPUT FROM °F TO °C. RETURN RESULT
    return (f-32.0)*5.0/9.0;    
}

double calcHeatIndex(double TempC, double Humid){
  // 1) Clamp humidity to valid range
  if (Humid < 0) Humid = 0;
  if (Humid > 100) Humid = 100;

  // 2) If temp is low, heat index ~ actual temperature (avoid regression nonsense)
  //    NOAA heat index regression is mainly for warm conditions (~>= 26.7C / 80F)
  if (TempC < 26.7) {
    return TempC;  // keep everything in Celsius to match your DB temperature
  }

  // 3) Convert C -> F for the NOAA regression
  double T = convert_Celsius_to_fahrenheit(TempC);
  double R = Humid;

  // 4) NOAA regression gives heat index in Fahrenheit
  double HI_F =
    -42.379
    + 2.04901523*T
    + 10.14333127*R
    - 0.22475541*T*R
    - 0.00683783*T*T
    - 0.05481717*R*R
    + 0.00122874*T*T*R
    + 0.00085282*T*R*R
    - 0.00000199*T*T*R*R;

  // 5) Convert back to Celsius so DB stays consistent
  return convert_fahrenheit_to_Celsius(HI_F);
}


bool isNumber(double number){       
    // A cleaner “is valid number” check for sensor reads
    return (!isnan(number) && !isinf(number));
}

static void setAll(uint8_t r, uint8_t g, uint8_t b){
  for(int i = 0; i < NUM_LEDS; i++){
    leds[i] = CRGB(r, g, b);
  }
  FastLED.show();
}

double square (double value){ return value*value; }
