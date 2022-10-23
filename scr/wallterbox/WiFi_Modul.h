/*********

*********/

// Log: V1.03: Adding JSON for Shelly 3EM

/*
ESP32: you need to install the ESPAsyncWebServer and the AsyncTCP libraries.
https://github.com/me-no-dev/ESPAsyncWebServer
https://github.com/me-no-dev/AsyncTCP

*/

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

#include "SPIFFS.h"
#include <HTTPClient.h>
#include "AsyncWebConfig.h"


#include <ArduinoJson.h>




#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#define SETUPBUTTON       25
#define BOOTBUTTON        0

#define LED_OK            23
#define LED_FAULT         17
#define LED_ERROR         4



//#define RXD2 13
//#define TXD2 12



// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebConfig conf;

// web.h muss nach AsyncWebServer server(80); eingefügt werden, da es "server.on" verwendet
#include "web.h" // String params = "["

//Variables to save values from HTML form
String apssid;
String appass;
String ssid;
String pass;
String ip;
String gateway;
String DNS;
String subnet;
String powermeter;
String powermeterport;
String maxPhase;


String uname;                                               // Username
String psw;                                                 // Password

String powermarging;                                        // Sicherheitsmarge die ins Netz verbleiben soll, damit keine Wolke zum Strombezug führt

String shelly;

String PowerToCharger;
String PowerFromCharger = "";

String StartSolarCharging = "";

String TooMutch = "";

String DebugMSG = "";


long JSONinterval = 10000;           // interval at which to blink (milliseconds)
unsigned long JSONpreviousMillis = 0;        // will store last time LED was updated

long Einspeiseinterval = 1000;           // interval at which to blink (milliseconds)
unsigned long EinspeisepreviousMillis = 0;        // will store last time LED was updated

long Bezuginterval = 2000;           // interval at which to blink (milliseconds)
unsigned long BezugpreviousMillis = 0;        // will store last time LED was updated

unsigned long chargingpreviousMillis = 0;        // will store last time LED was updated
unsigned long chargeMillis = 0;

long MaxOffChargeTime = 60000;

String AvailablePower ="0";
String UsedPower ="0";
String total_power = "0";

String PowerMeter = "0";

byte Ladefreigabe = 0;
byte BenutzteLadephasen = 0;

long AllgemeineLeistung =0;
long LadeleistungFromCharger = 0;

byte UsedChargingPhase = 0;

byte LadenGestartet = 0;

// ########################### LADE Modi ###########################
#define Solar_Charging              1
#define ECO_Charging                2
#define Night_Charging              3
#define Charging_Disabled           99

byte ChargingType = Charging_Disabled;

unsigned int FullChrgPower = 1380;
// ######################## LADE Modi ENDE #########################

IPAddress localIP;
//IPAddress localIP(192, 168, 1, 200); // hardcoded

// Set your Gateway IP address
IPAddress localGateway;
//IPAddress localGateway(192, 168, 1, 1); //hardcoded
//IPAddress subnet(255, 255, 0, 0);
IPAddress localSubnet;

IPAddress primaryDNS;
IPAddress secondaryDNS;
/*
    Available ESP32 RF power parameters:
    WIFI_POWER_19_5dBm    // 19.5dBm (For 19.5dBm of output, highest. Supply current ~150mA)
    WIFI_POWER_19dBm      // 19dBm
    WIFI_POWER_18_5dBm    // 18.5dBm
    WIFI_POWER_17dBm      // 17dBm
    WIFI_POWER_15dBm      // 15dBm
    WIFI_POWER_13dBm      // 13dBm
    WIFI_POWER_11dBm      // 11dBm
    WIFI_POWER_8_5dBm     //  8dBm
    WIFI_POWER_7dBm       //  7dBm
    WIFI_POWER_5dBm       //  5dBm
    WIFI_POWER_2dBm       //  2dBm
    WIFI_POWER_MINUS_1dBm // -1dBm( For -1dBm of output, lowest. Supply current ~120mA)
    Available ESP8266 RF power parameters:
    0    (for lowest RF power output, supply current ~ 70mA
    20.5 (for highest RF power output, supply current ~ 80mA
*/

// 1380 W - 3680 W
// 2760 W - 7360 W
// 4140 W - 11000 W


void debug(String DB){
 // Hier könnt ihr eure Debug-Routinen reinsetzen um zu sehen was los ist
 
}


void SwitchTo1Ph(){
  if (UsedChargingPhase != 1){  
         debug("SwitchTo1Ph");
          UsedChargingPhase = 1;
          SerialData.Zieladresse = 1;
          SerialData.Wert        = 0;
          SerialData.Befehl        = ONE_PH_CHRG;
          SendData();
          BenutzteLadephasen = 1;
  }
}

void SwitchTo2Ph(){
  if (UsedChargingPhase != 2){
         debug("SwitchTo2Ph");
          UsedChargingPhase = 2;
          SerialData.Zieladresse = 1;
          SerialData.Wert        = 0;
          SerialData.Befehl        = TWO_PH_CHRG;
          SendData();
          BenutzteLadephasen = 2;
  }
}

void SwitchTo3Ph(){
  if (UsedChargingPhase != 3){
         debug("SwitchTo3Ph");
          UsedChargingPhase = 3;
          SerialData.Zieladresse = 1;
          SerialData.Wert        = 0;
          SerialData.Befehl        = TRE_PH_CHRG;
          SendData();
          BenutzteLadephasen = 3;
  }
}

void FullCharging(unsigned int Power){
         debug("FullCharging");
          SerialData.Zieladresse = 1;
          //SerialData.Wert        = data;
          SerialData.Wert          = Power;
          SerialData.Befehl        = DAUER_LADEN;
          SendData();
          Ladefreigabe             = 1;
}

void SolarCharging(){          // Dies kann durch die RFID "Sunshine Charger" aktiviert werden
         debug("SolarCharging");
          SerialData.Zieladresse = 1;
          //SerialData.Wert        = data;
          SerialData.Befehl        = SOLAR_LADEN;
          SendData();
          delay(100);
          SerialData.Zieladresse   = 1;
          SerialData.Befehl        = SOLAR_LEISTUNG;  
          SendData();       
          Ladefreigabe             = 1;
}


void StopCharging(){           // 
         debug("StopCharging");
          UsedChargingPhase = 0;
          SerialData.Zieladresse = 1;
          SerialData.Wert        = 0;
          SerialData.Befehl        = LADEN_STOP;
          SendData();
          Ladefreigabe = 0;
}


// Initialize WiFi
bool initWiFi() {
  
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable   detector

  WiFi.persistent(false);         //Permanentes Schreiben im Flash abschalten http://www.forum-raspberrypi.de/Thread-esp8266-achtung-flash-speicher-schreibzugriff-bei-jedem-aufruf-von-u-a-wifi-begin
  //WiFi.setTxPower(WIFI_POWER_17dBm); // Sets WiFi RF power output
  delay(100);
 
  if(ssid=="" || ip==""){
    Serial.println("Undefined SSID or IP address.");
    return false;
  }

  WiFi.mode(WIFI_STA);
  
  uint16_t maxlen = strlen("wallterbox-") + 20;
  char *fullhostname = new char[maxlen];
  uint8_t mac[6];
  
  WiFi.macAddress(mac);
  snprintf(fullhostname, maxlen, "%s-%02x%02x%02x", "wallterbox-", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], mac[6]);
  WiFi.setHostname(fullhostname); //define hostname
  delete[] fullhostname;
  
  
  localIP.fromString(ip.c_str());
  localGateway.fromString(gateway.c_str());
  localSubnet.fromString(subnet.c_str());
  primaryDNS.fromString(DNS.c_str());
  secondaryDNS.fromString("1.1.1.1");


//  if (!WiFi.config(localIP, primaryDNS, localGateway, localSubnet)){
//  if (!WiFi.config(localIP, localGateway, localSubnet)){
  if (!WiFi.config(localIP, localGateway, localSubnet, primaryDNS, secondaryDNS)){
    Serial.println("STA Failed to configure");
    return false;
  }
  
  WiFi.begin(ssid.c_str(), pass.c_str());
  WiFi.setTxPower(WIFI_POWER_15dBm); // Sets WiFi RF power output
  Serial.println("Connecting to WiFi...");

  unsigned long currentMillis = millis();
  unsigned long previousMillis = currentMillis;



  while(WiFi.status() != WL_CONNECTED) {
    currentMillis = millis();
    if (currentMillis - previousMillis >= 10000) {
      Serial.println("Failed to connect.");
      return false;
    }
  }

  Serial.println(WiFi.localIP());
      
  return true;
}

// Initialize WiFi
bool initWiFiAP() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable   detector

  WiFi.persistent(false);         //Permanentes Schreiben im Flash abschalten http://www.forum-raspberrypi.de/Thread-esp8266-achtung-flash-speicher-schreibzugriff-bei-jedem-aufruf-von-u-a-wifi-begin
  //WiFi.mode(WIFI_AP);
  WiFi.setTxPower(WIFI_POWER_2dBm); // Sets WiFi RF power output
  delay(100);

  if (!WiFi.softAP(apssid.c_str(), appass.c_str() )) {
    Serial.println("AP Failed");
    return false;
  }
   delay(2000);
   
  IPAddress AP_LOCAL_IP(192, 168, 178, 1);
  IPAddress AP_GATEWAY_IP(192, 168, 178, 1);
  IPAddress AP_NETWORK_MASK(255, 255, 0, 0);
  delay(3000);

  Serial.println(apssid);

  Serial.print("AP IP address: ");
  IPAddress IP = WiFi.softAPIP();
  Serial.println(IP);
      
  return true;
}

// Initialize SPIFFS
void initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");
}

// Read File from SPIFFS
String readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if(!file || file.isDirectory()){
    Serial.println("- failed to open file for reading");
    return String();
  }
  
  String fileContent;
  while(file.available()){
    fileContent = file.readStringUntil('\n');
    break;     
  }
  return fileContent;
}

// Write file to SPIFFS
void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- fwrite failed");
  }
}

void readConfigFromFS() {
  apssid            = String(conf.getApName());
  appass            = String(conf.getValue("appwd"));
  ssid              = String(conf.getValue("ssid"));
  pass              = String(conf.getValue("pwd"));
  ip                = String(conf.getValue("IP"));
  gateway           = String(conf.getValue("Gateway"));
  DNS               = String(conf.getValue("DNS"));
  subnet            = String(conf.getValue("Subnetmask"));

  JSONinterval = 2000;
  if (conf.getInt("shellyitervall") == 2) { JSONinterval = 2000;  }
  if (conf.getInt("shellyitervall") == 10) { JSONinterval = 10000;  }
  if (conf.getInt("shellyitervall") == 30) { JSONinterval = 30000;  }
  
  powermeter        = String(conf.getValue("SHELLY"));
  powermarging      = String(conf.getValue("powerMarging"));
  maxPhase          = String(conf.getValue("maxUsedPhase"));
  shelly            = String(conf.getValue("SHELLY"));
  MaxOffChargeTime  = conf.getInt("MaxOffCharging");
  MaxOffChargeTime  = MaxOffChargeTime * 60000;

  StartSolarCharging= String(conf.getValue("StartSolarCharging"));
  http_username     = conf.getValue("http_username");
  http_password     = conf.getValue("http_password");

}


void handleRoot(AsyncWebServerRequest *request) {
  if(!request->authenticate(http_username, http_password)) return request->requestAuthentication();
  
  conf.handleFormRequest(request);
  
  if (request->hasParam("SAVE")) {
    uint8_t cnt = conf.getCount();
    Serial.println("*********** Konfiguration ************");
    for (uint8_t i = 0; i<cnt; i++) {
      Serial.print(conf.getName(i));
      Serial.print(" = ");
      Serial.println(conf.values[i]);
    }
    if (conf.getBool("switch")) Serial.printf("%s %s %i %5.2f \n",
                                conf.getValue("ssid"),
                                conf.getString("continent").c_str(), 
                                conf.getInt("amount"), 
                                conf.getFloat("float"));
  }

}

void runWebServer(){
  
  //LogIn();
  
  CheckLogIn();
  
  server.on("/SolarCharge", HTTP_GET, [](AsyncWebServerRequest *request){
    if(!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    //request->send_P(200, "text/html", index_html, processor);
    request->send(SPIFFS, "/solar_sel.html", "text/html");
    
    ChargingType = Solar_Charging; 

  });

  server.on("/NightCharge", HTTP_GET, [](AsyncWebServerRequest *request){
    if(!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    //request->send_P(200, "text/html", index_html, processor);
    request->send(SPIFFS, "/night_sel.html", "text/html");

    ChargingType = Night_Charging; 
  
  });
  
  server.on("/OffCharge", HTTP_GET, [](AsyncWebServerRequest *request){
    if(!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    //request->send_P(200, "text/html", index_html, processor);
    request->send(SPIFFS, "/index.html", "text/html");

    ChargingType = Charging_Disabled; StopCharging(); delay(1000); 
       
  });    

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.serveStatic("/", SPIFFS, "/"); // Datei aus dem Speicher laden

  server.on("/config",handleRoot);
  
  server.on("/logo.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/logo.png", "image/png");
  });

  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/favicon.png", "image/png");
  });

  // Request for the latest sensor readings
  server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request){
    DynamicJsonDocument doc(1024);

    if (          LadenGestartet == 1){
      doc["chargingpower"]                          = String( UsedPower.toInt() ); 
    }else{
      doc["chargingpower"]                          = String( 0 ); 
    }

    String payload ="";
    serializeJsonPretty(doc, payload);
    request->send(200, "application/json", payload);
  });


  server.on("/params", HTTP_GET, [](AsyncWebServerRequest *request){
 
    int paramsNr = request->params();
    String ReturnMessage = "OK";
 
    for(int i=0;i<paramsNr;i++){
        String Parameter;
        String Value;
        
        AsyncWebParameter* p = request->getParam(i);
        Parameter = p->name();
        Value = p->value();

        if ( (Parameter == "Phase") && (Value == "1") ) { SwitchTo1Ph(); delay(100); }
        if ( (Parameter == "Phase") && (Value == "2") ) { SwitchTo2Ph(); delay(100);}
        if ( (Parameter == "Phase") && (Value == "3") ) { SwitchTo3Ph(); delay(100);}

        if ( (Parameter == "Charging") && (Value == "Solar")) { ChargingType = Solar_Charging; ReturnMessage = "Solar Charging"; }
        if ( (Parameter == "Charging") && (Value == "Night")) { ChargingType = Night_Charging; }
        if ( (Parameter == "Charging") && (Value == "Stop")) { ChargingType = Charging_Disabled; StopCharging(); delay(1000); ReturnMessage = "Stop"; }

        //FullChrgPower =
        if (ChargingType == Night_Charging) {
          if ( (Parameter == "Power") && (Value.toInt() >= 1380)) { FullChrgPower = Value.toInt(); FullCharging(FullChrgPower); delay(1000); ReturnMessage = "Full Charging"; }
        }
    }
 
    request->send(200, "text/plain", ReturnMessage);
  });
 
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
    
    digitalWrite(LED_FAULT, HIGH); 
    DynamicJsonDocument doc(1024);
    doc["wallterbox"]["wallterbox_mac"]               = WiFi.macAddress();
    doc["wallterbox"]["uptime"]                       = millis();
    doc["wallterbox"]["acivePhase"]                   = UsedChargingPhase;
    doc["wallterbox"]["powermarging"]                 = powermarging;
    doc["wallterbox"]["maxPhase"]                     = maxPhase;
    doc["wallterbox"]["MaxOffChargeTime"]             = MaxOffChargeTime;
    doc["wallterbox"]["PowerToCharger"]               = PowerToCharger;
    doc["wallterbox"]["PowerFromCharger"]             = LadeleistungFromCharger;
    doc["wallterbox"]["StartSolarCharging"]           = StartSolarCharging;
    doc["wallterbox"]["Ladefreigabe"]                 = Ladefreigabe;
    doc["wallterbox"]["Solar"]                        = TooMutch;
    
    if ( (MaxOffChargeTime - (millis()-chargingpreviousMillis)) >= MaxOffChargeTime ) {
     doc["wallterbox"]["SolarTimeout"]                 = 0;
    }else
    {
     doc["wallterbox"]["SolarTimeout"]                 = MaxOffChargeTime - (millis()-chargingpreviousMillis);
    }
    
    doc["wallterbox"]["DebugMSG"]                     = DebugMSG;

    doc["shelly3em"]["shelly3em_intervall"]          = JSONinterval;
    doc["shelly3em"]["total_power_em"]               = total_power.toInt();
    doc["shelly3em"]["powermeterIP"]                 = powermeter;

    doc["wallterbox"]["config"]["wallterbox_apssid"]            = apssid;
    doc["wallterbox"]["config"]["ssid"]                         = ssid;
    doc["wallterbox"]["config"]["ip"]                           = ip;
    doc["wallterbox"]["config"]["gateway"]                      = gateway;
    doc["wallterbox"]["config"]["subnet"]                       = subnet;    
    doc["wallterbox"]["config"]["DNS"]                          = DNS;    
    
    String payload ="";
    serializeJsonPretty(doc, payload);
    
    request->send(200, "application/json", payload);
    digitalWrite(LED_FAULT, LOW); 
    
  });
  
  
  server.begin();  
}


char* string2char(String command){
    if(command.length()!=0){
        char *p = const_cast<char*>(command.c_str());
        return p;
    }
}


unsigned int getloops = 0;

void serialEventRun(){
      digitalWrite(LED_ERROR, LOW); 
      ReceiveData();

  if (DataValid == 1){
      digitalWrite(LED_ERROR, HIGH); 
      delay(10);
                 
        if ( SerialData.Befehl == LADE_LEISTUNG ) { // 
           LadeleistungFromCharger = SerialData.Wert;
          if (LadeleistungFromCharger <0) {
            UsedPower = String(LadeleistungFromCharger *-1); // For the WebSite
          }else{
            UsedPower = String(LadeleistungFromCharger); // For the WebSite
          }
        }

 
        if ( SerialData.Befehl == ONE_PH_CHRG) { // 
          DebugMSG = "ONE_PH_CHRG";
          debug("ONE_PH_CHRG");          
        }

        if ( SerialData.Befehl == TWO_PH_CHRG) { // 
          DebugMSG = "TWO_PH_CHRG";
          debug("TWO_PH_CHRG");          
        }

        if ( SerialData.Befehl == TRE_PH_CHRG) { // 
          DebugMSG = "TRE_PH_CHRG";
          debug("TRE_PH_CHRG");          
        }

        if ( SerialData.Befehl == LADEN_BEENDET) { // 
         DebugMSG = "LADEN_BEENDET";
          debug("LADEN_BEENDET");          
          LadenGestartet = 0;        
        }
        
        if ( SerialData.Befehl == LADE_TRENNUNG) { // 
         DebugMSG = "LADE_TRENNUNG";
          debug("LADE_TRENNUNG"); 
          LadenGestartet = 0;        
          UsedPower = "0";         
        }        

        if ( SerialData.Befehl == LADE_UNTERBRECHUNG) { // 
         DebugMSG = "LADE_UNTERBRECHUNG";
          debug("LADE_UNTERBRECHUNG");          
          LadenGestartet = 0;        
        }        

        if ( SerialData.Befehl == STANDBY_LADEN) { // 
         DebugMSG = "STANDBY_LADEN";
          debug("STANDBY_LADEN");          
          LadenGestartet = 0;        
        }        

        if ( SerialData.Befehl == LADEN_GESTARTET) { // 
         DebugMSG = "LADEN_GESTARTET";
          debug("LADEN_GESTARTET");  
          LadenGestartet = 1;        
        }        

        if ( SerialData.Befehl == LADE_VERBINDUNG) { // 
         DebugMSG = "LADE_VERBINDUNG";
          debug("LADE_VERBINDUNG");          
        }  

  }

}

void StartChargeCounter(){
  chargeMillis = millis();
  chargingpreviousMillis = chargeMillis;
}

void StopChargeCounter(){
   chargeMillis = millis();
   if (chargeMillis - chargingpreviousMillis >= MaxOffChargeTime) {
      chargingpreviousMillis = chargeMillis;
      StopCharging();
   }  
}


void checkSolarChargingEinspeisen(){

         if (Ladefreigabe == 1){  
           if ( total_power.toInt() < 0  ) {       // prüfe ob Bezug oder Einspeisen  // StartSolarCharging ist der Wert der überschritten werden muss um das Laden zu starten.
              StartChargeCounter();     
              TooMutch="Ausreichend Solarleistung E1"        ;
           }              
         }
          else
         {
           if ( ((total_power.toInt() + ((UsedChargingPhase*1360)+StartSolarCharging.toInt())) < 0)  ) {       // prüfe ob Bezug oder Einspeisen  // StartSolarCharging ist der Wert der überschritten werden muss um das Laden zu starten.
              StartChargeCounter();     
              Ladefreigabe = 1;
              TooMutch="Ausreichend Solarleistung E0"        ;
           }
         }

  
           if (chargeMillis - chargingpreviousMillis >= MaxOffChargeTime) {         // prüft ob die Zeit abgelaufen ist. Ansonsten Ladewert übertragen
           } 
             else 
           {
                    
                if (Ladefreigabe == 1){  
                     if (PowerToCharger.toInt() < 0 ){
                       SerialData.Wert = PowerToCharger.toInt();
                       SolarCharging();
                     }
                }
                  
                
           }
        
           if (total_power.toInt() > 0)                                             // Sobald vom Netz bezogen wird, wird der Counter ausgelöst.
           { 
              StopChargeCounter();
              TooMutch="zu wenig Solarleistung"        ;
              
           }
}



void checkSolarChargingBezug(){
          
         if (Ladefreigabe == 1){  
           if ( total_power.toInt() < 0  ) {       // prüfe ob Bezug oder Einspeisen  // StartSolarCharging ist der Wert der überschritten werden muss um das Laden zu starten.
              StartChargeCounter();     
              TooMutch="Ausreichend Solarleistung B1"        ;
           }              
         }
          else
         {
           if ( ((total_power.toInt() + ((UsedChargingPhase*1360)+StartSolarCharging.toInt())) < 0)  ) {       // prüfe ob Bezug oder Einspeisen  // StartSolarCharging ist der Wert der überschritten werden muss um das Laden zu starten.
              StartChargeCounter();     
              Ladefreigabe = 1;
              TooMutch="Ausreichend Solarleistung B0"        ;
           }
         }
  
           if (chargeMillis - chargingpreviousMillis >= MaxOffChargeTime) {         // prüft ob die Zeit abgelaufen ist. Ansonsten Ladewert übertragen
           } 
             else 
           {
                     if ( (PowerToCharger.toInt() > 0 ) && ((Ladefreigabe == 1)) ){
                       SerialData.Wert = PowerToCharger.toInt();
                       SolarCharging();
                     }
           }
        
           if (total_power.toInt() > 0)                                             // Sobald vom Netz bezogen wird, wird der Counter ausgelöst.
           { 
              StopChargeCounter();
                TooMutch="zu wenig Solarleistung"        ;
           }
}






void getJSONfromShelly3EM(){

           
      String payload =""; 
      char json[] ="";   
       
    if(WiFi.status()== WL_CONNECTED){
          digitalWrite(LED_OK, LOW); 
          HTTPClient http;
    
          String serverPath = "http://"+ shelly + "/status";
          http.begin(serverPath.c_str());
          int httpResponseCode = http.GET();
          
          if (httpResponseCode>0) {
            payload = http.getString();
          }
          else {
            Serial.print("Error code: ");
            Serial.println(httpResponseCode);
          }
     }
        else {
          Serial.println("WiFi Disconnected");  
          return;
     }

    DynamicJsonDocument doc(4500);
    DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    digitalWrite(LED_ERROR, HIGH); 
    Serial.print(F("deserializeJson() failed1: "));
    Serial.println(error.f_str());
    return;
  }    
     else 
  {
  
        String power = doc["emeters"]["0"]["power"];
        total_power = doc["total_power"].as<String>();

  }
        digitalWrite(LED_OK, HIGH); 
        digitalWrite(LED_ERROR, LOW); 
}



double Input;

void setup() {



  // Serial port for debugging purposes
  Serial.begin(19200, SERIAL_8E1);
  Serial.setRxBufferSize(1024);  // https://stackoverflow.com/questions/60879021/how-to-increase-rx-serial-buffer-size-for-esp32-library-hardwareserial-platform
                                 // https://www.arduinoforum.de/arduino-Thread-Esp32-FIFO-Problem
  //  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);  

  String Copyright = "Copyright by Alexander Walter (c) 2022";

  delay(1000);

  conf.setDescription(params);
  conf.readConfig();

  delay(1000);
  pinMode(SETUPBUTTON, INPUT);
  pinMode(BOOTBUTTON, INPUT);

    pinMode(LED_OK, OUTPUT);
    pinMode(LED_FAULT, OUTPUT);
    pinMode(LED_ERROR, OUTPUT);

    digitalWrite(0, LOW); 
    
    digitalWrite(LED_OK, HIGH); 
    digitalWrite(LED_FAULT, HIGH); 
    digitalWrite(LED_ERROR, HIGH);   

    StopCharging();
    delay(1000);

    initSPIFFS();

    digitalWrite(LED_ERROR, LOW);
    delay(500);
    readConfigFromFS();
      
   if (digitalRead(SETUPBUTTON) == 1){
    digitalWrite(LED_ERROR, LOW);
       
          if(initWiFi()) {
              runWebServer();
              digitalWrite(LED_FAULT, LOW);
          } else {
              //newConfiguration();
              digitalWrite(LED_FAULT, HIGH); 
          }

   }else{
      digitalWrite(LED_ERROR, HIGH);
      digitalWrite(LED_FAULT, LOW);
      digitalWrite(LED_OK, LOW);
      
          if(initWiFiAP()) {
            runWebServer();
            digitalWrite(LED_FAULT, LOW);
          } else {
            //newConfiguration();
            digitalWrite(LED_FAULT, HIGH); 
          }
          
   }
    #ifdef OTA_ENABLE_EVAL
        setupOTA("wallterbox");
    #endif    



  Input = 0;
  Ladefreigabe = 0;
  delay(1000);
  SwitchTo1Ph(); // 1380 W - 3680 W
//    SwitchTo2Ph(); // 2760 W - 7360 W
//    SwitchTo3Ph(); // 4140 W - 11000 W
    
}


long ChDeliverInterval = 10000;           // interval at which to blink (milliseconds)
unsigned long ChDeliverMillis = 0;        // will store last time LED was updated

long ChGetInterval = 2000;           // interval at which to blink (milliseconds)
unsigned long ChGetMillis = 0;        // will store last time LED was updated



void loop() {
   #ifdef OTA_ENABLE_EVAL
      ArduinoOTA.handle(); 
   #endif
    
   if (digitalRead(SETUPBUTTON) == 1){
   }else{
    Serial.println("Startbutton pressed...");
    ESP.restart();
   }

   unsigned long currentMillis = millis();


  if (WiFi.status() == WL_CONNECTED) {

            if (currentMillis - JSONpreviousMillis >= JSONinterval) 
               {
                 JSONpreviousMillis = currentMillis;
                 getJSONfromShelly3EM();
                 Input = total_power.toInt() + powermarging.toInt(); 
                 int InputInt = Input;
                 PowerToCharger= InputInt;
        
                 if (ChargingType == Solar_Charging) {
                             checkSolarChargingBezug();
                 }
     
                 if (ChargingType == Charging_Disabled) { StopCharging();}
                 if (ChargingType == Night_Charging) { FullCharging(FullChrgPower);/* delay(3000); */ }

                 if (Ladefreigabe == 0){  
                         if (total_power.toInt() > 0)                                             // Sobald vom Netz bezogen wird, wird der Counter ausgelöst.
                         { 
                            StopChargeCounter();
                            TooMutch="zu wenig Solarleistung"        ;
                         } 
                         
                         if ( ((total_power.toInt() + ((UsedChargingPhase*1360)+StartSolarCharging.toInt())) < 0)  ) 
                         {
                            TooMutch="Ausreichend Solarleistung 00"        ;
                         }
                 }
              }
        
        
            if (currentMillis - ChDeliverMillis >= ChDeliverInterval) 
               {
                 ChDeliverMillis = currentMillis;
                 if (ChargingType == Solar_Charging) {
                    checkSolarChargingEinspeisen();
                 }
               }

   
       }


  
}
