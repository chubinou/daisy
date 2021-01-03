#include "daisysp.h"
#include "daisy_patch.h"
#include <string>

using namespace daisy;
using namespace daisysp;

DaisyPatch patch;

class Plugin
{
public:
    virtual void init() = 0;
    virtual void AudioCallback(float**, float**, size_t) = 0;
    virtual void process() = 0;

    void printSimpleParam(short line, const char* name, const char* value) {

        std::string str = name;
        str += ": ";
        str += value;
        char* cstr = &str[0];
        patch.display.SetCursor(0, line * 10);
        patch.display.WriteString(cstr, Font_7x10, true);
    }

    void printSimpleParam(short line, const char* name, int value) {
        printSimpleParam(line, name, std::to_string(value).c_str());
    }
};

class Arp : public Plugin {
public:
    enum MODE {
        MODE_UP,
        MODE_DOWN,
        MODE_UPDOWN,
        MODE_RANDOM,
        MODE_MAX
    };

    void init() override{
        InitChords();

        pitchParam.Init(patch.controls[DaisyPatch::CTRL_1], 0, 10, pitchParam.LINEAR);
        stepsParam.Init(patch.controls[DaisyPatch::CTRL_2], 1, 13, pitchParam.LINEAR);
        chordParam.Init(patch.controls[DaisyPatch::CTRL_3], 0, 10, pitchParam.LINEAR);
        modeParam.Init(patch.controls[DaisyPatch::CTRL_4], 0, MODE_MAX, pitchParam.LINEAR);
    }

    void AudioCallback(float**, float**, size_t)  override {
    }

    void process()  override {
        UpdateControls();
        ProcessOutput();
        ProcessOled();
    }
private:

    void InitChords() {
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

    void ProcessOutput()
    {
        int32_t chordStep = chordList[chord][step % 3] + (12 * (step / 3));
        int32_t outNote = (chordStep * 4095) / 120.;
        dsy_dac_write(DSY_DAC_CHN1, outNote < 4095 ? outNote : 4095);
    }

    void UpdateControls()
    {
        pitch = pitchParam.Process();
        steps_count = stepsParam.Process();
        chord = chordParam.Process();
        mode = modeParam.Process();

        if (patch.gate_input[0].Trig()) {
            switch (mode)
            {
            case MODE_UP:
                step++;
                step %= steps_count;
                break;
            case MODE_DOWN:
                if (step <= 0)
                    step = steps_count;
                step--;
                break;
            case MODE_UPDOWN:
                if (step >= (steps_count - 1))
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
                step = rand() % steps_count;
                break;

            }
            step %= steps_count;
        }

        if (patch.gate_input[1].Trig()) {
            step = 0;
        }
    }

    void ProcessOled() {
        //OLED
        patch.display.Fill(false);

        patch.display.SetCursor(0,0);
        patch.display.WriteString("ARP", Font_7x10, true);

        std::string stepRepr = std::to_string(step) + " / " + std::to_string(steps_count);
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
        printSimpleParam(2, "chord", chordName);


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

    Parameter pitchParam;
    float pitch = 0.0;

    Parameter stepsParam;
    uint8_t steps_count;

    Parameter chordParam;
    uint8_t chord = 0;

    uint8_t step = 0;

    Parameter modeParam;
    int mode = 0;
    bool upDown_UpPhase = true;

    int chordList[10][3];

};

Arp arp;


void AudioCallback(float** input, float** output, size_t size)
{
    arp.AudioCallback(input, output, size);
}

int main(void)
{
    patch.Init(); // Initialize hardware (daisy seed, and patch)

    arp.init();

    patch.StartAdc();
    patch.StartAudio(AudioCallback);
    while(1)  {
        patch.DebounceControls();
        arp.process();
    }
}




