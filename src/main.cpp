#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <LittleFS.h>

#include <WiFiManager.h>
#include <webServer.h>
#include <updater.h>
#include <fetch.h>
#include <configManager.h>
#include <timeSync.h>

#include "httpserver.h"
#include "sequencePlayer.h"
#include <ESP8266mDNS.h>

SequencePlayer seqPlayer;

void setup()
{
    Serial.begin(115200);

    LittleFS.begin();
    MDNS.begin("lightsequencer");

    GUI.begin();
    configManager.begin();
    WiFiManager.begin("lightsequencer");
    timeSync.begin();

    seqPlayer = SequencePlayer{};
    HttpServer::setup(&GUI, &seqPlayer);
}

void loop()
{
    MDNS.update();
    WiFiManager.loop();
    updater.loop();
    configManager.loop();

    seqPlayer.loop();
}