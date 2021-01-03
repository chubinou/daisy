/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
#include "daisysp.h"
#include "daisy_patch.h"
#include <string>
#include <algorithm>

#include "euclid.hpp"
#include "arp.hpp"
#include "xfade.hpp"
#include "rand.hpp"

DaisyPatch patch;

Arp arp;
Euclidian eucl;
Rand randplug;
XFade xfade;

Plugin* currentPlugin = nullptr;
Plugin* pluginList[] = {
    &arp,
    &eucl,
    &randplug,
    &xfade
};

class MetaPlugin : public Plugin
{
public:
    const char* name() const override { return "META"; }


    void init() override{
        for (unsigned i = 0; i < ARRAY_SIZE(pluginList); i++)
            pluginList[i]->init();
    }

    void AudioCallback(float**, float**, size_t)  override {
    }

    void process()  override {
        int inc = patch.encoder.Increment();
        if ( patch.encoder.RisingEdge() ) {
            std::exchange(currentPlugin, pluginList[idx]);
        } else if (inc > 0) {
            if (idx < ARRAY_SIZE(pluginList) - 1)
                idx++;
        } else if (inc < 0) {
            if (idx > 0)
                idx--;
        }
        patch.display.Fill(false);
        for (unsigned i = 0; i < std::min((unsigned)5, ARRAY_SIZE(pluginList)); i++) {
            patch.display.SetCursor(7, i * 10);
            patch.display.WriteString((char*)pluginList[i]->name(), Font_7x10, true);
        }
        patch.display.SetCursor(0, idx * 10);
        patch.display.WriteChar('>', Font_7x10, true);
        patch.display.Update();
    }

    u_int16_t idx = 0;
};

MetaPlugin meta;

void AudioCallback(float** input, float** output, size_t size)
{
    currentPlugin->AudioCallback(input, output, size);
}

int main(void)
{

    patch.Init(); // Initialize hardware (daisy seed, and patch)

    meta.init();
    currentPlugin = &meta;

    patch.StartAdc();
    patch.StartAudio(AudioCallback);
    while(1)  {
        patch.DebounceControls();
        if (patch.encoder.TimeHeldMs() > 1000) {
            currentPlugin = &meta;
        }
        currentPlugin->process();
    }
}
