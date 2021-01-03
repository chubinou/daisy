/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "plugin.hpp"

void fail(const std::string& msg) {
    patch.display.Fill(false);
    patch.display.SetCursor(0, 0);
    patch.display.WriteString((char*)msg.c_str(), Font_7x10, true);
    patch.display.Update();
    asm("bkpt 255");
}


void warn(const std::string& msg) {
    patch.display.Fill(false);
    patch.display.SetCursor(0, 0);
    patch.display.WriteString((char*)msg.c_str(), Font_7x10, true);
    patch.display.Update();
    patch.DelayMs(1000);
}
