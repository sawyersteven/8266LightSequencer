#include "sequencePlayer.h"
#include "pindata.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <Preferences.h>

// Reads default sequence settings from Preferences
// Returns fallback values if an error occurs
PlayerStatus ReadDefault()
{

    Preferences prefs;
    prefs.begin("playerDefaults");
    int seqID = prefs.getInt("sequenceID", 1);
    int speed = prefs.getInt("speed", Speed::Value::Normal);
    prefs.end();
    Serial.print("Read default sequence: ");
    Serial.println(seqID);
    Serial.print("Read default speed: ");
    Serial.println(speed);
    return PlayerStatus{ Sequences::Constrain(seqID), Speed::Constrain(speed) };
}

SequencePlayer::SequencePlayer()
{
    PlayerStatus ps = ReadDefault();
    setNewSequence(ps.sequenceID, ps.speed);
}

// Change currently playing sequence to seqID @ speed
void SequencePlayer::setNewSequence(int seqID, Speed::Value speed)
{
    seqID = Sequences::Constrain(seqID);

    currentSequence = Sequences::getSequence(seqID);

    Serial.print("Starting sequence: ");
    Serial.println(currentSequence->getName());

    currentSpeed = Speed::Constrain(speed);

    currentState = LoopState::Restart;
}

PlayerStatus SequencePlayer::status()
{
    return { currentSequence->ID, currentSpeed };
}

unsigned long nextExecutionTime = 0;
void SequencePlayer::loop()
{
    unsigned long currentTime = millis();

    switch (currentState)
    {
    case LoopState::Restart:
        nextExecutionTime = 0;
        currentState = LoopState::Execute;
        break;
    case LoopState::Delay:
        if (currentTime >= nextExecutionTime)
        {
            currentState = LoopState::Execute;
        };
        break;
    case LoopState::Execute:
    {
        ushort flag = currentSequence->next();
        applySequence(flag);
        currentState = LoopState::Delay;
        nextExecutionTime = currentTime + currentSpeed;
    }
    break;
    default:
        currentState = LoopState::Restart;
        break;
    }
}

// Sets pin output based on flag
void SequencePlayer::applySequence(ushort flag)
{
    for (int i = 0; i < PinData::Count; i++)
    {
        digitalWrite(PinData::IDs[i], PinData::Trigger ^ !(flag & 1));
        flag >>= 1;
    }
}

// Returns list of names as json-formatted string
String* SequencePlayer::getSequenceNames()
{
    return Sequences::getNames();
}