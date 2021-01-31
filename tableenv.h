#ifndef TABLEENV_H
#define TABLEENV_H

#include "plugin.hpp"
#include "Menu.hpp"

#define TABLE_ENV_TABLE_SIZE 1024


struct FileHeader {
    char magic[4];
    int32_t count;
} __attribute__((packed));

struct Envelope {
    char name[8];
    float samples[TABLE_ENV_TABLE_SIZE];
} __attribute__((packed));

static_assert ( sizeof (FileHeader) == 8,"bad Header size");
static_assert ( sizeof (Envelope) == 8 + 1024*sizeof (float),"bad enveloppe size");

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

    ShowTable(const char* name, EnvelopeState* envstate);

    bool process() override;

    void drawSelection(int xpos, int width);

    void print(int line, bool on) override;

    void onClick() override;

    void setEnvState(EnvelopeState* envstate);
private:
    bool m_stateEdit = false;
    ShowTableState m_state = TS_BACK;
    const char* m_name = nullptr;
    EnvelopeState* m_envstate = nullptr;
};



struct EnvelopeState {
    EnvelopeState();

    void init(TableEnv* parent);

    void processTrigger(GateIn* );
    void update();

    void reset();


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

    ShowTable m_showTableParam;

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

    void AudioCallback(float**, float**, size_t)  override;

    void processOled();
    void processInput();
    void processOutput();

    void process() override;

public:
    bool loadBank(const char* bankName);

    size_t m_tableCount = 0;
    Envelope* m_tables = nullptr;
    const char** m_labels = nullptr;

    friend struct EnvelopeState;
};

#endif // TABLEENV_H
