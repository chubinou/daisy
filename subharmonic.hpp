#ifndef SUBHARMONIC_HPP
#define SUBHARMONIC_HPP

#include "plugin.hpp"


class SubHarmonic : public Plugin
{
public:
    const char* name() const override { return "SubHarmonic"; }

    void init() override;

    void AudioCallback(float**in, float** out, size_t size)  override;

    void processOled();
    void processInput();

    void process() override;

    struct SubOsc{
        Parameter m_subDivParam;
        uint8_t m_subDiv = 0;
        Parameter m_volParam;
        float m_vol = 0.f;
        int8_t* m_subptr = nullptr;
        SubHarmonic* m_parent = nullptr;

        void init(SubHarmonic* parent, int ctrlDiv, int ctrlVol);

        void process();

    };

private:
    void subClock();

public:
    bool m_state =  false;

    SubOsc m_sub1;
    SubOsc m_sub2;

    size_t m_count = 0;

    int8_t m_sub[9];


};

#endif // SUBHARMONIC_HPP
