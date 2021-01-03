/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "rand.hpp"

void Rand::init()
{
    freqParam.Init(patch.controls[DaisyPatch::CTRL_1], 50, 15000, Parameter::EXPONENTIAL);
    resParam.Init(patch.controls[DaisyPatch::CTRL_2], 0, 1, Parameter::LINEAR);
    probParam.Init(patch.controls[DaisyPatch::CTRL_3], 0, 1, Parameter::LINEAR);
    filter.Init(patch.AudioSampleRate());
}

void Rand::AudioCallback(float**, float** out, size_t size)
{

    for (size_t i = 0; i < size; i++)
    {
        float r = ((float)rand() / (float)(RAND_MAX));
        filter.Process(r);

        out[0][i] = r;
        out[1][i] = filter.Low();
        out[2][i] = filter.Notch();
        out[3][i] = filter.High();
    }
}

void Rand::processOled()
{
    patch.display.Fill(false);
    patch.display.SetCursor(0,0);
    patch.display.WriteString("RAND", Font_7x10, true);
    printSimpleParam(1, "freq", (int)(freq));
    printSimpleParam(2, "res", (int)(res * 100));
    printSimpleParam(3, "prob", (int)(prob * 100));
    printSimpleParam(45, "dacOut", (int)(dacOut));
    patch.display.Update();
}

void Rand::processInput()
{
    float new_freq = freqParam.Process();
    float new_res = resParam.Process();
    prob = probParam.Process();

    if (fabs(new_res - res) > 0.04
            || fabs(new_freq - freq) > 5)
    {
        freq = new_freq;
        res = new_res;
        filter.SetFreq(freq);
        filter.SetRes(res);
        filter.SetDrive(0.1);
    }
}

void Rand::processOutput()
{
    if (patch.gate_input[1].Trig()) {
        if (rand() < (float)RAND_MAX * prob) {
            dsy_gpio_write(&patch.gate_output, 1);
            patch.DelayMs(10);
            dsy_gpio_write(&patch.gate_output, 0);
        }
    }

    if (patch.gate_input[0].Trig()) {
        dacOut = (uint16_t)((4095.f * (float)rand()) / (float)RAND_MAX);
        dsy_dac_write(DSY_DAC_CHN1, (uint16_t)((4095.f * (float)rand()) / (float)RAND_MAX));
        dsy_dac_write(DSY_DAC_CHN2, (uint16_t)((4095.f * (float)rand()) / (float)RAND_MAX));
    }
}

void Rand::process() {
    processInput();
    processOled();
    processOutput();
}
