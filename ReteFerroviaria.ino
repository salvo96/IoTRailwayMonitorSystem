

// TODO: Configure your WiFi here
#define WIFI_SSID "SSID_HERE"
#define WIFI_PSK  "PASSWORD_HERE"


// We will use wifi
#include <WiFi.h>
#include <vector>
#include <string>
#include <ArduinoJson.h>

// Includes for the server
#include <HTTPServer.hpp>
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>
#include "NTPClient.h"
#include <WiFiUdp.h>

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
String formattedDate;
String dayStamp;
String timeStamp;
bool first = true;


// We create an HTTPServer
using namespace httpsserver;
HTTPServer insecureServer = HTTPServer();

// Declare some handler functions for the various URLs on the server
void handle404(HTTPRequest * req, HTTPResponse * res);
void handleAll1(HTTPRequest * req, HTTPResponse * res);
void handleAll2(HTTPRequest * req, HTTPResponse * res);
void handleId(HTTPRequest * req, HTTPResponse * res);

void setup() {
  // For logging
  Serial.begin(115200);

  // Connect to WiFi
  Serial.println("Setting up WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PSK);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  timeClient.begin();
  timeClient.setTimeOffset(7200);

  // Create a ResourceNode to handle requests from "/all" and "/id"
  // Create a 404 ResourceNode (that would be the deafult node)
  ResourceNode * all1 = new ResourceNode("/all/1", "GET", &handleAll1);
  ResourceNode * all2 = new ResourceNode("/all/2", "GET", &handleAll2);
  ResourceNode * node404  = new ResourceNode("", "GET", &handle404);
  ResourceNode * id  = new ResourceNode("/id", "GET", &handleId);
  insecureServer.registerNode(id);
  insecureServer.registerNode(all1);
  insecureServer.registerNode(all2);

  // We do the same for the default Node
  insecureServer.setDefaultNode(node404);
  Serial.println("Starting HTTP server...");
  insecureServer.start();
  if (insecureServer.isRunning()) {
    Serial.println("Servers ready.");
  }
}

void loop() {
  // We'll call HTTP server loop here
  insecureServer.loop();
  delay(1);
}

void handleId(HTTPRequest * req, HTTPResponse * res){
  res->setHeader("Content-type", "text/html");
  if(first){
    res->println("1");
    first = false;
  }
  else{
    res->println("2");
    first = true;
  }
  
}



void handleAll1(HTTPRequest * req, HTTPResponse * res){
    String response;
    res->setHeader("Content-type", "application/json");
    DynamicJsonDocument jsondata(1024);
    jsondata["ID"] = String("1");
    JsonObject coord  = jsondata.createNestedObject("coordinates");
    JsonObject temps  = jsondata.createNestedObject("temperatures");
    coord["latitude"] = serialized("37.511520");
    coord["longitude"] = serialized("15.084343");
    timeClient.forceUpdate();
    formattedDate = timeClient.getFormattedDate();
 
    jsondata["timestamp"] = String(formattedDate);
    temps["vagone1"] = serialized(String(random(25,50)));
    temps["vagone2"] = serialized(String(random(25,50)));
    jsondata["speed"] = serialized(String(random(50)));
    serializeJson(jsondata, response);
    Serial.println("json data: "+ response);
    res->println(response);
}

void handleAll2(HTTPRequest * req, HTTPResponse * res){
    String response;
    res->setHeader("Content-type", "application/json");
    DynamicJsonDocument jsondata(1024);
    jsondata["ID"] = String("2");
    JsonObject coord  = jsondata.createNestedObject("coordinates");
    JsonObject temps  = jsondata.createNestedObject("temperatures");
    coord["latitude"] = serialized("40.511520");
    coord["longitude"] = serialized("15.084343");
    timeClient.forceUpdate();
    formattedDate = timeClient.getFormattedDate();
 
    jsondata["timestamp"] = String(formattedDate);
    temps["vagone1"] = serialized(String(random(25,50)));
    temps["vagone2"] = serialized(String(random(25,50)));
    jsondata["speed"] = serialized(String(random(50)));
    serializeJson(jsondata, response);
    Serial.println("json data: "+ response);
    res->println(response);
}

void handle404(HTTPRequest * req, HTTPResponse * res) {
  req->discardRequestBody();
  res->setStatusCode(404);
  res->setStatusText("Not Found");
  res->setHeader("Content-Type", "text/html");
  res->println("<!DOCTYPE html>");
  res->println("<html>");
  res->println("<head><title>Not Found</title></head>");
  res->println("<body><h1>404 Not Found</h1><p>The requested resource was not found on this server.</p></body>");
  res->println("</html>");
}
