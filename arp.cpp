#include "daisysp.h"
#include "daisy_patch.h"
#include <string>
#include <algorithm>

using namespace daisy;
using namespace daisysp;

DaisyPatch patch;

class Plugin
{
public:
    virtual void init() = 0;
    virtual void AudioCallback(float**, float**, size_t) = 0;
    virtual void process() = 0;
    virtual const char* name() const = 0;

    virtual void setup() {}
    virtual void tearDown() {}

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

    const char* name() const override { return "ARP"; }

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

    void ProcessOled() {
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

class Euclidian : public Plugin
{
public:
    const char* name() const override { return "Euclidian"; }

    void init() override
    {
        stepsParam.Init(patch.controls[DaisyPatch::CTRL_1], 0, 32, Parameter::LINEAR);
        fillParam.Init(patch.controls[DaisyPatch::CTRL_2], 0, 1, Parameter::LINEAR);
        rotParam.Init(patch.controls[DaisyPatch::CTRL_3], 0, 1, Parameter::LINEAR);
        strPattern[16] = '\0';
    }

    void AudioCallback(float**, float**, size_t)  override
    {}

    void refill_pattern()
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

    void processOled() {
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

    void processInput()
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

    void processOutput()
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

    void process() override {
        processInput();
        processOled();
        processOutput();
    }

private:
    Parameter stepsParam;
    uint16_t stepsCount = -1;

    Parameter fillParam;
    float fill = -1;

    Parameter rotParam;
    float rot = -1;

    bool pattern[32];
    char strPattern[17];
    uint16_t step;
};


Arp arp;
Euclidian eucl;
Plugin* currentPlugin = nullptr;
Plugin* pluginList[] = {
    &arp,
    &eucl
};

#define ARRAY_SIZE(array) sizeof(array) / sizeof(*array)

class MetaPlugin : public Plugin
{
public:
    const char* name() const override { return "Euclidian"; }


    void init() override{
        for (unsigned i = 0; i < ARRAY_SIZE(pluginList); i++)
            pluginList[i]->init();
    }

    void AudioCallback(float**, float**, size_t)  override {
    }

    void process()  override {
        int inc = patch.encoder.Increment();
        if ( patch.encoder.RisingEdge() ) {
            currentPlugin = pluginList[idx];
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




