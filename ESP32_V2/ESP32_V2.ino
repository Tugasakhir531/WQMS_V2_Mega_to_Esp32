

#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include "time.h"


// define rx tx 
#define RXp2 16
#define TXp2 17


// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "Ali"
#define WIFI_PASSWORD "16385245"

// Insert Firebase project API Key
#define API_KEY "AIzaSyDMvshvjpAT2Di9qMOe1lc1iVCP1Q--fgM"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "user@gmail.com"
#define USER_PASSWORD "user123"

// Insert RTDB URLefine the RTDB URL
#define DATABASE_URL "https://wqms-81d27-default-rtdb.firebaseio.com/"

// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Variable to save USER UID
String uid;

// Database main path (to be updated in setup with the user UID)
String databasePath;
// Database child nodes
String suhuPath = "/suhu";
String tdsPath = "/tds";
String phPath = "/ph";
String kekeruhanPath = "/kekeruhan";
String timePath = "/timestamp";

// Parent Node (to be updated in every loop)
String parentPath;

int timestamp;
FirebaseJson json;

const char* ntpServer = "pool.ntp.org";


//variabel array 
String arrData[4];

// millis sebagai pengganti delay 
unsigned long previousMillis = 0 ;
const long interval = 10000;


// Timer variables (send new readings every three seconds)
unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 60000;

 int tds,kekeruhan;
  float ph,suhu;


bool signupOK = false;

// Initialize WiFi
void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
}

// Function that gets current epoch time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}

void setup(){
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXp2, TXp2);

  // Initialize BME280 sensor
  // initBME();
  initWiFi();
  configTime(0, 0, ntpServer);

  // Assign the api key (required)
  config.api_key = API_KEY;

  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign the RTDB URL (required)
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  // Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);

  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  // // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  // Update database path
  databasePath = "/DataLogging/readings";
}

void loop(){

  // suhu = random(10,100);
  // ph = random(1,14);
  // tds = random(1,1000);
  // kekeruhan= random(1,3000);

  unsigned long currentMillis = millis(); 
  if (currentMillis - previousMillis >= interval){
    previousMillis = currentMillis ;
    String data2 = ""; 
    while(Serial2.available() > 0){
      data2 += char(Serial2.read());
    }
    Serial.print(data2);

    data2.trim();

    if(data2 != ""){
      int index = 0 ;
      for (int i=0; i<=data2.length();i++){
        char delimiter = '#';
        if(data2[i] != delimiter ){
          arrData[index] += data2[i];
        }
        else{
          index++;
        }
      }

      if (index == 3){
        suhu = arrData[0].toInt();
        tds = arrData[1].toInt();
        ph = arrData[2].toFloat();
        kekeruhan = arrData[3].toInt();
        Serial.print("suhu : ");
        Serial.println(suhu);
        Serial.print("PH : ");
        Serial.println(ph);
        Serial.print("TDS : ");
        Serial.println(tds);
        Serial.print("Kekeruhan : ");
        Serial.println(kekeruhan);
        Serial.println("-------------------------------");
      }    
    }
    // request data to Arduino Mega  
    Serial2.println("Ya");

    //Get current timestamp
    timestamp = getTime();
    Serial.print ("time: ");
    Serial.println (timestamp);

    parentPath= databasePath + "/" + String(timestamp);

    json.set(suhuPath.c_str(), suhu);
    json.set(tdsPath.c_str(),tds);
    json.set(phPath.c_str(),ph );
    json.set(kekeruhanPath.c_str(), kekeruhan );
    json.set(timePath, String(timestamp));
    Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());

    if (Firebase.RTDB.setFloat(&fbdo, "WQMS/suhu",suhu)){
            Serial.print("suhu: ");
            Serial.println(suhu);
            
          }
          else {
            Serial.println("FAILED");
            Serial.println("REASON: " + fbdo.errorReason());
          }
          
          // Write an Float number on the database path WQMS/ph
          if (Firebase.RTDB.setFloat(&fbdo, "WQMS/ph", ph)){
            Serial.print("ph: ");
            Serial.println(ph);
          }
          else {
            Serial.println("FAILED");
            Serial.println("REASON: " + fbdo.errorReason());
          }

          // send tds value
          if (Firebase.RTDB.setInt(&fbdo, "WQMS/tds", tds)){
            Serial.print("tds: ");
            Serial.println(tds);
          }
          else {
            Serial.println("FAILED");
            Serial.println("REASON: " + fbdo.errorReason());
          }

          if (Firebase.RTDB.setInt(&fbdo, "WQMS/kekeruhan", kekeruhan)){
            Serial.print("kekeruhan: ");
            Serial.println(kekeruhan);
          }
          else {
            Serial.println("FAILED");
            Serial.println("REASON: " + fbdo.errorReason());
          }

    
      arrData[0] = "";
      arrData[1] = "";
      arrData[2] = "";
      arrData[3] = "";

    

    
    
  }
  // Send new readings to database
  // if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)){
  //   sendDataPrevMillis = millis();

    
       
  // }

  
}
