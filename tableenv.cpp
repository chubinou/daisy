#include "tableenv.h"
#include "fatfs.h"
#include <machine/endian.h>


static EnvelopeState envA;
static EnvelopeState envB;

static const char* playModeLabel[] = {
    "FWD",
    "LOOP",
    "REV",
    "RLOP",
    "PONG"
};


static LabelEntry       labelA("--- Env A ---");
static LabelEntry       labelB("--- Env B ---");
static LabelEntry       labelBind("--- Bind ---");


static RangeParamEntry* bindableEntries[] = {
    &envA.m_timeParam,
    &envA.m_tscaleParam,
    &envA.m_vscaleParam,
    &envB.m_timeParam,
    &envB.m_tscaleParam,
    &envB.m_vscaleParam,
};

static BindAllParamEntry paramBind(bindableEntries, ARRAY_SIZE(bindableEntries));

static MenuEntry* mainMenuEntries[] = {
    &labelA,
    &envA.m_tableParam,
    &envA.m_timeParam,
    &envA.m_loopParam,
    &envA.m_invertParam,
    &envA.m_tscaleParam,
    &envA.m_vscaleParam,
    &envA.m_showTableParam,

    &labelB,
    &envB.m_tableParam,
    &envB.m_timeParam,
    &envB.m_loopParam,
    &envB.m_invertParam,
    &envB.m_tscaleParam,
    &envB.m_vscaleParam,
    &envB.m_showTableParam,

    &labelBind,
    &paramBind.p1,
    &paramBind.p2,
    &paramBind.p3,
    &paramBind.p4,
};
static SimpleMenu mainMenu(mainMenuEntries, ARRAY_SIZE(mainMenuEntries));



ShowTable::ShowTable(const char* name, EnvelopeState* envstate)
    : m_name(name)
    , m_envstate(envstate)
{}

bool ShowTable::process() {

    int inc = patch.encoder.Increment();
    if (inc != 0) {
        bool udpateSelection = true;
        switch (m_state) {
        case TS_SHAPE:
            if (m_stateEdit) {
                m_envstate->m_tableParam.increment(inc);
                udpateSelection = false;
            }
            break;
        case TS_TSCALE:
            if (m_stateEdit) {
                m_envstate->m_tscaleParam.increment(inc);
                udpateSelection = false;
            }
            break;
        case TS_LOOP:
            if (m_stateEdit) {
                m_envstate->m_loopParam.increment(inc);
                udpateSelection = false;
            }
            break;
        case TS_VSCALE:
            if (m_stateEdit) {
                m_envstate->m_vscaleParam.increment(inc);
                udpateSelection = false;
            }
            break;
        default:
            break;
        }
        if (udpateSelection)
            m_state = (ShowTableState)((m_state + inc + TS_MAX) % TS_MAX);
    }

    if (patch.encoder.RisingEdge()) {
        switch (m_state) {
        case TS_BACK:
            m_parentMenu->leaveSubMenu();
            return true;
            break;
        case TS_SHAPE:
        case TS_TSCALE:
        case TS_VSCALE:
        case TS_LOOP:
            m_stateEdit = !m_stateEdit;
            return true;
            break;
        default:
            break;
        }
    }

    patch.display.Fill(false);
    if (!m_envstate || !m_envstate->m_env) {
        patch.display.SetCursor(0, 0);
        patch.display.WriteString("no table loaded", Font_7x10, false);
        return false;
    }
    for (int i =0; i < SSD1309_WIDTH; i++) {

        float shapedPhase = fclamp(powf(float(i) / SSD1309_WIDTH, m_envstate->m_timeShape), 0.f, 1.f);

        int idx = (shapedPhase * TABLE_ENV_TABLE_SIZE);

        float val = fclamp(powf(m_envstate->m_env->samples[idx],
                                      m_envstate->m_valShape),
                           0.f, 1.f);

        //screen coordiante are upside down
        if (!m_envstate->m_invert)
            val = 1.f - val;

        patch.display.DrawPixel(i, 9 + val * (SSD1309_HEIGHT - 10), true);
    }

    patch.display.DrawLine(SSD1309_WIDTH * m_envstate->m_phase, 10,
                           SSD1309_WIDTH * m_envstate->m_phase, SSD1309_HEIGHT - 1, true);

    int xpos = 1;
    patch.display.SetCursor(xpos,1);
    patch.display.WriteChar('<', Font_6x8, true);
    if (m_state == TS_BACK) drawSelection(xpos, 1);
    xpos += 6*1+3;

    patch.display.SetCursor(xpos ,1);
    patch.display.WriteString(m_envstate->m_env->name, Font_6x8, !(m_state == TS_SHAPE && m_stateEdit));
    if (m_state == TS_SHAPE) drawSelection(xpos, 7);
    xpos += 6*7+3;

    patch.display.SetCursor(xpos,1);
    patch.display.WriteString(m_envstate->m_loopParam.repr().c_str(), Font_6x8, true);
    if (m_state == TS_LOOP) drawSelection(xpos, 4);
    xpos += 6*4+3;

    patch.display.SetCursor(xpos,1);
    patch.display.WriteChar('T', Font_6x8, true);
    patch.display.SetCursor(xpos+6,1);
    patch.display.WriteString(m_envstate->m_tscaleParam.repr().c_str(), Font_6x8, !(m_state == TS_TSCALE && m_stateEdit));
    if (m_state == TS_TSCALE) drawSelection(xpos, 5);
    xpos += 6*5+3;

    patch.display.SetCursor(xpos,1);
    patch.display.WriteChar('V', Font_6x8, true);
    patch.display.SetCursor(xpos+6,1);
    patch.display.WriteString(m_envstate->m_vscaleParam.repr().c_str(), Font_6x8, !(m_state == TS_VSCALE && m_stateEdit));
    if (m_state == TS_VSCALE) drawSelection(xpos, 5);
    xpos += 6*5+3;

    return false;
}

void ShowTable::drawSelection(int xpos, int width) {
    patch.display.DrawLine(xpos-1,         9, xpos+6*width,   9, true);
    patch.display.DrawLine(xpos-1,         0, xpos+6*width,   0, true);
    patch.display.DrawLine(xpos-1,         0, xpos,           9, true);
    patch.display.DrawLine(xpos-1+6*width, 0, xpos-1+6*width, 9, true);
}

void ShowTable::print(int line, bool on) {
    patch.display.SetCursor(0, line * 10);
    patch.display.WriteString((char*)m_name, Font_7x10, on);
}

void ShowTable::onClick(){
    m_parentMenu->enterSubMenu(this);
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

void TableEnv::AudioCallback(float ** input, float ** output, size_t size)
{
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

    m_delta = m_duration / patch.AudioCallbackRate();

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


EnvelopeState::EnvelopeState()
    : m_tableParam("Table")
    , m_timeParam("Duration", 0.1f, 30.f, 128, 256, Parameter::EXPONENTIAL)
    , m_loopParam("Mode", playModeLabel, PM_MAX)
    , m_invertParam("Invert")
    , m_tscaleParam("TScale", 0.01f, 10.f, 64, 128, Parameter::EXPONENTIAL)
    , m_vscaleParam("VScale", 0.01f, 10.f, 64, 128, Parameter::EXPONENTIAL)
    , m_levelParam("Level",   0.1f,  5.f, 39, 39,  Parameter::LINEAR)
    , m_showTableParam("view...", this)
{}


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
    envA.processTrigger(&patch.gate_input[0]);
    envB.processTrigger(&patch.gate_input[1]);
    envA.update();
    envB.update();
}


void TableEnv::processOled()
{
    mainMenu.process();
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

    envA.m_tableParam.setSet(m_labels, m_tableCount);
    envB.m_tableParam.setSet(m_labels, m_tableCount);
    return true;

}

void TableEnv::processOutput()
{
    dsy_dac_write(DSY_DAC_CHN1, (uint16_t)(envA.m_val * 0xFFF));
    dsy_dac_write(DSY_DAC_CHN2, (uint16_t)(envB.m_val * 0xFFF));
}

void TableEnv::process() {
    processInput();
    processOled();
    processOutput();
}

