#include "sequences.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include "pindata.h"

/*
Sequences are generators of ushort flags representing the on/off state of light
relays. When a sequence is active Next() will be called at regular intervals
and lights set accordingly

Example:
0b0000000000000001 # Only pin 1 is enabled
0b0101010101010101 # Light all odd numbered pins
*/

namespace Sequences
{
    /* Chase Single
    Repeating pattern:
    #...
    .#..
    ..#.
    ...#
    */
    struct : Sequence
    {
        ushort on = 0;

        void reset() override
        {
            on = 0;
            return;
        }

        ushort next() override
        {
            ushort flag = 1 << on;
            on = (on + 1) % PinData::Count;
            return flag;
        }

        String getName() override
        {
            return "Chase Single";
        }
    } Chase_Single;

    /* Chase Double
    Repeating pattern:
    ##..
    .##.
    ..##
    #..#
    */
    struct : Sequence
    {
        ushort on[2]{ 1, 0 };

        void reset() override
        {
            on[0] = 1;
            on[1] = 0;
            return;
        }

        ushort next() override
        {
            ushort flag = (1 << on[1]) + (1 << on[0]);
            on[1] = (on[1] + 1) % PinData::Count;
            on[0] = (on[0] + 1) % PinData::Count;
            return flag;
        }

        String getName() override
        {
            return "Chase Double";
        }
    } Chase_Double;


    /* Wave Single
    Repeating pattern:
    #...
    .#..
    ..#.
    ...#
    ..#.
    .#..
    */
    struct : Sequence
    {
        ushort count_max = (PinData::Count * 2) - 2;
        ushort counter = 0;

        void reset() override
        {
            counter = 0;
            return;
        }

        ushort next() override
        {
            int shift = counter - (counter >= PinData::Count) * (counter - PinData::Count + 1) * 2;
            counter = (counter + 1) % count_max;
            return 1 << shift;
        }

        String getName() override
        {
            return "Wave Single";
        }
    } Wave_Single;


    /* Wave Double
    Repeating pattern:
    ##..
    .##.
    ..##
    .##.
    ##..
    */
    struct : Sequence
    {
        ushort count_max = (PinData::Count * 2) - 4;
        ushort counter = 0;

        void reset() override
        {
            counter = 0;
            return;
        }

        ushort next() override
        {
            int shift = counter - (counter >= PinData::Count - 1) * (counter - PinData::Count + 2) * 2;
            counter = (counter + 1) % count_max;
            return 0b11 << shift;
        }

        String getName() override
        {
            return "Wave Double";
        }
    } Wave_Double;

    /* Random
    */
    struct : Sequence
    {
        void reset() override
        {
            return;
        }

        ushort next() override
        {
            return random(0xFFFF);
        }

        String getName() override
        {
            return "Random";
        }
    } Random;

    /* Alternate
    Repeating pattern:
    #.#.
    .#.#
    */
    struct : Sequence
    {
        ushort flag = 0b0101010101010101;

        void reset() override
        {
            flag = 0b0101010101010101;
            return;
        }

        ushort next() override
        {
            flag = ~flag;
            return flag;
        }
        String getName() override
        {
            return "Alternate";
        }
    } Alternate;

    /* OFF
    ....
    */
    struct : Sequence
    {
        void reset() override
        {
            return;
        }

        ushort next() override
        {
            return 0;
        }

        String getName() override
        {
            return "OFF";
        }
    } OFF;

    /* ON
    ####
    */
    struct : Sequence
    {
        void reset() override
        {
            return;
        }

        ushort next() override
        {
            return 0b1111111111111111;
        }

        String getName() override
        {
            return "ON";
        }
    } ON;

    const int seqCount = 8;
    Sequence* refList[seqCount] = {
        &Chase_Single,
        &Chase_Double,
        &Wave_Single,
        &Wave_Double,
        &Random,
        &Alternate,
        &OFF,
        &ON,
    };

    Sequence* getSequence(int seqInd)
    {
        seqInd = Constrain(seqInd);

        // I don't love this, but this automatically sets the ID field to
        // the Sequence's index in refList. Doing this manually leaves
        // many opportunities for things to go wrong, and I don't want to
        // have to call an init function before using any sequence. All
        // IDs start as -1, but checking that costs more than just re-
        // assigning every time it gets got
        refList[seqInd]->ID = seqInd;

        refList[seqInd]->reset();
        return refList[seqInd];
    };

    String jsonNames;
    // Returns list of names as json-formatted string
    String* getNames()
    {
        if (jsonNames == "")
        {
            JsonDocument doc;
            JsonArray arr = doc.to<JsonArray>();
            for (int i = 0; i < seqCount; i++)
            {
                arr.add(refList[i]->getName());
            }
            serializeJson(doc, jsonNames);
        }
        return &jsonNames;
    }

    // Returns int constrained to the range 0..seqCount
    int Constrain(int i)
    {
        return constrain(i, 0, seqCount);
    }
}