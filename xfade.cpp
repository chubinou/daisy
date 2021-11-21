/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "xfade.hpp"

void XFade::init()
{
    position12Param.Init(patch.controls[DaisyPatch::CTRL_1], 0, 1, Parameter::LINEAR);
    position34Param.Init(patch.controls[DaisyPatch::CTRL_2], 0, 1, Parameter::LINEAR);
}

void XFade::AudioCallback(const float * const *in, float** out, unsigned int size)
{
    for (size_t i = 0; i < size; i++)
    {
        out[0][i] = in[0][i] * (1 - position12) + in[1][i] * position12;
        out[1][i] = in[2][i] * (1 - position34) + in[3][i] * position34;
    }
}

void XFade::processOled()
{
    patch.display.Fill(false);
    patch.display.SetCursor(0,0);
    patch.display.WriteString("XFADE", Font_7x10, true);
    printSimpleParam(1, "1 <-> 2", (int)(position12 * 200 - 100));
    printSimpleParam(2, "3 <-> 4", (int)(position34 * 200 - 100));
    patch.display.Update();
}

void XFade::processInput()
{
    position12 = position12Param.Process();
    position34 = position34Param.Process();
}

void XFade::process() {
    processInput();
    processOled();
}
