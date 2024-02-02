#pragma once
#include "sequencePlayer.h"
#include <ESPAsyncWebServer.h>
#include <webServer.h>

namespace HttpServer
{
    void setup(webServer* iotServer, SequencePlayer* sp);
}