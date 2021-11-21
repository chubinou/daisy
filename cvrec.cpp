#include "cvrec.hpp"
#include "Menu.hpp"

static const unsigned short stepMapping[] {
    1,
    2,
    3,
    4,
    6,
    8,
    12,
    16,
    24,
    32
};




struct CVRecRecording {
public:
    uint16_t trackASample[TOTAL_STEPS];
    uint16_t trackBSample[TOTAL_STEPS];
    bool triggerSample[TOTAL_STEPS];
};

class CVRecStatusEntry : public KVMenuEntry {
public:
    CVRecStatusEntry()
        : KVMenuEntry("status")
    {}

    std::string repr() const override {
        switch (m_cvRec->m_state) {
         case CVRec::WaitClock:
            return "WAIT CLK";
         case CVRec::Recording:
            return "REC";
        default:
            return "PLAY";
        }
    }

    void onClick() override
    {
        //N/A
    }

    void setCVRec(const CVRec* cvRec) {
        m_cvRec = cvRec;
    }

private:
    const CVRec* m_cvRec = nullptr;
};

static CVRecStatusEntry statusEntry;
static ShowROStringEntry stepEntry{ "step" };
static BoolMenuEntry recEntry{ "record" };
static BoolMenuEntry quantEntry{ "quantize" };
static ShowROStringEntry numBeatsEntry{ "length" };
static ShowROStringEntry segmentEntry{ "segment" };

static MenuEntry* mainMenuEntries[] = {
    &statusEntry,
    &stepEntry,
    &recEntry,
    &quantEntry,
    &numBeatsEntry,
    &segmentEntry,
};
static SimpleMenu mainMenu(mainMenuEntries, ARRAY_SIZE(mainMenuEntries));

CVRec::CVRec()
{
}

void CVRec::init()
{
    m_inputAParam.Init(patch.controls[DaisyPatch::CTRL_1], 0, 0xFFF, Parameter::LINEAR);
    m_inputBParam.Init(patch.controls[DaisyPatch::CTRL_2], 0, 0xFFF, Parameter::LINEAR);
    m_numBeatsParam.Init(patch.controls[DaisyPatch::CTRL_3], 0, ARRAY_SIZE(stepMapping), Parameter::LINEAR);
    m_segmentParam.Init(patch.controls[DaisyPatch::CTRL_4], 0, 1, Parameter::LINEAR);
    statusEntry.setCVRec(this);
}

bool CVRec::load()
{
    m_rec = new  CVRecRecording();
    memset(m_rec, 0, sizeof(CVRecRecording));
    return true;
}

void CVRec::unload()
{
    delete m_rec;
}

void CVRec::processInput()
{
    uint32_t now = System::GetNow();
    bool trig = patch.gate_input[0].Trig();

    m_numBeatsParam.Process();
    m_segmentParam.Process();

    auto numBeatsIdx = (unsigned short)m_numBeatsParam.Value();
    if (numBeatsIdx < 0)
        numBeatsIdx = 0;
    else if (numBeatsIdx >= ARRAY_SIZE(stepMapping))
        numBeatsIdx = ARRAY_SIZE(stepMapping) - 1;
    auto newNumBeats = stepMapping[numBeatsIdx];
    if (newNumBeats != m_numBeats)
    {
        m_numBeats = newNumBeats;
        numBeatsEntry.setValue(std::to_string(m_numBeats));
    }

    uint16_t numSegment = (NUM_BEAT / m_numBeats) - 1;
    uint16_t segment = round( numSegment * m_segmentParam.Value());
    if (segment < 0)
        segment = 0;
    if (segment != m_segment) {
        m_segment = segment;
        segmentEntry.setValue(std::to_string(segment));
    }

    //update clock
    if (trig)
    {
        std::swap(m_clockCur, m_clockPrev);
        m_clockCur = now;
        if (m_state == WaitClock) {
            if (m_clockPrev != 0)
                m_state = Playing;
        }

        if (m_state != WaitClock)
        {
            m_clockCur = now;
            m_step = (m_step + 1) % m_numBeats;
            m_subStep = 0;
            m_stepLastTime = now;
            m_stepDuration = (now - m_clockPrev) / STEPS_PER_BEAT;
            m_stepUpdated = true;
            stepEntry.setValue(std::to_string(m_step));
        }
    }
    else if (m_state != WaitClock)
    {
        m_quantize = quantEntry.value();

        if (recEntry.value())
            m_state = Recording;
        else
            m_state = Playing;


        //update sub steps
        if (now - m_stepLastTime > m_stepDuration)
        {
            if (m_subStep < STEPS_PER_BEAT - 1)
            {
                m_subStep++;
                m_stepLastTime = now; // - ((now - m_stepLastTime) % m_stepDuration);
                m_stepUpdated = true;
            }
        }
    }

    uint16_t pos = (m_segment * m_numBeats + m_step)  * STEPS_PER_BEAT + m_subStep;
    if (m_state == Recording && m_stepUpdated)
    {
        m_inputAParam.Process();
        m_inputBParam.Process();
        m_rec->trackASample[pos] = m_inputAParam.Value();
        m_rec->trackBSample[pos] = m_inputBParam.Value();
        m_rec->triggerSample[pos] = patch.gate_input[1].State();
    }

}

void CVRec::processOled()
{
    mainMenu.process();
    patch.display.Update();
}

void CVRec::processOutput()
{
    if (m_stepUpdated) {
        uint16_t pos = (m_segment * m_numBeats + m_step) * STEPS_PER_BEAT + (m_quantize ? 0: m_subStep);
        patch.seed.dac.WriteValue(DacHandle::Channel::ONE, m_rec->trackASample[pos]);
        patch.seed.dac.WriteValue(DacHandle::Channel::TWO, m_rec->trackBSample[pos]);
        dsy_gpio_write(&patch.gate_output, m_rec->triggerSample[pos]);
        m_stepUpdated = false;
    }
}

void CVRec::process()
{
    processInput();
    processOled();
    processOutput();
}

void CVRec::AudioCallback(const float* const*, float**, unsigned int)
{
    //N/A
}
