#include "tableenv.h"
#include "fatfs.h"


static EnvelopeState envA;
static EnvelopeState envB;

static EnvelopeState* allEnvelopes[] {
    &envA,
    &envB
};

static const char* playModeLabel[] = {
    "FWD",
    "LOOP",
    "REV",
    "RLOP",
    "PONG"
};

const char* envNames[] = {
    "Env A",
    "Env B",
};



ShowTable::ShowTable()
    : m_envelopeSelect("Env", envNames, 2)
{
    m_envelopeSelect.setParentMenu(this);
    bindTable();
}

void ShowTable::init(EnvelopeState** envstate, size_t count)
{
    m_allEnv = envstate;
    m_allEnvCount = count;

}

void ShowTable::bindTable()
{
    unsigned envId = m_envelopeSelect.value();
    if (m_envstate != allEnvelopes[envId])
    {
        m_envstate = allEnvelopes[envId];
        m_params = m_envstate->bind(m_paramsCount);
        for (unsigned i = 0; i < m_paramsCount; i++)
            m_params[i]->setParentMenu(this);
    }
}


MenuEntry* ShowTable::getCurrentMenu()
{
    size_t idx = m_state;
    if (idx == INTERNAL_PARAM_ENV_SELECT)
        return &m_envelopeSelect;
    return m_params[idx - INTERNAL_PARAM_COUNT];
}

bool ShowTable::process()
{
    patch.display.Fill(false);

    bindTable();

    if (!m_envstate || !m_envstate->m_env) {
        patch.display.SetCursor(0, 0);
        patch.display.WriteString("no table loaded", Font_7x10, false);
        return false;
    }

    MenuEntry* menu = getCurrentMenu();

    if (m_subMenu != nullptr) {
        bool ret = m_subMenu->process();
        if (!ret)
            return false;
    } else {
        if (patch.encoder.RisingEdge()) {
            menu->onClick();
        }

        int32_t inc = patch.encoder.Increment();
        m_state = (m_state + inc + m_paramsCount + INTERNAL_PARAM_COUNT) % (m_paramsCount + INTERNAL_PARAM_COUNT);
    }


    menu->print( 0, true );


    for (int i =0; i < patch.display.Width(); i++) {

        float shapedPhase = fclamp(powf(float(i) / patch.display.Width(), m_envstate->m_timeShape), 0.f, 1.f);

        int idx = (shapedPhase * TABLE_ENV_TABLE_SIZE);

        float val = fclamp(powf(m_envstate->m_env->samples[idx],
                                      m_envstate->m_valShape),
                           0.f, 1.f);

        //screen coordiante are upside down
        if (!m_envstate->m_invert)
            val = 1.f - val;

        patch.display.DrawPixel(i, 9 + val * (patch.display.Height() - 10), true);
    }

    patch.display.DrawLine(patch.display.Width() * m_envstate->m_phase, 10,
                           patch.display.Width() * m_envstate->m_phase, patch.display.Height() - 1, true);

    return false;
}

void ShowTable::drawSelection(int xpos, int width) {
    patch.display.DrawLine(xpos-1,         9, xpos+6*width,   9, true);
    patch.display.DrawLine(xpos-1,         0, xpos+6*width,   0, true);
    patch.display.DrawLine(xpos-1,         0, xpos,           9, true);
    patch.display.DrawLine(xpos-1+6*width, 0, xpos-1+6*width, 9, true);
}

void ShowTable::setEnvState(EnvelopeState* envstate) {
    m_envstate = envstate;
}



void TableEnv::init()
{
    unload();
    envA.init(this);
    envB.init(this);
}

bool TableEnv::load()
{
    if (!loadBank("bank1.envt")) {
        unload();
        return false;
    }
    return true;
}

void TableEnv::unload()
{
    envA.m_tableParam.setSet(nullptr, 0);
    envB.m_tableParam.setSet(nullptr, 0);
    envA.reset();
    envB.reset();
    if (m_tables)
        free(m_tables);
    m_tables = nullptr;
    if (m_labels)
        free(m_labels);
    m_labels = nullptr;
    m_tableCount = 0;
}

void TableEnv::AudioCallback(const float* const* input, float** output, unsigned int size)
{

    envA.updatePhase();
    envB.updatePhase();
    for (size_t i = 0; i < size; i++) {
        output[0][i] = envA.m_val * input[0][i];
        output[1][i] = envA.m_val * input[1][i];
        output[2][i] = envB.m_val * input[2][i];
        output[3][i] = envB.m_val * input[3][i];
    }
}

void EnvelopeState::update()
{
    m_timeParam.processParam();
    m_duration = m_timeParam.value();

    m_invert = m_invertParam.value();

    m_tscaleParam.processParam();
    m_timeShape = m_tscaleParam.value();

    m_vscaleParam.processParam();
    m_valShape  = m_vscaleParam.value();

    m_playMode  = (PlayMode)m_loopParam.value();
    if (m_playMode == PM_LOOP || m_playMode == PM_REVLOOP || m_playMode == PM_PONG) {
        m_running = true;
    }

    if (m_tableParam.value() != -1) {
        m_env = &m_parent->m_tables[m_tableParam.value()];
    } else {
        m_env = nullptr;
    }

    if (!m_running || !m_env)
        return;

    m_delta = 1  / (m_duration * patch.AudioCallbackRate());

    float shapedPhase = fclamp(powf(m_phase, m_timeShape), 0.f, 1.f);

    int16_t loValIdx = floorf(shapedPhase * TABLE_ENV_TABLE_SIZE);
    float phaseFrac = shapedPhase * TABLE_ENV_TABLE_SIZE - loValIdx;

    m_val = fclamp(pow(m_env->samples[loValIdx] * (1 - phaseFrac) +
                       m_env->samples[(loValIdx + 1)] *  phaseFrac,
                       m_valShape),
                   0.f, 1.f);

    if (m_invert) {
        m_val = 1.f - m_val;
    }
}

void EnvelopeState::updatePhase()
{
    if (m_fwd)
        m_phase += m_delta;
    else
        m_phase -= m_delta;

    if (m_phase > 1.f) {
        if (m_playMode ==  PM_FORWARD) {
            m_val = 0;
            m_phase = 0.f;
            m_running = false;
            return;
        } else if (m_playMode == PM_LOOP) {
            m_phase -= 1.f;
        } else if (m_playMode == PM_PONG) {
            m_phase = 2.f - m_phase;
            m_fwd = false;
        }
    } else if (m_phase < 0.f) {
        if (m_playMode == PM_REVERSE) {
            m_val = 0;
            m_phase = 0.f;
            m_running = false;
            return;
        } else if (m_playMode == PM_REVLOOP) {
            m_phase += 1.f;
        } else if (m_playMode == PM_PONG) {
            m_phase *= -1;
            m_fwd = true;
        }
    }
}


EnvelopeState::EnvelopeState()
    : m_tableParam("Table")
    , m_timeParam("Duration", 0.1f, 10.f, 128, 256, Parameter::EXPONENTIAL)
    , m_loopParam("Mode", playModeLabel, PM_MAX)
    , m_invertParam("Invert")
    , m_tscaleParam("TScale", 0.01f, 10.f, 64, 128, Parameter::EXPONENTIAL)
    , m_vscaleParam("VScale", 0.01f, 10.f, 64, 128, Parameter::EXPONENTIAL)
    , m_levelParam("Level",   0.1f,  5.f, 39, 39,  Parameter::LINEAR)
{
    int i = 0;
    m_allParam[i++] = &m_tableParam;
    m_allParam[i++] = &m_timeParam;
    m_allParam[i++] = &m_loopParam;
    m_allParam[i++] = &m_invertParam;
    m_allParam[i++] = &m_tscaleParam;
    m_allParam[i++] = &m_vscaleParam;
    m_allParam[i++] = &m_levelParam;
    assert(i == ENV_STATE_COUNT_PARAM);
}


void EnvelopeState::init(TableEnv *parent)
{
    m_parent = parent;
}


void EnvelopeState::reset()
{
    m_val = 0;
    m_env = nullptr;
    m_running = false;
    m_delta = 0;
}

MenuEntry** EnvelopeState::bind(size_t &count)
{
    count = ENV_STATE_COUNT_PARAM;
    return m_allParam;
}

void EnvelopeState::processTrigger(GateIn * gate)
{
    if (gate->Trig()) {
        switch(m_playMode) {
        case PM_FORWARD:
        case PM_LOOP:
        case PM_PONG:
            m_phase = 0.f;
            m_fwd = true;
            break;
        case PM_REVERSE:
        case PM_REVLOOP:
            m_phase = 1.f;
            m_fwd = false;
            break;
        default:
            //unreachable
            break;
        }
        m_running = true;
    }
}

void TableEnv::processInput()
{
    for (unsigned i = 0; i < ARRAY_SIZE(allEnvelopes); i++)
    {
        allEnvelopes[i]->processTrigger(&patch.gate_input[i]);
        allEnvelopes[i]->update();
    }
}


void TableEnv::processOled()
{
    m_showTable.process();
    patch.display.Update();
}


bool TableEnv::loadBank(const char* bankName) {
    unload();

    pauseAudio();
    f_mount(&SDFatFS, SDPath, 0);

    if(f_open(&SDFile, bankName, FA_OPEN_EXISTING | FA_READ) != FR_OK) {
        warn("can't open file");
        return false;
    }

    FileHeader header;
    UINT bitRead;
    if (f_read(&SDFile, &header, sizeof (FileHeader), &bitRead) != FR_OK) {
        warn("read header failed");
        return false;
    }

    if (strncmp(header.magic, "Envt", 4) != 0) {
        warn("bad file format");
        return false;
    }
    if (header.version != 1) {
        warn("bad file version");
        return false;
    }

    //header.count = __ntohl(header.count);
    m_tableCount = header.count;

    UINT readSize = m_tableCount * sizeof (Envelope);

    m_tables = (Envelope*)malloc(readSize);
    if (!m_tables) {
        warn("malloc failed");
        return false;
    }
    memset(m_tables, 0, readSize);

    m_labels = (const char**)malloc( m_tableCount * sizeof (char*));
    if (!m_labels) {
        warn("malloc failed");
        return false;
    }
    memset(m_labels, 0, m_tableCount * sizeof (char*));

    char* ptr = (char*)m_tables;
    while (readSize > 0) {
        FRESULT res = f_read(&SDFile, ptr, readSize, &bitRead);
        if (res != F_OK) {
            warn("FREAD FAIL: " + std::to_string(readSize));
            break;
        }
        ptr += bitRead;
        readSize -= bitRead;
    }
    f_close(&SDFile);

    resumeAudio();

    for (size_t i =0; i < m_tableCount; i++) {
        m_labels[i] = m_tables[i].name;
    }

    for (unsigned i = 0; i < ARRAY_SIZE(allEnvelopes); i++) {
        allEnvelopes[i]->m_tableParam.setSet(m_labels, m_tableCount);
    }
    m_showTable.init(allEnvelopes, ARRAY_SIZE(allEnvelopes));

    return true;
}

void TableEnv::processOutput()
{
    patch.seed.dac.WriteValue(DacHandle::Channel::ONE, (uint16_t)(envA.m_val * 0xFFF));
    patch.seed.dac.WriteValue(DacHandle::Channel::TWO, (uint16_t)(envB.m_val * 0xFFF));
}

void TableEnv::process() {
    processInput();
    processOled();
    processOutput();
}

