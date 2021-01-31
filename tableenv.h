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

enum  PlayMode: int {
    PM_FORWARD = 0,
    PM_LOOP,
    PM_REVERSE,
    PM_REVLOOP,
    PM_PONG,
    PM_MAX
};


struct EnvelopeState {
    void init(TableEnv* parent,
              SetMenuEntry* tableVar, SetMenuEntry* loopVar,
              RangeParamEntry* timeEntry,
              RangeParamEntry* tscale, RangeParamEntry* vscale,
              ShowTable* showTableEntry);

    void processTrigger(GateIn* );
    void update();

    void reset();

    RangeParamEntry* m_durationParam = nullptr;
    float m_duration = 0.f;

    RangeParamEntry* m_vscale = nullptr;
    float m_valShape = 1.f;

    RangeParamEntry* m_tscale = nullptr;
    float m_timeShape = 1.f;

    Envelope* m_env = nullptr;
    float m_phase = 0.f;
    float m_delta = 0.f;
    float m_val = 0;
    bool m_running = false;
    bool m_fwd = true;

    SetMenuEntry* m_loopVar = nullptr;
    PlayMode m_playMode = PM_FORWARD;

    SetMenuEntry* m_tableVar = nullptr;

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

    EnvelopeState m_envA;
    EnvelopeState m_envB;

    size_t m_tableCount = 0;
    Envelope* m_tables = nullptr;
    const char** m_labels = nullptr;

    friend struct EnvelopeState;
};

#endif // TABLEENV_H
