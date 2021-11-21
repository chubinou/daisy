#ifndef CVREC_HPP
#define CVREC_HPP

#include "plugin.hpp"

#define NUM_BEAT 32
#define STEPS_PER_BEAT 32
#define TOTAL_STEPS (STEPS_PER_BEAT * NUM_BEAT)

class CVRecStatusEntry;
struct CVRecRecording;
class CVRec : public Plugin
{
    enum State {
        WaitClock,
        Recording,
        Playing,
    };

public:
    CVRec();

    const char* name() const override { return "CV Rec"; }

    void init() override;

    bool load() override;
    void unload() override;

    void AudioCallback(const float* const*, float**, unsigned int)  override;

    void processInput();
    void processOled();
    void processOutput();

    void process() override;

private:
    uint32_t  m_stepDuration = 0;
    uint32_t  m_stepLastTime = 0;
    uint32_t m_clockCur = 0;
    uint32_t m_clockPrev = 0;

    State m_state = WaitClock;

    unsigned short m_step = 0;
    unsigned short m_subStep = 0;

    bool m_stepUpdated = false;
    CVRecRecording* m_rec;

    Parameter m_inputAParam;
    Parameter m_inputBParam;

    Parameter m_numBeatsParam;
    unsigned short m_numBeats = -1;

    Parameter m_segmentParam;
    unsigned short m_segment = 0;

    bool m_quantize = false;

    friend CVRecStatusEntry;
};

#endif // CVREC_HPP
