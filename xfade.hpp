/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
#include "plugin.hpp"

class XFade : public Plugin
{
public:
    const char* name() const override { return "XFade"; }

    void init() override;

    void AudioCallback(const float* const*, float**, unsigned int)  override;

    void processOled();
    void processInput();

    void process() override;

private:
    Parameter position12Param;
    float position12 = 0.f;

    Parameter position34Param;
    float position34 = 0.f;
};
