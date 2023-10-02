#include <ESP8266WiFi.h>

#define _ESP8266

#include "Base93.hpp"

WiFiServer* server;
const int DIGITAL1 = 5;

//Device configuration in flash
typedef struct _wifi_cfg
{
    char initTag[8];    //0   (Dummy tag for memory area start)
    char ssid[64];      //8   (Client WiFi SSID)
    char password[64];  //72  (Client WiFi password)
    char hasDhcp;       //136 (Use DHCP)
    char ip[16];        //137 (Device IP address, works if hasDhcp is 0)
    char gateway[16];   //153 (Device gateway, works if hasDhcp is 0)
    char subNet[16];    //169 (Subnet mask, works if hasDhcp is 0)
    char hostName[16];  //185 (Network host name)
    char baudRate[8];   //201 (Serial port baud rate with 8 bits, no parity, 1 stop bit)
    char port[6];       //209 (TCP port created)
    char padding[33];   //215 (Unused area to guarantee 256 bytes config block)
    char endTag[8];     //248 (Dummy tag for memory area end)
} SWiFiCfg;;

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

  str_t sPass = Base93::toString(_devInfo.password);
  const char* cszPassword = sPass.c_str();

  WiFi.begin(_devInfo.ssid, cszPassword); //Connect to wifi

  digitalWrite(DIGITAL1, HIGH); //Indicate device's start point
 
  //Wait for connection  
  Serial.print("+CONNECTING");
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
  WiFiClient client = server->available();
  boolean uiLed = HIGH;
  long ml;

  if (client) //If a client's got connected
  {    
    Serial.print("+CLIENT: ");
    Serial.println(client.remoteIP());
    while(client.connected())
    { 
      ml = millis() % 1000;
      if (ml < 70) //Every 1000 - 1070 ms
      { //Blink led every 1s while connected
        uiLed = uiLed == HIGH ? LOW : HIGH;
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
