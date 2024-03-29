/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
#include "plugin.hpp"

class Euclidian : public Plugin
{
public:
    const char* name() const override { return "Euclidian"; }

    void init() override;

    void AudioCallback(const float* const*, float**, unsigned int)  override;

    void refill_pattern();
    void processOled();

    void processInput();

    void processOutput();

    void process() override;

private:
    static const int MAX_STEP = 32;

    Parameter stepsParam;
    uint16_t stepsCount = -1;

    Parameter fillParam;
    float fill = -1;

    Parameter rotParam;
    float rot = -1;

    bool pattern[MAX_STEP];
    char strPattern[MAX_STEP + 1];
    uint16_t step;
};
