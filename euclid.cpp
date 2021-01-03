/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "euclid.hpp"

void Euclidian::init()
{
    stepsParam.Init(patch.controls[DaisyPatch::CTRL_1], 0, 32, Parameter::LINEAR);
    fillParam.Init(patch.controls[DaisyPatch::CTRL_2], 0, 1, Parameter::LINEAR);
    rotParam.Init(patch.controls[DaisyPatch::CTRL_3], 0, 1, Parameter::LINEAR);
    strPattern[16] = '\0';
}

void Euclidian::AudioCallback(float**, float**, size_t)
{}

void Euclidian::refill_pattern()
{
    uint16_t pulses = stepsCount * fill;
    uint16_t rotation = stepsCount * rot;
    uint16_t bucket = 0; //out variable to add pulses together for each step

    //fill array with rhythm
    for( int i=rotation ; i < stepsCount + rotation; i++){
        bucket += pulses;
        if(bucket >= stepsCount) {
            bucket -= stepsCount;
            pattern[i % stepsCount] = true;
        } else {
            pattern[i % stepsCount] = false;
        }
    }

    int i = 0;
    for( ; i < stepsCount; i++)
    {
        strPattern[i] = pattern[i] ? 'X' : '.';
    }
    for( ; i < 16; i++)
    {
        strPattern[i] = ' ';
    }
}

void Euclidian::processOled() {
    patch.display.Fill(false);
    patch.display.SetCursor(0,0);
    patch.display.WriteString("EUCLIDIAN", Font_7x10, true);
    printSimpleParam(1, "steps", stepsCount);
    printSimpleParam(2, "fill", (int)(stepsCount * fill));
    printSimpleParam(3, "rot", (int)(stepsCount * rot));
    patch.display.SetCursor(0, 40);
    patch.display.WriteString(strPattern, Font_7x10, true);
    patch.display.SetCursor(7*step, 50);
    patch.display.WriteString("|", Font_7x10, true);

    patch.display.Update();
}

void Euclidian::processInput()
{
    float new_steps_count = stepsParam.Process();
    float new_fill = fillParam.Process();
    float new_rot = rotParam.Process();

    if (new_steps_count != stepsCount
            || -(int)(new_fill * 100) != (int)(fill * 100)
            || -(int)(new_rot * 100 != (int)(rot * 100)))
    {
        stepsCount = new_steps_count;
        fill =  new_fill;
        rot =  new_rot;
        refill_pattern();
    }
}

void Euclidian::processOutput()
{
    if (patch.gate_input[1].Trig()) {
        step = 0;
    }
    if (patch.gate_input[0].Trig()) {
        if (pattern[step]) {
            dsy_gpio_write(&patch.gate_output, 1);
            patch.DelayMs(10);
            dsy_gpio_write(&patch.gate_output, 0);
        }
        step++;
        step %= stepsCount;
    }
}

void Euclidian::process() {
    processInput();
    processOled();
    processOutput();
}
