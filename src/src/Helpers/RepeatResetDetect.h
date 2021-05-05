#ifndef HELPERS_REPEATRESET_H
#define HELPERS_REPEATRESET_H

#include <stdint.h>

// Detect if the reset button has been pressed multiple times in short sequence,
// to perform Factory Reset.
//
// inspired by https://github.com/datacute/DoubleResetDetector
//
// requires 2 bytes of RTC storage, passed to methods via reference.
//
// Usage during boots sequence:
// 1) on warm boot, after reading RTC memory, call onWarmBoot(). It detects via
//    resetMarker if last reset occured within the "double-click" timeout
// 2) on warm or cold boots. call markBoot to mark start of "double-click"
//    timeout, then save resetCount and resetMarker to RTC
// 3) after RepeatResetDetect::TIMEOUT elapsed (e.g. periodic 1-sec loop counts),
//    call disarm() to make end of detect timeout, then save resetCount to RTC
//
// At any point call testTrigger() to check if repeat-reset has reached the
// factory reset trigger count

struct RepeatResetDetect
{
    enum
    {
        TIMEOUT = 3, // seconds
        TRIGGERCOUNT = 3
    }; 

    inline static void onWarmBoot(uint8_t& resetCount, const uint8_t& resetMarker)
    {
        resetCount = (resetMarker == 0b01101001) ? resetCount + 1 : 0;
    }

    inline static void markBoot(uint8_t& resetMarker)
    {
        resetMarker = 0b01101001;
    }

    inline static bool testTrigger(const uint8_t& resetCount)
    {
        return resetCount == TRIGGERCOUNT - 1; // trigger on 3rd reset button push, adjust if needed
    }

    inline static void disarm(uint8_t& resetMarker)
    {
        resetMarker = 0; // disarm FactoryReset through repeat-reset 
    }
};

#endif // HELPERS_REPEATRESET_H
