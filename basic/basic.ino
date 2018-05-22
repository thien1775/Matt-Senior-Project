
//************************************************************
// this is a simple example that uses the painlessMesh library
//
// 1. sends a silly message to every node on the mesh at a random time betweew 1 and 5 seconds
// 2. prints anything it recieves to Serial.print
//
//
//************************************************************
#include "painlessMesh.h"
#include <ArduinoJson.h>


#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

void sendMessage() ; // Prototype so PlatformIO doesn't complain

painlessMesh  mesh;
Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );

void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
  //print out people in the list not includeing yourself
  SimpleList<uint32_t> temp = mesh.getNodeList();
  for (SimpleList<uint32_t>::iterator itr = temp.begin(); itr != temp.end();){
    Serial.println(*itr);
    ++itr;
  }
}

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

  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP);  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT, STA_AP, WIFI_AUTH_WPA2_PSK, 6 );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  mesh.scheduler.addTask( taskSendMessage );
  taskSendMessage.enable() ;
}

void loop() {
  mesh.update();
  
}

void sendMessage() {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root  = jsonBuffer.createObject();
  root["topic"] = "firefighter";
  JsonObject& message = root.createNestedObject("message");
  message["device_id"] = 1; 
  message["lat"] = 47.24458;
  message["lon"] = -122.437618;
  message["co"] = 20;
  message["timestamp"] = 1243433;
  String jsonStr;
  root.printTo(jsonStr);
  /*String ff = "ff";
  int id = 1;
  float lat = 47.34;
  float lon = 122.23;
  String msg = ff + id + lat + lon + 20.4545;
  //String msg = " firefighter 1 47.24458 -122.437618 20.1243433";*/
  mesh.sendBroadcast( jsonStr );
  taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 5 ));
}
