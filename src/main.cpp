#include <Arduino.h>
#include <heltec.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "SPIFFS.h"
#include <DNSServer.h>
#include "main.h"

#include "confidential.h"

#define USE_DSN 
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
DNSServer dnsServer;
// Search for parameter in HTTP POST request
const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "pass";
const char* PARAM_INPUT_3 = "ip";

byte selectionWifi = 0;
byte maxItems=3;
byte offset =0;


wifiparamconnexion test2 []=  {{"baptou","test",false},{"doudou","12",false},{"loulou","32",false},{"Quetsch","er",false},{"manou","tr",false}};
wifiparamconnexion * test3 = test2;
//Variables to save values from HTML form
String ssid;
String pass;
String ip;

String processor(const String& var);

class CaptiveRequestHandler : public AsyncWebHandler {
public:
  CaptiveRequestHandler() {}
  virtual ~CaptiveRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request){
    //request->addInterestingHeader("ANY");
    return true;
  }

  void handleRequest(AsyncWebServerRequest *request) {
    request->send(SPIFFS,"/wifimanager.html","text/html",false,processor); 
  }
};

IPAddress IP;

// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

// File paths to save input values permanently
const char* ssidPath = "/ssid.txt";
const char* passPath = "/pass.txt";
const char* ipPath = "/ip.txt";

unsigned long lastScanMillis = 0;

int n;
class menu
{
private:
  /* data */
public:
/**
 * @brief Construct a new menu object
 * 
 * @param maxI 
 * @param maxR 
 */
  menu(int maxI,int maxR);
  ~menu();
  void render(void(*callback)(int));
  void onSelect(void(*callback)(int));
  void onNext();
  int getFirst();
  int getLast();
  int selectNext();
  int selectPrevious();

  
  int maxItems = 0;
  int maxRow =0;
  int select = 0;
  int first=0;
  int last =0;
};

menu::menu(int maxI,int maxR)
{
  maxItems = maxI;
  maxRow = maxR;
  last = maxR;
  Serial.println("maxI: " + String(maxItems));
  Serial.println("maxr: " + String(maxRow));
}

menu::~menu()
{
}
int menu::getFirst(){
  return first;
}
int menu::getLast(){
  return last;
}
int menu::selectNext(){
  select++;
  if (first + select > maxRow-1)
  {
    first++;
    last++;
    
  }
  
  if (select > maxItems-1)
  {
    first=0;
    last=maxRow;
    select=0;
    return 0;
  }
  return select;
  
}
int menu::selectPrevious(){
 
  if (first ==select)
  {
    first--;
    last--;
    
  }
   select--;
  
  if (select <0)
  {
    first=0;
    last=maxRow;
    select=0;
    return 0;
  }
  return select;
  
}

menu menuWifi(5,3);


enum ModeWifi {
  initSoftAp,
  SoftAp,
  SelectWifi,
  PrintSelectedWifi,
  InitSta,
  Sta,
  InitScanWifi,
  ScanWifi
};
ModeWifi MWifi;
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
    Serial.println("- frite failed");
  }
}

String WifiAuthModeToString(wifi_auth_mode_t authMode){
  switch (authMode)
  {
  case WIFI_AUTH_OPEN:
    return "Open";
    break;
  case WIFI_AUTH_WEP:
    return "WEP";
    break;              /**< authenticate mode : WEP */
  case WIFI_AUTH_WPA_PSK:
    return "WPA_PSK";
    break;          /**< authenticate mode : WPA_PSK */
  case WIFI_AUTH_WPA2_PSK:
    return "WPA2_PSK";
    break;         /**< authenticate mode : WPA2_PSK */
  case WIFI_AUTH_WPA_WPA2_PSK:
    return "WPA_WPA2_PS";
    break;     /**< authenticate mode : WPA_WPA2_PSK */
  case WIFI_AUTH_WPA2_ENTERPRISE:
    return "WPA2_ENTERPRISE";
    break;  /**< authenticate mode : WPA2_ENTERPRISE */
  case WIFI_AUTH_MAX:
    return "AUTH_MAX";
    break;
  default:
    return "default";
    break;
  }
}

String processor(const String& var)
{
  Serial.println("processing : " + String(var));
  String retour = String();
  if(var == "liste"){
    for (int i = 0; i < n; i++)
    {
      retour += "<li>";
      //String modeConnect = WifiAuthModeToString(WiFi.encryptionType(i));
      //Serial.printf("%d: %s, Ch:%d (%ddBm) ", i+1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i));
      retour += WiFi.SSID(i).c_str();
      
      retour += "</li>";    
         
    }
    return retour; 
  }
  if (var == "redirect")
  {
    return ip.c_str();
  }
  
    
  return retour;
}


void HandleModeWifi(){
  switch ( MWifi)
  {
  case initSoftAp:
    /* code */
    WiFi.mode(WIFI_AP);
    WiFi.softAP("ESP-WIFI-Manager", NULL);
    IP = WiFi.softAPIP();
    
    server.on("/",HTTP_GET,[](AsyncWebServerRequest *request){
      Serial.println("erer");
      request->send(SPIFFS,"/wifimanager.html","text/html",false,processor);
    });
    server.on("/style.css",HTTP_GET,[](AsyncWebServerRequest * request){
      request->send(SPIFFS,"/style.css","");
    });
    server.on("/ping.js",HTTP_GET,[](AsyncWebServerRequest * request){
      request->send(SPIFFS,"/ping.js","");
    });
    
    server.on("/",HTTP_POST,[](AsyncWebServerRequest *request){
      int params = request->params();
      for (int i = 0; i < params; i++)
      {
        AsyncWebParameter* p = request->getParam(i);
        if (p->isPost())
        {
          if (p->name()== PARAM_INPUT_1)
          {
            ssid = p->value().c_str();
            Serial.print("SSID set to: ");
            Serial.println(ssid);
            writeFile(SPIFFS,ssidPath,ssid.c_str());
          }
          if (p->name() == PARAM_INPUT_2) {
            pass = p->value().c_str();
            Serial.print("Password set to: ");
            Serial.println(pass);
            // Write file to save value
            writeFile(SPIFFS, passPath, pass.c_str());
          }
          // HTTP POST ip value
          if (p->name() == PARAM_INPUT_3) {
            ip = p->value().c_str();
            Serial.print("IP Address set to: ");
            Serial.println(ip);
            // Write file to save value
            writeFile(SPIFFS, ipPath, ip.c_str());
          }
          
        }
        
      }
      //request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip);
      request->send(SPIFFS,"/redirect.html","text/html",false,processor);
      delay(5000);
      ESP.restart();
    });

  #if defined(USE_DSN)
  
    dnsServer.start(53, "*", WiFi.softAPIP());
    server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);//only when requested from AP
  
  #endif // USE_DSN
  
    
    server.begin();
    MWifi = SoftAp;
    break;
  case SoftAp:
  #if defined(USE_DSN)
    dnsServer.processNextRequest();
  
  
  #endif // USE_DSN
  
    //
    Heltec.display->clear();
    Heltec.display->drawString(0,20,"SoftAp: ");
    Heltec.display->drawString(0,40,(String)IP.toString());
    Heltec.display->display();
    break;
  case InitSta:
  Heltec.display->clear();
    WiFi.mode(WIFI_STA);
    IP.fromString(ip.c_str());
    if (!WiFi.config(IP, gateway, subnet,gateway)){
      Serial.println("STA Failed to configure");
      
    }
    WiFi.begin(ssid.c_str(), pass.c_str());
    Serial.println("Connecting to WiFi...");
    Heltec.display->drawString(0,0,"Connecting to WiFi...");
    while(WiFi.status() != WL_CONNECTED) {
      Heltec.display->clear();
      Heltec.display->drawString(0,0,"Connecting to WiFi...");
      static byte point = 0;
      static byte start = 0;
       unsigned long currentMillis = millis();
      static unsigned long previousMillis = 0;
      //Serial.println((String)currentMillis + " " + (String) previousMillis);
      //delay(100);
      if (currentMillis - previousMillis >= 10) {
        previousMillis = currentMillis;
        //Serial.println("Failed to connect.");
        point ++;
        if (point > 128)
        {
          point--;
          start++;
          if (start > 127)
          {
            point = 0;
            start = 0;
          }
          
        }
        if (point > 16)
        {
          start++;
        }
        Serial.println(start);
        
        
      }
      for (size_t i = start; i < point; i++)
        {
          Heltec.display->setPixel(i+1,42);
          
        }
        
      Heltec.display->display();
    }
    IP = WiFi.localIP();
    server.on("/", HTTP_GET , [](AsyncWebServerRequest * request) {
      request->send(200,"ok","text/plain");
    });
    server.begin();


    MWifi = Sta;

    break;
  case Sta:
    Heltec.display->clear();
    Heltec.display->drawString(0,00,"Sta: ");
    Heltec.display->drawString(0,20,(String)WiFi.status());
    Heltec.display->drawString(0,40,(String)WiFi.localIP().toString());
    Heltec.display->display();
    break;
  case InitScanWifi:
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
  break;
  case ScanWifi:
    if (millis() - lastScanMillis > 5000)
    {
      WiFi.scanNetworks(true);
      Serial.print("\nScan start ... ");

      Heltec.display->clear();
      lastScanMillis = millis();

      Heltec.display->drawString(0,0,"Scan Start");
      Heltec.display->display();
      delay(1000);
      
    }

    n = WiFi.scanComplete();
    if(n >= 0)
    {
      Serial.printf("%d network(s) found\n", n);
      Heltec.display->clear();
      for (int i = 0; i < n; i++)
      {
        String modeConnect = WifiAuthModeToString(WiFi.encryptionType(i));
        Serial.printf("%d: %s, Ch:%d (%ddBm) ", i+1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i));
        Serial.println(modeConnect);
                
      }
      MWifi = initSoftAp;
    }
    break;
  case SelectWifi:
    Heltec.display->clear();
    Heltec.display->drawString(0,0,"MENU ");
    static uint32_t previousappui = 0;
    int decalage ;
    decalage = 14;
    //Serial.println((String)digitalRead(0))
    if (digitalRead(0) == LOW )
    {
      if (millis()>previousappui + 500)
      {
        previousappui = millis();
        // selectionWifi ++;
        menuWifi.selectNext();

      }
    }
    if (digitalRead(38) == LOW )
    {
      if (millis()>previousappui + 500)
      {
        previousappui = millis();
        // selectionWifi ++;
        menuWifi.selectPrevious();
      }
    }
    if (digitalRead(37) == LOW){
      test[menuWifi.select].selected = !test[menuWifi.select].selected ;
      ssid = test[menuWifi.select].SSID;
      pass = test[menuWifi.select].Credential;
      ip = "192.168.1.148";


      delay(100);
      MWifi = PrintSelectedWifi;
      break;
    }
    
    
    Serial.println("f " + (String) menuWifi.getFirst() + " L : " + (String)menuWifi.getLast()+ " S : " + (String)menuWifi.select);
    
    for (size_t i = 0; i < menuWifi.maxRow; i++)
    {
      if (test[i+menuWifi.first].selected)
      {
        Heltec.display->fillRect(0,i*12+decalage,120,12);
        Heltec.display->setColor(OLEDDISPLAY_COLOR::BLACK);
      }else
      {
        Heltec.display->setColor(OLEDDISPLAY_COLOR::WHITE);
      }
      
      Heltec.display->drawString(10,i*12+decalage,test[i+menuWifi.first].SSID);
      
      if (menuWifi.select == i+menuWifi.first)
      {
        //Heltec.display->fillRect(2,i*12+2,8,8);
        Heltec.display->fillCircle(6,i*12+decalage+6,3);
      }
      Heltec.display->setColor(OLEDDISPLAY_COLOR::WHITE);
      
    }
    
    Heltec.display->display();
    break;
  case PrintSelectedWifi:
  static unsigned long delai = millis();
    Heltec.display->clear();
    Heltec.display->drawString(0,0,"Sélection: ");
    Heltec.display->drawString(0,15,ssid);
    Heltec.display->display();
    if (delai > millis() +0)
    {
      
    }
    MWifi = ModeWifi::InitSta;
    break;
  default:
    break;
  }
}
void setup() {
  // put your setup code here, to run once:
  Heltec.begin(true,true,true,true,868E6);
  pinMode(0,INPUT_PULLUP);
  pinMode(37,INPUT_PULLUP);
  pinMode(38,INPUT_PULLUP);

  Heltec.display->drawString(0,20,"SPIFS : ");
  if (!SPIFFS.begin(true))
  {
    Heltec.display->drawString(40,20,"Failed");
    Heltec.display->display();
    while (true)
    {
      
    }
    
  } else
  {
     Heltec.display->drawString(40,20,"Ok !");
     Serial.println("Spiff failed");
  }
  
  // Load values saved in SPIFFS
  ssid = readFile(SPIFFS, ssidPath);
  pass = readFile(SPIFFS, passPath);
  ip = readFile(SPIFFS, ipPath);


  if (ssid == "" || ip =="")
  {
    MWifi = ScanWifi;
  } else
  {
    MWifi = InitSta;
  }
  //TODO 
  MWifi = SelectWifi;
  

}

void loop() {
  // put your main code here, to run repeatedly:

  HandleModeWifi();
}