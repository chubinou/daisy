#ifndef MENU_HPP
#define MENU_HPP

#include <string>
#include <cstring>
#include <algorithm>
#include "daisysp.h"
#include "daisy_patch.h"

using namespace daisy;
using namespace daisysp;

extern DaisyPatch patch;

class Menu {
public:
    virtual bool process() = 0;
};
class SimpleMenu;

class MenuEntry {
public:
    virtual void print(int line, bool on) = 0;
    virtual void onClick() = 0;
    void setParentMenu(SimpleMenu* parent);
protected:
    SimpleMenu* m_parentMenu = nullptr;
};

class SimpleMenu : public Menu {
public:
    SimpleMenu( MenuEntry** entries, int32_t count);

    bool process() override;

    void leaveSubMenu();

    void enterSubMenu(Menu* menu);

    int32_t m_idx = 0;
    int32_t m_showIdx = 0;

    Menu* m_subMenu = nullptr;

    MenuEntry** m_entryList = nullptr;
    int32_t m_entryCount = 0;
};


class KVMenuEntry : public MenuEntry {
public:
    KVMenuEntry(const char* name);

    void print(int line, bool on) override;
    virtual std::string repr() const = 0;

private:
    const char* m_name;
    int m_paramPos = 0;
};

class KVEditMenuEntry : public KVMenuEntry, public Menu {
public:
    KVEditMenuEntry(const char* name);

    bool process() override;
    void onClick() override;

    virtual void increment(int inc) = 0;

};

class BackMenuEntry : public MenuEntry {
public:
    void print(int line, bool on) override;

    void onClick() override;
};

class BoolMenuEntry : public KVMenuEntry {
public:
    BoolMenuEntry(const char* name, bool defaultValue = false);

    std::string repr() const override;

    void onClick() override;

    inline bool value() { return m_value; }

private:
    bool m_value;
};

class SubMenuEntry : public MenuEntry, public SimpleMenu {
public:
    SubMenuEntry(const char* name, MenuEntry **entries, int32_t count);

    void print(int line, bool on) override;

    void onClick() override;

private:
    const char* m_name;
};

class SetMenuEntry : public KVEditMenuEntry {
public:
    SetMenuEntry(const char* name, const char** labels = nullptr, int32_t maxValue = 0);

    void setSet(const char** labels, int32_t maxValue);

    void increment(int val) override;

    std::string repr() const override;

    int32_t value() const;
    const char* label() const;

private:
    const char* m_name = nullptr;
    const char** m_labels = nullptr;

    int32_t m_maxValue = 0;
    int32_t m_value = 0;

    int m_paramPos = 0;
};

class ShowParamEntry : public KVMenuEntry {
public:
    ShowParamEntry(const char* name);

    std::string repr() const override;

    void onClick() override;

    void setParam(Parameter* param);

private:
    Parameter* m_param = nullptr;
};

class RangeParamEntry : public KVEditMenuEntry {
public:
    enum Scale {
        LINEAR,
        LOGARITHMIC,
        EXPONENTIAL,
        CUBE
    };

    RangeParamEntry(const char* name, float min, float max, int initial, int steps, Scale scale);

    std::string repr() const override;
    void increment(int val) override;

    inline float value() const { return m_value; }

private:
    float m_min;
    float m_max;
    float m_lmin;
    float m_lmax;

    int m_intValue;
    float m_value;
    int m_steps = 0;
    Scale m_scale;
};


#endif // MENU_HPP
