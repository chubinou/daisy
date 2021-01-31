#include "Menu.hpp"
#include "plugin.hpp"

void MenuEntry::setParentMenu(SimpleMenu *parent) {
    m_parentMenu = parent;
}

//////////////////////////////////// SimpleMenu

SimpleMenu::SimpleMenu(MenuEntry **entries, int32_t count)
    : m_entryList(entries)
    , m_entryCount(count)
{
    for(int32_t i = 0; i < count; i++)
        m_entryList[i]->setParentMenu(this);
}

bool SimpleMenu::process() {
    if (m_subMenu != nullptr) {
        bool ret = m_subMenu->process();
        if (!ret)
            return false;
    } else {
        if (patch.encoder.RisingEdge()) {
            m_entryList[m_idx]->onClick();
        }

        int32_t inc = patch.encoder.Increment();
        if (inc > 0) {
            if (m_idx < m_entryCount - 1)
                m_idx++;
        } else if (inc < 0) {
            if (m_idx > 0)
                m_idx--;
        }
    }

    patch.display.Fill(false);

    if (m_idx < m_showIdx)
        m_showIdx = m_idx;
    else if (m_idx >= m_showIdx + 6)
        m_showIdx = m_idx - 5;

    int line = 0;
    for (int32_t i = m_showIdx; i < std::min(m_showIdx + 6, m_entryCount); i++) {
        if (i == m_idx)
            patch.display.DrawRect(0, line*10, SSD1309_WIDTH - 1, line*10 + 9, true);
        m_entryList[i]->print( line, i != m_idx );
        line++;
    }
    return false;
}

void SimpleMenu::leaveSubMenu() {
    m_subMenu = nullptr;
}

void SimpleMenu::enterSubMenu(Menu *menu) {
    m_subMenu = menu;
}

//////////////////////////////////// KVMenuEntry


KVMenuEntry::KVMenuEntry(const char *name)
    : m_name(name)
{
    m_paramPos = strlen(m_name) + 1;
}

void KVMenuEntry::print(int line, bool on)
{
    patch.display.SetCursor(0, line * 10);
    patch.display.WriteString((char*)m_name, Font_7x10, on);
    patch.display.SetCursor(m_paramPos * 7, line * 10);
    patch.display.WriteString((char*)repr().c_str(), Font_7x10, on);
}

const std::string KVMenuEntry::label() const
{
    return m_name;
}

//////////////////////////////////// KVEditMenuEntry

KVEditMenuEntry::KVEditMenuEntry(const char *name)
    : KVMenuEntry(name)
{
}

bool KVEditMenuEntry::process()
{
    if (patch.encoder.RisingEdge()) {
        m_parentMenu->leaveSubMenu();
        return true;
    }

    int32_t inc = patch.encoder.Increment();
    increment(inc);
    return true;
}

void KVEditMenuEntry::onClick()
{
    m_parentMenu->enterSubMenu(this);
}


//////////////////////////////////// BackMenuEntry

void BackMenuEntry::print(int line, bool on) {
    patch.display.SetCursor(0, line * 10);
    patch.display.WriteString((char*)"..", Font_7x10, on);
}

void BackMenuEntry::onClick() {
    m_parentMenu->leaveSubMenu();
}

//////////////////////////////////// SubMenuEntry

SubMenuEntry::SubMenuEntry(const char *name, MenuEntry**entries, int32_t count)
    : SimpleMenu(entries, count)
    , m_name(name)
{}

void SubMenuEntry::print(int line, bool on) {
    patch.display.SetCursor(0, line * 10);
    patch.display.WriteString((char*)m_name, Font_7x10, on);
}

void SubMenuEntry::onClick() {
    m_parentMenu->enterSubMenu(this);
}

////////////////////////////////// BoolMenuEntry

BoolMenuEntry::BoolMenuEntry(const char *name, bool defaultValue)
    : KVMenuEntry(name)
    , m_value(defaultValue)
{
}

std::string BoolMenuEntry::repr() const
{
    return m_value ? "Y" : "N";
}

void BoolMenuEntry::onClick()
{
    m_value = !m_value;
}

////////////////////////////////// SetMenuEntry

SetMenuEntry::SetMenuEntry(const char *name, const char **labels, int32_t maxValue)
    : KVEditMenuEntry(name)
    , m_labels(labels)
    , m_maxValue(maxValue)
{
}

void SetMenuEntry::setSet(const char **labels, int32_t maxValue) {
    m_labels = labels;
    m_maxValue = maxValue;
    m_value = 0;
}

void SetMenuEntry::increment(int val)
{
    m_value = (m_value + val + m_maxValue) % m_maxValue;
}

std::string SetMenuEntry::repr() const
{
    if (!m_labels || m_maxValue == 0)
        return "N/A";
    return m_labels[m_value];
}

int32_t SetMenuEntry::value() const {
    if (m_maxValue == 0)
        return -1;
    return m_value;
}

ShowParamEntry::ShowParamEntry(const char *name)
    : KVMenuEntry(name)
{
}

std::string ShowParamEntry::repr() const
{
    if (!m_param) {
        return "N/A";
    } else {
        return std::to_string((int)(m_param->Value() * 1000.f));
    }
}

void ShowParamEntry::onClick()
{
    //N/A
}

void ShowParamEntry::setParam(Parameter *param)
{
    m_param = param;
}

////////////////////////////////// RangeParamEntry

RangeParamEntry::RangeParamEntry(const char *name, float min, float max, int initial, int steps, Parameter::Curve scale)
    : KVEditMenuEntry(name)
    , m_min(min)
    , m_max(max)
    , m_intValue(initial)
    , m_steps(steps)
    , m_scale(scale)
{
    m_lmin = logf(min < 0.0000001f ? 0.0000001f : min);
    m_lmax = logf(max);
    increment(0);
}

std::string RangeParamEntry::repr() const
{
    return std::to_string((int)(m_value * 1000.f));
}

void RangeParamEntry::increment(int inc)
{
    if (m_bind != -1)
        return;

    m_intValue += inc;
    if (m_intValue < 0)
        m_intValue = 0;
    if (m_intValue > m_steps)
        m_intValue = m_steps;

    float val = (float)m_intValue / (float)m_steps;
    switch(m_scale)
    {
        case Parameter::LINEAR:
            m_value = (val * (m_max - m_min)) + m_min;
        break;
        case Parameter::EXPONENTIAL:
            m_value = ((val * val) * (m_max -m_min)) + m_min;
            break;
        case Parameter::LOGARITHMIC:
            m_value = expf((val * (m_lmax - m_lmax)) + m_lmin);
            break;
        case Parameter::CUBE:
            m_value = ((val * (val * val)) * (m_max - m_min)) + m_min;
            break;
        default: break;
    }
}

void RangeParamEntry::bind(int b)
{
    m_bind = b;
    if (b != -1)
        m_param.Init(patch.controls[b], m_min, m_max, m_scale);
    else
        increment(0);
}

void RangeParamEntry::processParam()
{
    if (m_bind == -1)
        return;

    m_value = m_param.Process() * 0.05 + m_value * 0.95;
}


BindParamEntry::BindParamEntry(const char* name, int hwCtrl, RangeParamEntry** entries, BindParamEntry** bindedEntries, size_t count)
    : KVEditMenuEntry(name)
    , m_hwCtrl(hwCtrl)
    , m_entries(entries)
    , m_bindedEntries(bindedEntries)
    , m_count(count)
{
}

std::string BindParamEntry::repr() const
{
    if (m_index == -1) {
       return "NONE";
    } else {
        return m_entries[m_index]->label();
    }
}

void BindParamEntry::increment(int inc)
{
    if (inc == 0)
        return;

    int m_oldIndex = m_index;
    do {
        m_index += inc;

        if (m_index <= -1) {
            m_index = -1;
            break;
        }

        if (m_index >= (int)m_count) {
            m_index = m_oldIndex;
            break;
        }
    } while (m_index != -1 && m_bindedEntries[m_index] != nullptr);


    if (m_oldIndex == m_index)
        return;

    if (m_oldIndex != -1) {
        m_entries[m_oldIndex]->bind(-1);
        m_bindedEntries[m_oldIndex] = nullptr;
    }

    if (m_index != -1) {
        m_entries[m_index]->bind(m_hwCtrl);
        m_bindedEntries[m_index] = this;
    }
}

BindAllParamEntry::BindAllParamEntry(RangeParamEntry** entries, size_t count)
    : m_bindedEntries(new class BindParamEntry*[count] )
    , p1("P1", DaisyPatch::CTRL_1, entries, m_bindedEntries, count)
    , p2("P2", DaisyPatch::CTRL_2, entries, m_bindedEntries, count)
    , p3("P3", DaisyPatch::CTRL_3, entries, m_bindedEntries, count)
    , p4("P4", DaisyPatch::CTRL_4, entries, m_bindedEntries, count)
{
    for (size_t i =0; i < count; i++) {
        m_bindedEntries = nullptr;
    }
}

BindAllParamEntry::~BindAllParamEntry()
{
    delete[] m_bindedEntries;
}
