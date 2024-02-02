#include "httpserver.h"
#include <LittleFS.h>
#include <ESPAsyncWebserver.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <AsyncJson.h>

namespace HttpServer
{
    namespace
    {
        SequencePlayer* sp;
        String indexHtml;
        String configHtml;
    }
    /* RPC command handlers

    All return JSON
    On error, the response will be {ok: false, error: String}
    On success, the response will be different based on the command, but
        will always contain {ok: true}
    */

    // Expects json body with {sequenceID: int, speed : int}
    // Returns json {ok: true, sequenceID: int, speed: int}
    void rpcSetDefault(AsyncWebServerRequest* req, JsonObject& jsonObj)
    {
        if (!jsonObj.containsKey("sequenceID"))
        {
            req->send(422, "text/html", "Missing required field: sequenceID");
            return;
        }

        if (!jsonObj.containsKey("speed"))
        {
            req->send(422, "text/html", "Missing required field: speed");
            return;
        }


        Preferences prefs;
        prefs.begin("playerDefaults");

        int seqID = Sequences::Constrain(jsonObj["sequenceID"].as<int>());
        Serial.print("Saving default sequenceID: ");
        Serial.println(seqID);
        prefs.putInt("sequenceID", seqID);

        int speed = Speed::Constrain(jsonObj["speed"].as<int>());
        Serial.print("Saving default sequence speed: ");
        Serial.println(speed);
        prefs.putInt("speed", speed);

        prefs.end();

        AsyncResponseStream* response = req->beginResponseStream("application/json");

        JsonDocument doc;
        doc["ok"] = true;
        doc["sequenceID"] = seqID;
        doc["speed"] = speed;
        serializeJson(doc, *response);
        req->send(response);
    }

    // Expects json body with {sequenceID: int, speed : int}
    // Returns json {ok: true, sequenceID: int, speed: int}
    void rpcSetSequence(AsyncWebServerRequest* req, JsonObject& jsonObj)
    {
        // Get current status then overwrite fields with command fields 
        PlayerStatus status = sp->status();

        int newSeqID = status.sequenceID;
        if (jsonObj.containsKey("sequenceID"))
        {
            newSeqID = jsonObj["sequenceID"].as<int>();
        }

        Speed::Value newSpeed = status.speed;
        if (jsonObj.containsKey("speed"))
        {
            newSpeed = Speed::Constrain(jsonObj["speed"].as<int>());
        }

        sp->setNewSequence(newSeqID, newSpeed);

        AsyncResponseStream* response = req->beginResponseStream("application/json");
        JsonDocument doc;

        status = sp->status();

        doc["ok"] = true;
        doc["sequenceID"] = status.sequenceID;
        doc["speed"] = status.speed;
        serializeJson(doc, *response);
        req->send(response);
    }

    // Expects json body with {ssid: string, password: string, hostname: string}
    // Returns json {ok: true, ssid: string, password: string, hostname: string}
    void rpcSetConfig(AsyncWebServerRequest* req, JsonObject& jsonObj) {
        Preferences prefs;
        prefs.begin("wifi");

        String setSSID;
        if (jsonObj.containsKey("ssid")) {
            setSSID = jsonObj["ssid"].as<String>();
            prefs.putString("ssid", setSSID);
        }
        else {
            setSSID = prefs.getString("ssid", "");
        }

        String setPASSWORD;
        if (jsonObj.containsKey("password")) {
            setPASSWORD = jsonObj["password"].as<String>();
            prefs.putString("password", setPASSWORD);
        }
        else {
            setPASSWORD = prefs.getString("password", "");
        }

        String setHOSTNAME;
        if (jsonObj.containsKey("hostname")) {
            setHOSTNAME = jsonObj["hostname"].as<String>();
            prefs.putString("hostname", setHOSTNAME);
        }
        else {
            setHOSTNAME = prefs.getString("hostname", "");
        }

        AsyncResponseStream* response = req->beginResponseStream("application/json");
        JsonDocument doc;
        doc["ok"] = true;
        doc["ssid"] = setSSID;
        doc["password"] = setPASSWORD;
        doc["hostname"] = setHOSTNAME;
        serializeJson(doc, *response);
        req->send(response);
    }

    void rpcReboot(AsyncWebServerRequest* req, JsonObject& jsonObj) {
        AsyncResponseStream* response = req->beginResponseStream("application/json");
        JsonDocument doc;
        doc["ok"] = true;
        serializeJson(doc, *response);
        req->send(response);
        ESP.restart();
    }

    // Adds all routes and makes changes to IoT Framework's routes
    void setup(webServer* iotServer, SequencePlayer* seqPlayer)
    {
        sp = seqPlayer;

        AsyncWebServer* server = &iotServer->server;

        server->onNotFound([](AsyncWebServerRequest* request) {
            request->send(404);
            });
        server->on("/admin", HTTP_GET, iotServer->requestHandler);

        // Sequences cannot change at runtime, so set the list here
        // This can probably be automated in the build, but this works for now
        File f = LittleFS.open("/static/index.html", "r");
        indexHtml = f.readString();
        f.close();
        indexHtml.replace("~SEQUENCELIST~", *(sp->getSequenceNames()));

        server->on("/", HTTP_GET, [](AsyncWebServerRequest* request)
            {
                request->send(200, "text/html", indexHtml);
            });

        server->on("/static/lit.css", HTTP_GET, [](AsyncWebServerRequest* request)
            {
                request->send(LittleFS, "/static/lit.css", "text/css");
            });

        server->on("/static/script.js", HTTP_GET, [](AsyncWebServerRequest* request)
            {
                request->send(LittleFS, "/static/script.js", "text/javascript");
            });

        server->on("/static/favicon.png", HTTP_GET, [](AsyncWebServerRequest* request)
            {
                request->send(LittleFS, "/static/favicon.png");
            });

        // Returns json {sequenceID : int, speed : float}
        server->on("/status", HTTP_GET, [](AsyncWebServerRequest* req)
            {
                AsyncResponseStream* response = req->beginResponseStream("application/json");
                JsonDocument doc;
                PlayerStatus status = sp->status();
                doc["sequenceID"] = status.sequenceID;
                doc["speed"] = status.speed;
                serializeJson(doc, *response);
                req->send(response); });

        AsyncCallbackJsonWebHandler* rpcHandler = new AsyncCallbackJsonWebHandler("/rpc", [](AsyncWebServerRequest* request, JsonVariant& json)
            {
                JsonObject jsonObj = json.as<JsonObject>();
                JsonVariant cmd = jsonObj["command"];
                if (!jsonObj.containsKey("command")) {
                    request->send(422, "text/html", "Unprocessable Entity");
                    return;
                }

                if (cmd == "setDefault") {
                    rpcSetDefault(request, jsonObj);
                }
                else if (cmd == "setSequence") {
                    rpcSetSequence(request, jsonObj);
                }
                else {
                    request->send(422, "text/html", "Invalid command: " + cmd.as<String>());
                }
            });
        server->addHandler(rpcHandler);
    }
}