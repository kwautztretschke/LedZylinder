#ifndef __WIFICREDENTIALS_H__
#define __WIFICREDENTIALS_H__

#include <ESP8266WiFi.h>

namespace wificredentials{
	const String 	ssid = 			"dummerhotspot";
	const String	password = 		"Amogus187";
	const IPAddress mqttServerIp = 	IPAddress(10,42,0,141);
	const String	deviceName = 	"tollerESP";
}


#endif // __WIFICREDENTIALS_H__