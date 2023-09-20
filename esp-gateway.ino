#include <ESP8266WiFi.h>

#define _ESP8266

#include "Base93.hpp"

WiFiServer* server;
const int DIGITAL1 = 5;

//Device configuration in flash
typedef struct _wifi_cfg
{
    char initTag[8];    //0
    char ssid[64];      //8
    char password[64];  //72
    char hasDhcp;       //136
    char ip[16];        //137
    char gateway[16];   //153
    char subNet[16];    //169
    char hostName[16];  //185
    char baudRate[8];   //201
    char port[6];       //209
    char padding[33];   //215
    char endTag[8];     //248
} SWiFiCfg;

constexpr SWiFiCfg _devInfo
{
    "DxWiFiS",
    "ETHEREAL-Test WAP",
    "(9G3lwDBmX%LgW^{LB6-.|W'[<>ZSP",
    '\x01',
    "0.0.0.0",
    "0.0.0.0",
    "0.0.0.0",
    "PW230703160837",
    "19200",
    "25532",
    "-",
    "DxWiFiE"
};

int count=0;

/*
 * Device configuration
 */
void setup() 
{
  pinMode(DIGITAL1, OUTPUT); // GPIO05, Digital Pin D1 (FOR LED)

  //Device gateway serial setup
  Serial.begin(String(_devInfo.baudRate).toInt());
  Serial.println();

  WiFi.mode(WIFI_STA); //Connect to WIFI AP
  WiFi.hostname(_devInfo.hostName); //Set host name
  if (!_devInfo.hasDhcp)
  { //Static IP configuration
    IPAddress iaDev, iaGw, iaSn;

    iaDev.fromString(_devInfo.ip);
    iaGw.fromString(_devInfo.gateway);
    iaSn.fromString(_devInfo.subNet);
    WiFi.config(iaDev, iaGw, iaSn);
  }

  const char* cszPassword = Base93::toBuffer(_devInfo.password).c_str();

  Serial.print("+PASSWORD: ");
  Serial.println(cszPassword);
  WiFi.begin(_devInfo.ssid, cszPassword); //Connect to wifi

  digitalWrite(DIGITAL1, HIGH); //Indicate device's start point
 
  //Wait for connection  
  Serial.println("+CONNECTING");
  while (WiFi.status() != WL_CONNECTED)
  {   
    delay(500);
    Serial.print(".");
    delay(500);
  }

  //Print status
  Serial.println("");
  Serial.print("+CONNECTED: ");
  Serial.println(_devInfo.ssid);
  Serial.print("+IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("+HOSTNAME: ");
  Serial.println(_devInfo.hostName);

  //Create a server port for clients to connect to
  server = new WiFiServer(String(_devInfo.port).toInt());
  server->begin();
  digitalWrite(DIGITAL1, LOW); //Indicate server starting
}

/*
 * IO Processing
 */
void loop()
{
  unsigned long ulCurrent;
  unsigned long ulNow;
  WiFiClient client = server->available();
  boolean uiLed = HIGH;

  if (client) //If a client's got connected
  {    
    ulCurrent = millis();
    Serial.print("+CLIENT: ");
    Serial.println(client.remoteIP());
    while(client.connected())
    { 
      ulNow = millis();
      if (ulNow + 1000 >= ulCurrent)
      { //Blink led every 1s while connected
        ulCurrent = ulNow;
        uiLed = ~uiLed;
        digitalWrite(DIGITAL1, uiLed);
      }
      //While client is connected...
      //all input from serial is transfered to the
      //client and viceversa
      while(client.available() > 0)
        Serial.write(client.read()); 
      //Send Data to connected client
      while(Serial.available() > 0)
        client.write(Serial.read());
    }
    client.stop(); //Release client resources
    Serial.println("-CLIENT"); //No client's connected
  }
}