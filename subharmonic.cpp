#include "subharmonic.hpp"

void SubHarmonic::init()
{
    m_sub1.init(this, DaisyPatch::CTRL_1, DaisyPatch::CTRL_2);
    m_sub2.init(this, DaisyPatch::CTRL_3, DaisyPatch::CTRL_4);
}

void SubHarmonic::AudioCallback(float** in, float** out, size_t size)
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


        out[0][i] = in[0][i]
            + m_sub1.m_vol * (*m_sub1.m_subptr)
            + m_sub2.m_vol * (*m_sub2.m_subptr);
    }
}

void SubHarmonic::processOled()
{
    patch.display.Fill(false);
    patch.display.SetCursor(0,0);
    patch.display.WriteString("Sub Harmonic", Font_7x10, true);

    printSimpleParam(1, "sub1", m_sub1.m_subDiv);
    printSimpleParam(2, "vol1", (int)(m_sub1.m_vol * 1000));
    printSimpleParam(3, "sub2", m_sub2.m_subDiv);
    printSimpleParam(4, "vol2", (int)(m_sub2.m_vol * 1000));

    patch.display.Update();
}

void SubHarmonic::processInput()
{
    m_sub1.process();
    m_sub2.process();
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

void SubHarmonic::SubOsc::init(SubHarmonic* parent, int ctrlDiv, int ctrlVol)
{
    m_parent = parent;
    m_subDivParam.Init(patch.controls[ctrlDiv], 2, 8, Parameter::LINEAR);
    m_volParam.Init(patch.controls[ctrlVol], 0, 1, Parameter::LINEAR);
    m_subptr = &m_parent->m_sub[0];
}

void SubHarmonic::SubOsc::process()
{
    float subSelect1 = m_subDivParam.Process();
    m_subDiv = roundf(subSelect1);
    m_subptr = &m_parent->m_sub[m_subDiv];
    m_vol = m_volParam.Process();
}
