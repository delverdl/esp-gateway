# esp-gateway

This is an arduino software to create a TCP server socket through a configured ESP8266 device,
in order to work as a WiFi gateway for serial devices. You will need esptool.py software to
get device setting area as the following command line:

```esptool.py -p COM7 -b 460800 read_flash 0x41000 0x1000 ctx_data.bin```

Where *0x41000* is the flash address were setting sector start; you should replace this value
according to your own device flash data. Default baud rate is 460800 bauds (460 kbps). Only
one sector is copied 4k (0x1000).

The following structure is defined to keep device configuration which is read with previous
call to _esptool.py_.

```
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
} SWiFiCfg;
```

Its normal (const) value is the following definition, and after reading *ctx_data.bin*
file you should replace it by your own configuration. If ```SWiFiCfg::hasDhcp``` is equal
to _1_ then you shouldn't configure ```SWiFiCfg::ip```, ```SWiFiCfg::gateway```
neither ```SWiFiCfg::subNet``` members.

```
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
```

In other to properly configure your device, you need to first erase config block,
without reseting, and then write modified block to your device flash memory.

```esptool.py -p COM7 -b 460800 --after no_reset erase_region 0x41000 0x1000```

```esptool.py -p COM7 -b 460800 write_flash 0x41000 ctx_data.bin```
