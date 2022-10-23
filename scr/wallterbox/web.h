// Canvas Gauge
// https://canvas-gauges.com/documentation/user-guide/

String params = "["
  "{"
  "'name':'appwd',"
  "'label':'Accesspoint Passwort',"
  "'type':"+String(INPUTTEXT)+","
  "'default':'wallterbox'"
  "},"
  
  "{'name':'line','label':'breakline','type':"+String(INPUTLINE)+",'default':''},"
  
  "{"
  "'name':'ssid',"
  "'label':'Name des WLAN',"
  "'type':"+String(INPUTTEXT)+","
  "'default':''"
  "},"
  "{"
  "'name':'pwd',"
  "'label':'WLAN Passwort',"
  "'type':"+String(INPUTPASSWORD)+","
  "'default':''"
  "},"

  "{'name':'line','label':'breakline','type':"+String(INPUTLINE)+",'default':''},"
    
 "{"
  "'name':'IP',"
  "'label':'IP der Wallterbox',"
  "'type':"+String(INPUTTEXT)+","
  "'default':'192.168.178.201'"
  "},"

 "{"
  "'name':'Subnetmask',"
  "'label':'Subnetmask',"
  "'type':"+String(INPUTTEXT)+","
  "'default':'255.255.0.0'"
  "},"
 
 "{"
  "'name':'Gateway',"
  "'label':'Gateway',"
  "'type':"+String(INPUTTEXT)+","
  "'default':'192.168.178.1'"
  "},"

   "{"
  "'name':'DNS',"
  "'label':'DNS',"
  "'type':"+String(INPUTTEXT)+","
  "'default':'192.168.178.1'"
  "},"

  "{'name':'break','label':'breakline','type':"+String(INPUTBREAK)+",'default':''},"
  
  "{'name':'break','label':'breakline','type':"+String(INPUTLINE)+",'default':''},"

   "{"
  "'name':'MaxOffCharging',"
  "'label':'Wie viel Minuten darf das Auto ohne Sonne laden?',"
  "'type':"+String(INPUTNUMBER)+","
  "'min':1,'max':60,"
  "'default':'5'"
  "},"  

  "{"
  "'name':'StartSolarCharging',"
  "'label':'Ab wieviel Überschuss-Watt zur Mindestladeleistung (1360W +?) soll das Solarladen starten?',"
  "'type':"+String(INPUTNUMBER)+","
  "'min':0,'max':20000,"
  "'default':'1000'"
  "},"  
 
 "{"
  "'name':'powerMarging',"
  "'label':'Watt die bei Solarladen ins Netz muss',"
  "'type':"+String(INPUTNUMBER)+","
  "'min':0,'max':11000,"
  "'default':'1000'"
  "},"

  "{'name':'line','label':'breakline','type':"+String(INPUTLINE)+",'default':''},"

   "{"
  "'name':'SHELLY',"
  "'label':'IP des Shelly 3EM',"
  "'type':"+String(INPUTTEXT)+","
  "'default':'192.168.33.1'"
  "},"


  "{'name':'line','label':'breakline','type':"+String(INPUTLINE)+",'default':''},"

   "{'name':'line','label':'breakline','type':"+String(INPUTLINE)+",'default':''}," 
   
  "{"
  "'name':'http_username',"
  "'label':'Username',"
  "'type':"+String(INPUTTEXT)+","
  "'default':'admin'"
  "},"
  "{"
  "'name':'http_password',"
  "'label':'Passwort',"
  "'type':"+String(INPUTPASSWORD)+","
  "'default':''"
  "},"

  "{'name':'line','label':'breakline','type':"+String(INPUTLINE)+",'default':''}" // Last line for }" without ,
  "]";
  
const char* http_username = "admin";
const char* http_password = "";


const char logout_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
</head>
<body>
  <p>Logged out or <a href="/">return to homepage</a>.</p>
  <p><strong>Note:</strong> close all web browser tabs to complete the logout process.</p>
</body>
</html>
)rawliteral";


String outputState(){
/*
  if(digitalRead(output)){
    return "checked";
  }
  else {
    return "";
  }
*/  
  return "";
}


// Replaces placeholder with button section in your web page
String processor(const String& var){
  //Serial.println(var);
  if(var == "BUTTONPLACEHOLDER"){
    String buttons ="";
    String outputStateValue = outputState();
    buttons+= "<p><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"output\" " + outputStateValue + "><span class=\"slider\"></span></label></p>";
    return buttons;
  }

/*  
  if (var == "STATE"){
    if(digitalRead(output)){
      return "ON";
    }
    else {
      return "OFF";
    }
  }
*/  
  return String();
}



  void CheckLogIn(){

   
   // Route for root / web page
  server.on("/i", HTTP_GET, [](AsyncWebServerRequest *request){
    if(!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    //request->send_P(200, "text/html", index_html, processor);
    request->send(SPIFFS, "/index.html", "text/html");
  });

  
  server.on("/logout", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(401);
  });

  server.on("/logged-out", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", logout_html, processor);
  });

  // Send a GET request to <ESP_IP>/update?state=<inputMessage>
  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if(!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    String inputMessage;
    String inputParam;
    // GET input1 value on <ESP_IP>/update?state=<inputMessage>
/*    
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
     // digitalWrite(output, inputMessage.toInt());
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.println(inputMessage);
*/    
    request->send(200, "text/plain", "OK");
  }); 
  }
