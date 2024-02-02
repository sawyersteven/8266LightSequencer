#pragma once
#include <Arduino.h>

struct Sequence
{
    int ID = -1;

    virtual ushort next() = 0;
    virtual void reset() = 0;
    virtual String getName() = 0;
};

namespace Sequences
{
    int Constrain(int i);
    Sequence* getSequence(int seqInd);
    String* getNames();
}
