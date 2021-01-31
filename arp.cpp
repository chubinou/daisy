/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
#include "arp.hpp"

const char* Arp::name() const { return "ARP"; }

void Arp::init(){
    InitChords();

    pitchParam.Init(patch.controls[DaisyPatch::CTRL_1], 0, 10, pitchParam.LINEAR);
    stepsParam.Init(patch.controls[DaisyPatch::CTRL_2], 1, 13, pitchParam.LINEAR);
    chordParam.Init(patch.controls[DaisyPatch::CTRL_3], 0, 10, pitchParam.LINEAR);
    modeParam.Init(patch.controls[DaisyPatch::CTRL_4], 0, MODE_MAX, pitchParam.LINEAR);
}

void Arp::AudioCallback(float**, float**, size_t) {
}

void Arp::process() {
    UpdateControls();
    ProcessOutput();
    ProcessOled();
}

void Arp::InitChords() {
    // Maj, min, Aug, Dim
    // Maj7, min7, dom7, min/Maj7
    // dim7, half dim7

    //set thirds
    for (int i = 0; i < 8; i++)
    {
        //every other chord, maj third, min third
        chordList[i][0] = 3 + ((i + 1) % 2);
    }
    //min 3rds
    chordList[8][0] = chordList[9][0] = 3;

    //set fifths
    // perfect 5th
    chordList[0][1] = chordList[1][1] = chordList[4][1] = chordList[5][1] = chordList[6][1] = chordList[7][1] = 7;
    // diminished 5th
    chordList[3][1] = chordList[8][1] = chordList[9][1] = 6;
    // augmented 5th
    chordList[2][1] = 8;

    //set sevenths
    // triads (octave since triad has no 7th)
    chordList[0][2] = chordList[1][2] = chordList[2][2] = chordList[3][2] = 12;
    // major 7th
    chordList[4][2] = chordList[7][2] = 11;
    // minor 7th
    chordList[5][2] = chordList[6][2] = chordList[9][2] = 10;
    // diminished 7th
    chordList[8][2] = 9;
}

void Arp::ProcessOutput()
{
    int32_t chordStep = chordList[chord][step % 3] + (12 * (step / 3));
    int32_t outNote = (chordStep * 0xFFF) / (5 * 12.);
    dsy_dac_write(DSY_DAC_CHN1, outNote < 4095 ? outNote : 4095);
}

void Arp::UpdateControls()
{
    pitch = pitchParam.Process();
    stepsCount = stepsParam.Process();
    chord = chordParam.Process();
    mode = modeParam.Process();

    if (patch.gate_input[1].Trig()) {
        step = 0;
    }
    if (patch.gate_input[0].Trig()) {
        switch (mode)
        {
        case MODE_UP:
            step++;
            step %= stepsCount;
            break;
        case MODE_DOWN:
            if (step <= 0)
                step = stepsCount;
            step--;
            break;
        case MODE_UPDOWN:
            if (step >= (stepsCount - 1))
                upDown_UpPhase = false;
            if (step <= 0)
                upDown_UpPhase = true;
            if (upDown_UpPhase)
                step++;
            else
                step--;
            break;
        case MODE_RANDOM:
        default:
            step = rand() % stepsCount;
            break;

        }
        step %= stepsCount;
    }

    if (patch.gate_input[1].Trig()) {
        step = 0;
    }
}

void Arp::ProcessOled() {
    //OLED
    patch.display.Fill(false);

    patch.display.SetCursor(0,0);
    patch.display.WriteString("ARP", Font_7x10, true);

    std::string stepRepr = std::to_string(step) + " / " + std::to_string(stepsCount);
    printSimpleParam(1, "step", stepRepr.c_str());

    const char* chordName = "";
    switch (chord) {
    case 0: chordName = "Maj"; break;
    case 1: chordName = "min";break;
    case 2: chordName = "Aug";break;
    case 3: chordName = "Dim";break;
    case 4: chordName = "Maj7";break;
    case 5: chordName = "min7";break;
    case 6: chordName = "dom7";break;
    case 7: chordName = "min/Maj7";break;
    case 8: chordName = "dim7";break;
    case 9: chordName = "half dim7";break;
    default: chordName = "unkn";break;
    }
    printSimpleParam(2, "mode", chordName);


    printSimpleParam(3, "chord", chordList[chord][step % 3] + (12 * (step / 3)));

    const char* modName = "";
    switch (mode) {
    case MODE_UP: modName = "UP"; break;
    case MODE_DOWN: modName = "DOWN";break;
    case MODE_UPDOWN: modName = "UP/DOWN";break;
    case MODE_RANDOM: modName = "RAND";break;
    default: modName = "unkn";break;
    }
    printSimpleParam(4, "mode", modName);
    patch.display.Update();
}
