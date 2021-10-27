#include "config.h"
#include <MorseBin.h>
#include <MorseBinNET2E.h>
#ifdef bridgeMode
#include <WiFiNINA.h>
#include "secrets.h"
WiFiServer server(serverPort);
#endif
boolean addresses[255];
unsigned long addressTimeStamp[255];
MorseBinNET2E net("00000001", 2);
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  addresses[0] = true;
  addresses[1] = true;
#ifdef bridgeMode
  WiFi.setHostname(serverHostname);
  int debugInt = WiFi.begin(ssid, password);
  delay(5000);
  Serial.println(debugInt);
  if (debugInt != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    while (1);
  }
  //server.println(clientAddresses);
  server.begin();
#endif
  for (byte i = 0; i < 255; i++) {
    addresses[i] = false;
    addressTimeStamp[i] = millis();
  }
  Serial.println("Setup finished!");
}

void loop() {
  // put your main code here, to run repeatedly:
  addresses[0] = true;
  addresses[1] = true;
  net.receive();
  handleNewDevice();
  handleRefresh();
  checkLease();
#ifdef bridgeMode
  handleBridgeMode();
#endif
}
void handleNewDevice() {
  if (net.lastMessage[0] == "errSuccess") {
    if (net.lastMessage[1] == "00000000") {
      if (net.lastMessage[3] == "00000001" && net.lastMessage[5] == "11111111" && net.lastMessage[6] == "11111111" && net.lastMessage[7] == "11111111" && net.lastMessage[8] == "11111111" && net.lastMessage[9] == "11111111" && net.lastMessage[10] == "11111111") {
        String freeAddress = "";
        if (addresses[MBStringToNum(net.lastMessage[4])] == false) {
          freeAddress = net.lastMessage[4];
          addresses[MBStringToNum(net.lastMessage[4])] = true;
          net.send("00000000", freeAddress, "11111111", "11111111", "11111111", "11111111", "11111111", "11111111", "11111111");
          addressTimeStamp[MBStringToNum(net.lastMessage[4])] = millis();
          Serial.println("Preferred Address Given to Device: " + freeAddress);
          return;
        }
        for (byte i = 2; i < 255; i++) {
          if (addresses[i] == false) {
            freeAddress = MBNumToString(i);
            addressTimeStamp[i] = millis();
            addresses[i] = true;
            break;
          }
        }
        if (freeAddress != "") {
          Serial.println("FoundFreeAddress!");
          Serial.println(freeAddress);
          net.send("00000000", freeAddress, "11111111", "11111111", "11111111", "11111111", "11111111", "11111111", "11111111");
        } else {
          Serial.println("NoFreeAddressFound!");
        }
      }
    }
  } else if (net.lastMessage[0] != "errTimeout") {
    Serial.println("");
  }
}
void checkLease() {
  for (byte i = 2; i < 255; i++) {
    if (addresses[i] == true) {
      if (millis() - addressTimeStamp[i] < 0) {
        addressTimeStamp[i] = millis();
        continue;
      }
      if (millis() - addressTimeStamp[i] > lease) {
        addressTimeStamp[i] = 0;
        addresses[i] = false;
        Serial.println("Purged Adress " + MBNumToString(i));
      }
    }
  }
}
void handleRefresh() {
  if (net.lastMessage[0] == "errSuccess") {
    if (net.lastMessage[3] == "00000010" && net.lastMessage[4] == "11111111" && net.lastMessage[5] == "11111111" && net.lastMessage[6] == "11111111" && net.lastMessage[7] == "11111111" && net.lastMessage[8] == "11111111" && net.lastMessage[9] == "11111111" && net.lastMessage[10] == "11111111") {
      Serial.println("Address Ackknowledge for" + net.lastMessage[1]);
      Ackknowledge();
      addresses[MBStringToNum(net.lastMessage[1])] = true;
      addressTimeStamp[MBStringToNum(net.lastMessage[1])] = millis();
    }
  }
}
void Ackknowledge() {
  net.send(net.lastMessage[1], "00000000", "11111111", "11111111", "11111111", "11111111", "11111111", "11111111", "11111111");
}
#ifdef bridgeMode
void handleBridgeMode() {
  WiFiClient client = server.available();
  String clientMessage = readStringFromClient(client);
  if ((clientMessage == "00000101") && ((!(net.lastMessage[3] == "00000000" && net.lastMessage[4] == "11111111" && net.lastMessage[5] == "11111111" && net.lastMessage[6] == "11111111" && net.lastMessage[7] == "11111111" && net.lastMessage[8] == "11111111" && net.lastMessage[9] == "11111111" && net.lastMessage[10] == "11111111" && net.lastMessage[11] == "11111111") || !(net.lastMessage[3] == "00000001" && net.lastMessage[4] == "11111111" && net.lastMessage[5] == "11111111" && net.lastMessage[6] == "11111111" && net.lastMessage[7] == "11111111" && net.lastMessage[8] == "11111111" && net.lastMessage[9] == "11111111" && net.lastMessage[10] == "11111111") || !(net.lastMessage[3] == "0000010" && net.lastMessage[4] == "11111111" && net.lastMessage[5] == "11111111" && net.lastMessage[6] == "11111111" && net.lastMessage[7] == "11111111" && net.lastMessage[8] == "11111111" && net.lastMessage[9] == "11111111" && net.lastMessage[10] == "11111111")))) {
    client.print("00000010" + net.lastMessage[1] + net.lastMessage[3] + net.lastMessage[4] + net.lastMessage[5] + net.lastMessage[6] + net.lastMessage[7] + net.lastMessage[8] + net.lastMessage[9] + net.lastMessage[10]);
    Serial.println("Sent latest Message to client!");
  } else if (clientMessage == "00000101") {
    client.print("NoMessage");
  }
  if (clientMessage.substring(0, 8) == "00000011") {
    net.send(clientMessage.substring(8, 16), clientMessage.substring(16, 24), clientMessage.substring(24, 32), clientMessage.substring(32, 40), clientMessage.substring(40, 48), clientMessage.substring(48, 56), clientMessage.substring(56, 64), clientMessage.substring(64, 72), clientMessage.substring(72, 80));
  } else if (clientMessage == "00000100") {
    String addressesString = "";
    for (int i = 0; i <= 255; i++) {
      if (addresses[i] == true) {
        addressesString += '1';
      } else if (addresses[i] == false) {
        addressesString += '0';
      }
    }
    Serial.println("Sent addresses to Client!");
    client.print(addressesString);
  }
}
String readStringFromClient(WiFiClient client) {
  String toReturn = "";
  while (client.available()) {
    toReturn += (char)client.read();
  }
  return toReturn;
}
#endif
