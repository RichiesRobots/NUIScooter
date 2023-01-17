
/*
Done to turn off the charger for the NUI scooter thus preserving its life when its fully charged

You need

433mhz Power plug (use the code in this project 433mhzShowReceivedCode.ino)

You could also (and welcome to) use a wifi plug instead

1 button

2 leds

1 128x64 oled screen

These are options in effect you could just use the esp32 and a button if you wanrted 

Free to use and alter as you wish

RichiesRobots https://www.youtube.com/@richiesrobots9177

*/



#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <Update.h>
#include <EasyButton.h>
#include <NewRemoteReceiver.h>
#include <NewRemoteTransmitter.h>
#include <ESPmDNS.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define HOST "esp32"

char* WIFI_SSID = "WIFI_SSID";
char* WIFI_PASSWORD = "WIFI_PW";


#define REDLED 17
#define BLUELED 18

char scooterInfoURL[60] = "https://app-api-fk.niu.com/v3/motor_data/index_info\?sn=";
char tokenURL[50] = "https://account-fk.niu.com/appv2/login";
char scooterSerial[40]="SCOOTERS SERIAL NUMBER YOU CAN FIND IN THE APP";

String email = "YOUR NUI USERNAME";
String countryCode = "COUNTRY CODE LIKE PHONE NUMBER COUNTRY CODE";
String password = "NUI PASSWORD";

String TOKEN;

volatile bool isOn = false;

int wifiReconnctCount = 0;

#define PORT 443
#define ONOFFBUTTON 32

EasyButton bOnOffButton(ONOFFBUTTON);

HTTPClient http;
WiFiClient client;

WebServer server(80);

Adafruit_SSD1306 displayStatus(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//CHANGE 433 ID RTO THE ONE OF YOUR RECEIVER ON YOUR PLUG
NewRemoteTransmitter transmitter(18898648, 16, 272);

void initializeWiFi(String initialMessage) {
  displayStatus.clearDisplay();
  displayStatus.display();

  displayStatus.setCursor(1, 0);
  displayStatus.setTextSize(1);
  displayStatus.setTextColor(WHITE);


  displayStatus.print(initialMessage);
  Serial.println(initialMessage);

  displayStatus.setCursor(1, 14);
  displayStatus.setTextSize(1);
  displayStatus.setTextColor(WHITE);

  displayStatus.println(WIFI_SSID);
  displayStatus.display();

  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    if(wifiReconnctCount > 450){
       ESP.restart();
    }
    delay(500);
    Serial.print(".");
    displayStatus.print(".");
    displayStatus.display();
    wifiReconnctCount++;
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  displayStatus.clearDisplay();
  displayStatus.display();
  displayStatus.setCursor(1, 1);
  displayStatus.setTextSize(1);
  displayStatus.setTextColor(WHITE);
  displayStatus.println("WiFi connected.");
  displayStatus.display();

  delay(2000);


  displayStatus.setCursor(1, 12);
  displayStatus.setTextSize(1);
  displayStatus.setTextColor(WHITE);
  displayStatus.println("IP Address");
  displayStatus.setCursor(1, 24);
  displayStatus.setTextSize(1);
  displayStatus.setTextColor(WHITE);
  displayStatus.println(WiFi.localIP());
  displayStatus.display();


 if(initialMessage == "Connecting"){
    delay(15000);
 }
 else{
  delay(3000);
 }
  displayStatus.clearDisplay();
  displayStatus.display();
}

void checkReconnectWifi(){

  // if WiFi is down, try reconnecting every CHECK_WIFI_TIME seconds
  if ((WiFi.status() != WL_CONNECTED)) {
   
    WiFi.disconnect();
    initializeWiFi("Reconnecting To ");
 }
}


String getConfigValue(String value) {
  String httpContent;
  
  checkReconnectWifi(); 

  http.begin(scooterInfoURL);
  
  http.addHeader("token", TOKEN);

  int httpCode = http.GET(); 

  if (httpCode > 0) { //Check for the returning code
  
        httpContent = http.getString();
}else {
  
      Serial.println("Error on HTTP request");
  
}

if(httpContent.indexOf("TOKEN ERROR")!= -1){
  TOKEN = getToken();
  return "Token Expired, getting New Token";
}

httpContent = httpContent.substring(httpContent.indexOf(value + "\":") + value.length() + 2, httpContent.indexOf(",", httpContent.indexOf(value + "\":")));

httpContent.replace("\"","");
httpContent.replace("{","");
httpContent.replace("}","");

return httpContent;

http.end(); //Free the resources
  
}

String getToken(){
  String httpContent;
  checkReconnectWifi(); 
  
  String postData = "account=" + email + "&countryCode=" + countryCode + "&password=" + password;
  
  http.begin(tokenURL);

  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpCode = http.POST(postData); 
  
  httpContent = http.getString();

  http.end();

  httpContent = httpContent.substring(httpContent.indexOf("token\":") + 7, httpContent.indexOf(",", httpContent.indexOf("token\":")));

  httpContent.replace("\"","");
  httpContent.replace("{","");
  httpContent.replace("}","");

  return httpContent;
}


/*
   Login page and web page 
*/

const char* loginIndex =
  "<form name='loginForm'>"
  "<table width='20%' bgcolor='A09F9F' align='center'>"
  "<tr>"
  "<td colspan=2>"
  "<center><font size=4><b>Scooterzilla</b></font></center>"
  "<br>"
  "</td>"
  "<br>"
  "<br>"
  "</tr>"
  "<tr>"
  "<td>Username:</td>"
  "<td><input type='text' size=25 name='userid'><br></td>"
  "</tr>"
  "<br>"
  "<br>"
  "<tr>"
  "<td>Password:</td>"
  "<td><input type='Password' size=25 name='pwd'><br></td>"
  "<br>"
  "<br>"
  "</tr>"
  "<tr>"
  "<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
  "</tr>"
  "</table>"
  "</form>"
  "<script>"
  "function check(form)"
  "{"
  "if(form.userid.value=='admin' && form.pwd.value=='SET PASSWORD!')"
  "{"
  "window.open('/serverIndex')"
  "}"
  "else"
  "{"
  " alert('Error Password or Username')/*displays error message*/"
  "}"
  "}"
  "</script>";

/*
   Server Index Page
*/

const char* serverIndex =
  "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
  "<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
  "<input type='file' name='update'>"
  "<input type='submit' value='Update'>"
  "</form>"
  "<div id='prg'>progress: 0%</div>"
  "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var form = $('#upload_form')[0];"
  "var data = new FormData(form);"
  " $.ajax({"
  "url: '/update',"
  "type: 'POST',"
  "data: data,"
  "contentType: false,"
  "processData:false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var per = evt.loaded / evt.total;"
  "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success:function(d, s) {"
  "console.log('success!')"
  "},"
  "error: function (a, b, c) {"
  "}"
  "});"
  "});"
  "</script>";


void setup(){


Serial.begin(115200);

Wire.begin();

  if (!displayStatus.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }


initializeWiFi("Connecting");


pinMode(REDLED, OUTPUT);
pinMode(BLUELED, OUTPUT);


  if(getConfigValue("Charging")=="1" && getConfigValue("batteryCharging")!= "100"){
    digitalWrite(REDLED,LOW);
    digitalWrite(BLUELED,HIGH);
    transmitter.sendUnit(1, 1);
  }
  else{
    transmitter.sendUnit(1, 0);
    digitalWrite(REDLED,HIGH);
    digitalWrite(BLUELED,LOW);
  }

 bOnOffButton.begin();
 bOnOffButton.onPressed(OnOff);

digitalWrite(REDLED,HIGH);
digitalWrite(BLUELED,LOW);

strcat (scooterInfoURL,scooterSerial);

TOKEN = getToken();

Serial.println(TOKEN);

 /*use mdns for host name resolution*/
  if (!MDNS.begin(HOST)) { //http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
  });
  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  server.begin();


xTaskCreatePinnedToCore(
                    OnOffCheck,   /* Function to implement the task */
                    "OnOffCheck", /* Name of the task */
                    10000,      /* Stack size in words */
                    NULL,       /* Task input parameter */
                    0,          /* Priority of the task */
                    NULL,       /* Task handle. */
                    0);  /* Core where the */

}


void loop(){

server.handleClient();

bool Charging;

if(getConfigValue("Charging")=="1"){
  Charging = true;
}
else{
  Charging = false;
}


if(isOn && getConfigValue("batteryCharging")!= "100"){
  transmitter.sendUnit(1, 1);
}

if(isOn && getConfigValue("batteryCharging")== "100"){
     transmitter.sendUnit(1, 0);
     OnOff();
}

if(!isOn){
  transmitter.sendUnit(1, 0);
}

sendMsg(Charging);

delay(1000);
}

void sendMsg(bool Charging){
 
     displayStatus.clearDisplay();
     displayStatus.setCursor(0, 0); 
     displayStatus.setTextSize(1);
     displayStatus.setTextColor(WHITE);
     displayStatus.print("Scooter is currently ");

        
     
     if(Charging){
        displayStatus.setCursor(27,20); 
        displayStatus.print("Charging ");
        displayStatus.print(getConfigValue("batteryCharging"));
        displayStatus.print("%");
        displayStatus.setCursor(6, 40);
        displayStatus.print(getConfigValue("leftTime"));
        displayStatus.print(" Hours Remaining.");
    }
     else{
        displayStatus.setCursor(25,20); 
        displayStatus.print("Not Charging.");
        displayStatus.setCursor(19, 40);
        displayStatus.print("Battery at ");
        displayStatus.print(getConfigValue("batteryCharging"));
        displayStatus.print("%");
     }
     displayStatus.display();
     
}

void OnOff(){

if(!isOn){
  digitalWrite(REDLED,LOW);
  digitalWrite(BLUELED,HIGH);
  isOn = true;
}
else{
  digitalWrite(REDLED,HIGH);
  digitalWrite(BLUELED,LOW);
  isOn = false;
}
}

void OnOffCheck(void * pvParameters){
for(;;){
bOnOffButton.read();
}
}
