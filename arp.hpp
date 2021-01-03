/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
#include "plugin.hpp"

class Arp : public Plugin {
public:
    enum MODE {
        MODE_UP,
        MODE_DOWN,
        MODE_UPDOWN,
        MODE_RANDOM,
        MODE_MAX
    };

    const char* name() const override;

    void init() override;

    void AudioCallback(float**, float**, size_t)  override;

    void process()  override;
private:

    void InitChords();

    void ProcessOutput();

    void UpdateControls();

    void ProcessOled();

    Parameter pitchParam;
    float pitch = 0.0;

    Parameter stepsParam;
    uint8_t stepsCount;

    Parameter chordParam;
    uint8_t chord = 0;

    uint8_t step = 0;

    Parameter modeParam;
    int mode = 0;
    bool upDown_UpPhase = true;

    int chordList[10][3];
};
