/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
#include "daisysp.h"
#include "daisy_patch.h"
#include "sys/fatfs.h"
#include <string>
#include <algorithm>

#include "euclid.hpp"
#include "arp.hpp"
#include "cvrec.hpp"
#include "xfade.hpp"
#include "rand.hpp"
#include "tableenv.h"
#include "subharmonic.hpp"


DaisyPatch patch;
SdmmcHandler sdcard;

Arp arp;
Euclidian eucl;
CVRec cvRec;
Rand randplug;
XFade xfade;
SubHarmonic subHarmonic;
TableEnv tableEnv;

Plugin* currentPlugin = nullptr;
Plugin* pluginList[] = {
    &tableEnv,
    &cvRec,
    &subHarmonic,
    &arp,
    &eucl,
    &randplug,
    &xfade
};

class MetaPlugin : public Plugin
{
public:
    const char* name() const override {
        return "META";
    }

    void init() override{
        for (unsigned i = 0; i < ARRAY_SIZE(pluginList); i++)
            pluginList[i]->init();
    }

    void AudioCallback(const float* const*, float**, size_t)  override {
    }

    void process()  override {
        int inc = patch.encoder.Increment();
        if ( patch.encoder.RisingEdge() ) {
            if (pluginList[idx]->load()) {
                currentPlugin->unload();
                std::exchange(currentPlugin, pluginList[idx]);
            }
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

private:
    u_int16_t idx = 0;
};

MetaPlugin meta;

void AudioCallback(const float* const* input, float** output, unsigned int size)
{
    currentPlugin->AudioCallback(input, output, size);
}

void pauseAudio() {
    patch.StopAudio();
}

void resumeAudio() {
    patch.StartAudio(AudioCallback);
}

int main(void)
{

    patch.Init(); // Initialize hardware (daisy seed, and patch)
    SdmmcHandler::Config sd_cfg;
    sd_cfg.Defaults();
    sdcard.Init(sd_cfg);
    dsy_fatfs_init();
    f_mount(&SDFatFS, SDPath, 1);

    meta.init();
    currentPlugin = &meta;

    patch.StartAdc();
    patch.StartAudio(AudioCallback);
    while(1)  {
        patch.ProcessAllControls();
        if (patch.encoder.TimeHeldMs() > 1000) {
            currentPlugin = &meta;
        }
        currentPlugin->process();
    }
}
