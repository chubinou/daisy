#ifndef TABLEENV_H
#define TABLEENV_H

#include "plugin.hpp"
#include "Menu.hpp"

#define TABLE_ENV_TABLE_SIZE 1024


struct FileHeader {
    char magic[4];
    uint8_t version;
    uint8_t reserved;
    int16_t count;
} __attribute__((packed));

struct Envelope {
    char name[12];
    float samples[TABLE_ENV_TABLE_SIZE];
} __attribute__((packed));

static_assert ( sizeof (FileHeader) == 8,"bad Header size");
static_assert ( sizeof (Envelope) == 12 + 1024*sizeof (float),"bad enveloppe size");

class TableEnv;
class ShowTable;
struct EnvelopeState;

enum  PlayMode: int {
    PM_FORWARD = 0,
    PM_LOOP,
    PM_REVERSE,
    PM_REVLOOP,
    PM_PONG,
    PM_MAX
};

class ShowTable : public ParentMenu {
public:
    ShowTable();

    enum {
        INTERNAL_PARAM_ENV_SELECT = 0,
        INTERNAL_PARAM_COUNT
    };

    void init(EnvelopeState **envstate, size_t count);
    void bindTable();

    bool process() override;

    void drawSelection(int xpos, int width);

    void setEnvState(EnvelopeState* envstate);
    MenuEntry* getCurrentMenu();
private:
    bool m_stateEdit = false;
    size_t m_currentState = 0;

    EnvelopeState** m_allEnv = nullptr;
    size_t m_allEnvCount = 0;

    EnvelopeState* m_envstate = nullptr;

    SetMenuEntry m_envelopeSelect;

    size_t m_state = 0;


    MenuEntry** m_params = nullptr;
    size_t m_paramsCount = 0;
};



struct EnvelopeState {
    EnvelopeState();

    void init(TableEnv* parent);

    void processTrigger(GateIn* );
    void update();

    void updatePhase();

    void reset();

    MenuEntry** bind(size_t& count);

    SetMenuEntry m_tableParam;

    RangeParamEntry m_timeParam;
    float m_duration = 0.f;

    SetMenuEntry m_loopParam;
    PlayMode m_playMode = PM_FORWARD;

    BoolMenuEntry m_invertParam;
    bool m_invert = false;

    RangeParamEntry m_tscaleParam;
    float m_timeShape = 1.f;

    RangeParamEntry m_vscaleParam;
    float m_valShape = 1.f;

    RangeParamEntry m_levelParam;
    float m_level = 5.f;

#define  ENV_STATE_COUNT_PARAM 7
    MenuEntry* m_allParam[ENV_STATE_COUNT_PARAM];

    Envelope* m_env = nullptr;
    float m_phase = 0.f;
    float m_delta = 0.f;
    float m_val = 0;
    bool m_running = false;
    bool m_fwd = true;

    TableEnv* m_parent = nullptr;
};

class TableEnv : public Plugin
{
public:
    const char* name() const override { return "Table Env"; }

    void init() override;
    bool load() override;
    void unload() override;

    void AudioCallback(const float* const*, float**, unsigned int)  override;

    void processOled();
    void processInput();
    void processOutput();

    void process() override;

public:
    bool loadBank(const char* bankName);

    Envelope* m_tables = nullptr;
    size_t m_tableCount = 0;
    const char** m_labels = nullptr;

    ShowTable m_showTable;

    friend struct EnvelopeState;
};

#endif // TABLEENV_H
