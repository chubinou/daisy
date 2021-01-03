/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
#include "plugin.hpp"

class Euclidian : public Plugin
{
public:
    const char* name() const override { return "Euclidian"; }

    void init() override;

    void AudioCallback(float**, float**, size_t)  override;

    void refill_pattern();
    void processOled();

    void processInput();

    void processOutput();

    void process() override;

private:
    Parameter stepsParam;
    uint16_t stepsCount = -1;

    Parameter fillParam;
    float fill = -1;

    Parameter rotParam;
    float rot = -1;

    bool pattern[32];
    char strPattern[17];
    uint16_t step;
};
