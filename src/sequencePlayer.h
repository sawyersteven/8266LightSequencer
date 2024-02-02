#pragma once
#include "sequences.h"
#include "speed.h"

enum PlaybackSpeed
{
    SLOW = 2000,
    NORMAL = 1000,
    FAST = 500,
};

typedef struct
{
    int sequenceID;
    Speed::Value speed;
} PlayerStatus;

struct SequencePlayer
{
private:
    Sequence* currentSequence;
    enum LoopState
    {
        Restart,
        Delay,
        Execute
    };
    LoopState currentState = LoopState::Delay;
    Speed::Value currentSpeed = Speed::Value::Normal;
    void applySequence(ushort flag);

public:
    void setNewSequence(int seqID, Speed::Value speed = Speed::Value::Normal);
    PlayerStatus status();
    void loop();
    SequencePlayer();
    static String* getSequenceNames();
};
