//************************************************************
// this is a simple example that uses the painlessMesh library to 
// connect to a node on another network. Please see the WIKI on gitlab
// for more details
// https://gitlab.com/painlessMesh/painlessMesh/wikis/bridge-between-mesh-and-another-network
//************************************************************
#include "painlessMesh.h"
#include <ArduinoJson.h>


#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

#define   STATION_SSID     "University of Washington"
#define   STATION_PASSWORD ""
#define   STATION_PORT     5555
#include <AsyncTCP.h>
uint8_t   station_ip[4] =  {10,16,21,247}; // IP of the server
//uint8_t   station_ip[4] =  {192,168,1,173}; // IP of the server
//uint8_t   station_ip[4] =  {192,168,1,101}; // IP of the server

// prototypes
void receivedCallback( uint32_t from, String &msg );
IPAddress getlocalIP();

painlessMesh  mesh;

IPAddress myIP(0,0,0,0);



void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
    Serial.printf("Changed connections %s\n",mesh.subConnectionJson().c_str());
}

void nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void setup() {
  Serial.begin(115200);
  mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION);  // set before init() so that you can see startup messages


  // Channel set to 6. Make sure to use the same channel for your mesh and for you other
  // network (STATION_SSID)
  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT, STA_AP, WIFI_AUTH_WPA2_PSK, 6 );
  mesh.stationManual(STATION_SSID, STATION_PASSWORD, STATION_PORT,
    station_ip);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  //mesh.scheduler.addTask( receivedCallback );
  //taskSendMessage.enable() ;
}

void loop() {
  mesh.update();
  if(myIP != getlocalIP()){
    myIP = getlocalIP();
    Serial.println("My IP is " + myIP.toString());
  }

}

void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("bridge: Received from %u msg=%s\n", from, msg.c_str());
}
IPAddress getlocalIP() {
  return IPAddress(mesh.getStationIP().addr);
}

