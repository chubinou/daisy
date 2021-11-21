#ifndef SUBHARMONIC_HPP
#define SUBHARMONIC_HPP

#include "plugin.hpp"
#include "Menu.hpp"

class SubHarmonic : public Plugin
{
public:
    const char* name() const override { return "SubHarmonic"; }

    void init() override;

    void AudioCallback(const float* const* in, float** out, unsigned int size)  override;

    void processOled();
    void processInput();

    void process() override;

private:
    void subClock();

public:
    struct SubOsc{
        SubOsc();

        RangeParamEntry m_subDivParam;
        uint8_t m_subDiv = 0;

        RangeParamEntry m_volParam;
        float m_vol = 0.f;

        int8_t* m_subptr = nullptr;
        SubHarmonic* m_parent = nullptr;

        void init(SubHarmonic* parent);

        void process();

    };

    bool m_state =  false;

    size_t m_count = 0;

    int8_t m_sub[9];


};

#endif // SUBHARMONIC_HPP
