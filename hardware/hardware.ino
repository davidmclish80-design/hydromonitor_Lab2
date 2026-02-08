//##################################################################################################################
//##                                      ELET2415 DATA ACQUISITION SYSTEM CODE                                   ##
//##                                                                                                              ##
//##################################################################################################################

// LIBRARY IMPORTS
#include <esp_sleep.h>
#include <math.h>  // https://www.tutorialspoint.com/c_standard_library/math_h.htm 
#include <ctype.h>

// ADD YOUR IMPORTS HERE
#include <Adafruit_NeoPixel.h>
#include <DHT.h>



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

//LED ARRAY
#define NEOPIXEL_PIN 33 //LED ARRAY
#define NUM_PIXELS 7
#define brightness 


// MQTT CLIENT CONFIG  
static const char* pubtopic      = "620171852";                    // Add your ID number here
static const char* subtopic[]    = {"620171852_sub","/elet2415"};  // Array of Topics(Strings) to subscribe to
static const char* mqtt_server   = "www.yanacreations";         // Broker IP address or Domain name as a String 
static uint16_t mqtt_port        = 1883;
//publish too pubtopic


// WIFI CREDENTIALS
//const char* ssid       = "YOUR_SSID";     // Add your Wi-Fi ssid
//const char* password   = "YOUR_PASSWORD"; // Add your Wi-Fi password 
const char* ssid = "ARRIS-F53D"; // Add your Wi-Fi ssid 
const char* password = "70DFF79FF53D"; // Add your Wi-Fi password



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
static void setAll(uint8_t r, uint8_t g, uint_8 b);

/* Init class Instances for the DHT22 etcc */
DHT dht(DHTPIN, DHTTYPE);
adafruit_NeoPixel strip(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800); 
  

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

  strip.begin();
  strip.setBrightness(brightness);
  strip.show()

  
  /* Add all other necessary sensor Initializations and Configurations here */


  initialize();     // INIT WIFI, MQTT & NTP 
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
   
          StaticJsonDocument<256> doc; //JSON OBJECT CREATED

          char message[256]  = {0}; //ARRAY MESSAGE TO HOLD DATA FROM JSON IN serializeJson(Doc, Message)


          // 1. Read Humidity and save in variable below
          double h = dht.readHumidity();
           
          // 2. Read temperature as Celsius   and save in variable below
          double t = dht.readTemperature();    
          

          if(isNumber(t)){
              // ##Publish update according to ‘{"id": "student_id", "timestamp": 1702212234, "temperature": 30, "humidity":90, "heatindex": 30}’
              doc["id"]="620171852";
              doc["timestamp"]=str(getTimeStamp());
              doc["temperature"]= str(t);
              doc["humidity"]= str(h);
              doc["heatindex"]=str(calcHeatIndex(t,h));

              serializeJson(Doc, Message);
              if(mqtt.connected()){ publish(pubtopic, message); }
          

              // 1. Create JSon object
              
              // 2. Create message buffer/array to store serialized JSON object
              
              // 3. Add key:value pairs to JSon object based on above schema

              // 4. Seralize / Covert JSon object to JSon string and store in message array
               
              // 5. Publish message to a topic subscribed to by both backend and frontend                

          }

          
            
        vTaskDelay(1000 / portTICK_PERIOD_MS);  
    }
}

 

unsigned long getTimeStamp(void) {
          // RETURNS 10 DIGIT TIMESTAMP REPRESENTING CURRENT TIME
          time_t now;         
          time(&now); // Retrieve time[Timestamp] from system and save to &now variable
          return now;
}


void callback(char* topic, byte* payload, unsigned int length) {
  // ############## MQTT CALLBACK  ######################################
  // RUNS WHENEVER A MESSAGE IS RECEIVED ON A TOPIC SUBSCRIBED TO
  
  Serial.printf("\nMessage received : ( topic: %s ) \n",topic ); 
  char *received = new char[length + 1] {0}; 
  
  for (int i = 0; i < length; i++) { 
    received[i] = (char)payload[i];    
  }

  // PRINT RECEIVED MESSAGE
  Serial.printf("Payload : %s \n",received);

 
  // CONVERT MESSAGE TO JSON
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, received);  

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }


  // PROCESS MESSAGE
  const char* type = doc["type"]; 

  if (strcmp(type, "controls") == 0){
    // 1. EXTRACT ALL PARAMETERS: NODES, RED,GREEN, BLUE, AND BRIGHTNESS FROM JSON OBJECT

    // 2. ITERATIVELY, TURN ON LED(s) BASED ON THE VALUE OF NODES. Ex IF NODES = 2, TURN ON 2 LED(s)

    // 3. ITERATIVELY, TURN OFF ALL REMAINING LED(s).
   
  }
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
    return (c*9/5)+32;  
}

double convert_fahrenheit_to_Celsius(double f){    
    // CONVERTS INPUT FROM °F TO °C. RETURN RESULT
    return (f-32)*5/9;    
}

double calcHeatIndex(double Temp, double Humid){
    // CALCULATE AND RETURN HEAT INDEX USING EQUATION FOUND AT https://byjus.com/heat-index-formula/#:~:text=The%20heat%20index%20formula%20is,an%20implied%20humidity%20of%2020%25
    double c = -42.379;
    double c2= -2.04901523;
    double c3= -10.14333127;
    double c4= -0.22475541;
    double c5= -6.83783 *0.001;
    double c6= -5.481717*0.01;
    double c7= -1.22874*0.001;
    double c8=  8.5282*0.0001;
    double c9= -1.99*0.000001;

    
    temp=convert_Celsius_to_fahrenheit(Temp);
    double HI= c + c2*temp +c3*humid + c4*temp*humid + c5*square(temp) +c6*square(humid)+ c7*square(temp)*humid + c8*temp*square(humid) + c9*square(temp)*square(humid);
    
    return HI;
}
 

bool isNumber(double number){       
        char item[20];
        snprintf(item, sizeof(item), "%f\n", number);
        if( isdigit(item[0]) )
          return true;
        return false; 
} 

static void setAll(uint8_t r, uint8_t g, uint8_t b){
  for(int i=0, i<NUM_PIXELS; i++){strip.setPixelColor(i, strip.Color(r,g,b));
  strip.show();
  }
}

double square (double value){return value*value;}