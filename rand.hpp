/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef RAND_HPP
#define RAND_HPP

#include "plugin.hpp"

class Rand : public Plugin
{
public:
    const char* name() const override { return "RAND"; }

    void init() override;

    void AudioCallback(float**, float**, size_t)  override;

    void processOled();
    void processInput();
    void processOutput();

    void process() override;

private:
    Parameter freqParam;
    float freq = 0.f;

    Parameter resParam;
    float res = 0.f;

    Parameter probParam;
    float prob = 0.f;

    uint16_t dacOut;

    daisysp::Svf filter;
};

#endif // RAND_HPP
