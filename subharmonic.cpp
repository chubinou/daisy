#include "subharmonic.hpp"

SubHarmonic::SubOsc subOscA;
SubHarmonic::SubOsc subOscB;

static LabelEntry       labelA("--- SubOsc A ---");
static LabelEntry       labelB("--- SubOsc B ---");
static LabelEntry       labelBind("--- Bind ---");

static RangeParamEntry* bindableEntries[] = {
    &subOscA.m_subDivParam,
    &subOscA.m_volParam,
    &subOscB.m_subDivParam,
    &subOscB.m_volParam,
};

static BindAllParamEntry paramBind(bindableEntries, ARRAY_SIZE(bindableEntries));


static MenuEntry* mainMenuEntries[] = {
    &labelA,
    &subOscA.m_subDivParam,
    &subOscA.m_volParam,

    &labelB,
    &subOscB.m_subDivParam,
    &subOscB.m_volParam,

    &labelBind,
    &paramBind.p1,
    &paramBind.p2,
    &paramBind.p3,
    &paramBind.p4,
};
static SimpleMenu mainMenu(mainMenuEntries, ARRAY_SIZE(mainMenuEntries));

void SubHarmonic::init()
{
    subOscA.init(this);
    subOscB.init(this);
}

void SubHarmonic::AudioCallback(const float * const *in, float** out, unsigned int size)
{

    for (size_t i = 0; i < size; i++)
    {
        if (m_state && in[0][i] > 0.f) {
            m_state = false;
            subClock();
        } else if (!m_state && in[0][i] < 0.f) {
            m_state = true;
            subClock();
        }


        out[0][i] = (in[0][i]
            + subOscA.m_vol * (*subOscA.m_subptr)
            + subOscB.m_vol * (*subOscB.m_subptr))
            / 3.f;
    }
}

void SubHarmonic::processOled()
{
    patch.display.Fill(false);
    patch.display.SetCursor(0,0);
    patch.display.WriteString("Sub Harmonic", Font_7x10, true);

    printSimpleParam(1, "sub1", subOscA.m_subDiv);
    printSimpleParam(2, "vol1", (int)(subOscA.m_vol * 1000));
    printSimpleParam(3, "sub2", subOscB.m_subDiv);
    printSimpleParam(4, "vol2", (int)(subOscB.m_vol * 1000));

    patch.display.Update();
}

void SubHarmonic::processInput()
{
    subOscA.process();
    subOscB.process();
}

void SubHarmonic::process()
{
    processInput();
    processOled();
}

void SubHarmonic::subClock()
{
    m_count++;


    switch (m_count % 16) {
    case 0:
        m_sub[8]  = -1;
        m_sub[4] = -1;
        m_sub[2] = -1;
        break;
    case 2:
        m_sub[2] = 1;
        break;
    case 4:
        m_sub[4] = 1;
        m_sub[2] = -1;
        break;
    case 6:
        m_sub[2] = 1;
        break;
    case 8:
        m_sub[8]  = 1;
        m_sub[4] = -1;
        m_sub[2] = -1;
        break;
    case 10:
        m_sub[2] = 1;
        break;
    case 12:
        m_sub[4] = 1;
        m_sub[2] = -1;
        break;
    case 14:
        m_sub[2] = 1;
        break;
    default:
        break;
    }

    switch (m_count % 12)
    {
    case 0:
        m_sub[3] = -1;
        m_sub[6] = -1;
        break;
    case 3:
        m_sub[3] = 1;
        break;
    case 6:
        m_sub[3] = -1;
        m_sub[6] = 1;
        break;
    case 9:
        m_sub[3] = 1;
        break;
    default:
        break;
    }


    if (m_count % 10 == 0)
        m_sub[5] = -1;
    else if (m_count % 10 == 5)
        m_sub[5] = 1;

    if (m_count % 14 == 0)
        m_sub[7] = -1;
    else if (m_count % 14 == 7)
        m_sub[7] = 1;

}

SubHarmonic::SubOsc::SubOsc()
    : m_subDivParam("SubDiv", 2.f, 8.f, 6, 2, Parameter::LINEAR)
    , m_volParam("Volume", 0.f, 1.f, 128, 64, Parameter::LINEAR)
{
}

void SubHarmonic::SubOsc::init(SubHarmonic* parent)
{
    m_parent = parent;
    m_subptr = &m_parent->m_sub[0];
}

void SubHarmonic::SubOsc::process()
{
    m_subDivParam.processParam();
    float subSelect1 = m_subDivParam.value();

    m_subDiv = roundf(subSelect1);
    m_subptr = &m_parent->m_sub[m_subDiv];

    m_volParam.processParam();
    m_vol = m_volParam.value();
}
