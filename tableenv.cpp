#include "tableenv.h"
#include "fatfs.h"
#include <machine/endian.h>

class ShowTable : public MenuEntry,  public Menu {
public:
    enum ShowTableState : int {
        TS_BACK = 0,
        TS_SHAPE,
        TS_LOOP,
        TS_TSCALE,
        TS_VSCALE,
        TS_MAX
    };

    ShowTable(const char* name)
        : m_name(name)
    {}

    bool process() override {

        int inc = patch.encoder.Increment();
        if (inc != 0) {
            bool udpateSelection = true;
            switch (m_state) {
            case TS_SHAPE:
                if (m_stateEdit) {
                    m_envstate->m_tableVar->increment(inc);
                    udpateSelection = false;
                }
                break;
            case TS_TSCALE:
                if (m_stateEdit) {
                    m_envstate->m_tscale->increment(inc);
                    udpateSelection = false;
                }
                break;
            case TS_VSCALE:
                if (m_stateEdit) {
                    m_envstate->m_vscale->increment(inc);
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
                m_stateEdit = !m_stateEdit;
                return true;
            break;
            case TS_LOOP:
                m_envstate->m_loopVar->onClick();
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
            float val = fclamp(1.f - powf(m_envstate->m_env->samples[idx],
                                          m_envstate->m_valShape),
                               0.f, 1.f);
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
        patch.display.WriteString(m_envstate->m_loopVar->value() ? "L" : "S", Font_6x8, true);
        if (m_state == TS_LOOP) drawSelection(xpos, 1);
        xpos += 6*1+3;

        patch.display.SetCursor(xpos,1);
        patch.display.WriteChar('T', Font_6x8, true);
        patch.display.SetCursor(xpos+6,1);
        patch.display.WriteString(m_envstate->m_tscale->repr().c_str(), Font_6x8, !(m_state == TS_TSCALE && m_stateEdit));
        if (m_state == TS_TSCALE) drawSelection(xpos, 5);
        xpos += 6*5+3;

        patch.display.SetCursor(xpos,1);
        patch.display.WriteChar('V', Font_6x8, true);
        patch.display.SetCursor(xpos+6,1);
        patch.display.WriteString(m_envstate->m_vscale->repr().c_str(), Font_6x8, !(m_state == TS_VSCALE && m_stateEdit));
        if (m_state == TS_VSCALE) drawSelection(xpos, 5);
        xpos += 6*5+3;

        return false;
    }

    void drawSelection(int xpos, int width) {
        patch.display.DrawLine(xpos-1,         9, xpos+6*width,   9, true);
        patch.display.DrawLine(xpos-1,         0, xpos+6*width,   0, true);
        patch.display.DrawLine(xpos-1,         0, xpos,           9, true);
        patch.display.DrawLine(xpos-1+6*width, 0, xpos-1+6*width, 9, true);
    }

    void print(int line, bool on) override {
        patch.display.SetCursor(0, line * 10);
        patch.display.WriteString((char*)m_name, Font_7x10, on);
    }

    void onClick() override{
        m_parentMenu->enterSubMenu(this);
    }

    void setEnvState(EnvelopeState* envstate) {
        m_envstate = envstate;
    }
private:
    bool m_stateEdit = false;
    ShowTableState m_state = TS_BACK;
    const char* m_name = nullptr;
    EnvelopeState* m_envstate = nullptr;
};


static SetMenuEntry     tableA("A Table:");
static ShowParamEntry    timeA("A Duration:");
static BoolMenuEntry     loopA("A Loop:", false);
static RangeParamEntry tscaleA("A TScale:", 0.01f, 10.f, 64, 128, RangeParamEntry::EXPONENTIAL);
static RangeParamEntry vscaleA("A VScale:", 0.01f, 10.f, 64, 128, RangeParamEntry::EXPONENTIAL);
static ShowTable         showA("A view...");

static SetMenuEntry     tableB("B Table:");
static ShowParamEntry    timeB("B Duration:");
static BoolMenuEntry     loopB("B Loop:", false);
static RangeParamEntry tscaleB("B TScale:", 0.01f, 10.f, 64, 128, RangeParamEntry::EXPONENTIAL);
static RangeParamEntry vscaleB("B VScale:", 0.01f, 10.f, 64, 128, RangeParamEntry::EXPONENTIAL);
static ShowTable         showB("B view...");

static SetMenuEntry        p2A("A P2 DEST:");
static SetMenuEntry        p2B("B P2 DEST:");

static MenuEntry* mainMenuEntries[] = {
    &tableA,
    &timeA,
    &loopA,
    &tscaleA,
    &vscaleA,
    &showA,

    &tableB,
    &timeB,
    &tscaleB,
    &vscaleB,
    &loopB,
    &showB,

    &p2B,
    &p2A,
};
static SimpleMenu mainMenu(mainMenuEntries, ARRAY_SIZE(mainMenuEntries));

static const char* paramDest[] = {
    "Time",
    "Val",
};

void TableEnv::init()
{
    unload();
    m_envA.init(this, DaisyPatch::CTRL_1, DaisyPatch::CTRL_2, &tableA, &loopA, &timeA, &tscaleA, &vscaleA, &p2A, &showA);
    m_envB.init(this, DaisyPatch::CTRL_3, DaisyPatch::CTRL_4, &tableB, &loopB, &timeB, &tscaleB, &vscaleB, &p2B, &showB);
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
    tableA.setSet(nullptr, 0);
    tableB.setSet(nullptr, 0);
    m_envA.reset();
    m_envB.reset();
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
        output[0][i] = m_envA.m_val * input[0][i];
        output[1][i] = m_envA.m_val * input[1][i];
        output[2][i] = m_envB.m_val * input[2][i];
        output[3][i] = m_envB.m_val * input[3][i];
    }
}

void EnvelopeState::update()
{
    m_durationParam.Process();
    m_duration = m_durationParam.Process() * 0.05 + m_duration * 0.95;

    //m_p2Param.Process();
    //if (m_p2Dest->value() == 0)
    //    m_timeShape = m_p2Param.Process() * 0.05 + m_timeShape * 0.95;
    //else if (m_p2Dest->value() == 1)
    //    m_valShape= m_p2Param.Process() * 0.05 + m_valShape * 0.95;
    m_timeShape = m_tscale->value();
    m_valShape = m_vscale->value();


    if (m_tableVar->value() != -1) {
        m_env = &m_parent->m_tables[m_tableVar->value()];
    } else {
        m_env = nullptr;
    }

    if ((!m_loopVar->value() && !m_running) || !m_env)
        return;

    m_delta = m_duration / patch.AudioCallbackRate();

    m_phase += m_delta;
    if (m_phase > 1.f) {
        if (!m_loopVar->value()) {
            m_val = 0;
            m_phase = 0.f;
            m_running = false;
            return;
        } else {
            m_phase -= 1.f;
        }
    }

    float shapedPhase = fclamp(powf(m_phase, m_timeShape), 0.f, 1.f);

    int16_t loValIdx = floorf(shapedPhase * TABLE_ENV_TABLE_SIZE);
    float phaseFrac = shapedPhase * TABLE_ENV_TABLE_SIZE - loValIdx;

    m_val = fclamp(pow(m_env->samples[loValIdx] * (1 - phaseFrac) +
                       m_env->samples[(loValIdx + 1)] *  phaseFrac,
                       m_valShape),
                   0.f, 1.f);
}



void EnvelopeState::init(TableEnv *parent,
                         int ctrlDuration,
                         int ctrlShape,
                         SetMenuEntry *tableVar,
                         BoolMenuEntry *loopVar,
                         ShowParamEntry* timeEntry,
                         RangeParamEntry *tscale, RangeParamEntry *vscale,
                         SetMenuEntry* p2Dest,
                         ShowTable* showTableEntry)
{
    m_parent = parent;
    m_tableVar = tableVar;
    m_loopVar = loopVar;
    m_tscale = tscale;
    m_vscale = vscale;

    m_p2Dest = p2Dest;

    m_durationParam.Init(patch.controls[ctrlDuration], 0.1f, 30.f, m_durationParam.EXPONENTIAL);
    m_p2Param.Init(patch.controls[ctrlShape], 0.01f, 10.f, m_durationParam.EXPONENTIAL);
    timeEntry->setParam(&m_durationParam);
    p2Dest->setSet(paramDest , ARRAY_SIZE(paramDest));
    showTableEntry->setEnvState(this);
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
        m_running = true;
        m_phase = 0.f;
    }
}

void TableEnv::processInput()
{
    m_envA.processTrigger(&patch.gate_input[0]);
    m_envB.processTrigger(&patch.gate_input[1]);
    m_envA.update();
    m_envB.update();
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
    //header.count = __ntohl(header.count);
    m_tableCount = header.count;

    UINT readSize = m_tableCount * sizeof (Envelope);
    if (m_tableCount != 3) {
        std::string count = "unexp count" + std::to_string(m_tableCount);
        fail(count);
    }

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

    tableA.setSet(m_labels, m_tableCount);
    tableB.setSet(m_labels, m_tableCount);
    return true;

}

void TableEnv::processOutput()
{
    dsy_dac_write(DSY_DAC_CHN1, (uint16_t)(m_envA.m_val * 0xFFF));
    dsy_dac_write(DSY_DAC_CHN2, (uint16_t)(m_envB.m_val * 0xFFF));
}

void TableEnv::process() {
    processInput();
    processOled();
    processOutput();
}
