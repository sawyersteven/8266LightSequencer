#include "speed.h"
#include <Arduino.h>

namespace Speed
{
    // Returns Speed::Value closest to given int speed
    Value Constrain(int speed)
    {
        int delta = std::numeric_limits<int>::max();
        Value closest = Value::Normal;

        if (abs(speed - Value::Slow) < delta)
        {
            delta = abs(speed - Value::Slow);
            closest = Value::Slow;
        }
        if (abs(speed - Value::Normal) < delta)
        {
            delta = abs(speed - Value::Normal);
            closest = Value::Normal;
        }
        if (abs(speed - Value::Fast) < delta)
        {
            delta = abs(speed - Value::Fast);
            closest = Value::Fast;
        }
        return closest;
    };
}