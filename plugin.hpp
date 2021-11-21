/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
#ifndef PLUGIN_HPP
#define PLUGIN_HPP

#include <string>
#include <algorithm>
#include "daisysp.h"
#include "daisy_patch.h"

using namespace daisy;
using namespace daisysp;

extern DaisyPatch patch;

void fail(const std::string& msg);
void warn(const std::string& msg);


//defined in main
void pauseAudio();
void resumeAudio();

class Plugin
{
public:
    //startup init
    virtual void init() = 0;

    //return true if the plugin can be loaded
    virtual bool load() { return true; }
    virtual void unload() { }

    virtual void AudioCallback(const float* const*, float**, unsigned int) = 0;
    virtual void process() = 0;
    virtual const char* name() const = 0;

    virtual void setup() {}
    virtual void tearDown() {}

    void printSimpleParam(short line, const char* name, const char* value) {

        std::string str = name;
        str += ": ";
        str += value;
        char* cstr = &str[0];
        patch.display.SetCursor(0, line * 10);
        patch.display.WriteString(cstr, Font_7x10, true);
    }

    void printSimpleParam(short line, const char* name, int value) {
        printSimpleParam(line, name, std::to_string(value).c_str());
    }

};

#define ARRAY_SIZE(array) sizeof(array) / sizeof(*array)

#endif // PLUGIN_HPP
