#include "Menu.hpp"

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
    return label();
}

int32_t SetMenuEntry::value() const {
    if (m_maxValue == 0)
        return -1;
    return m_value;
}

const char* SetMenuEntry::label() const {
    if (!m_labels || m_maxValue == 0)
        return "N/A";
    return m_labels[m_value];
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

RangeParamEntry::RangeParamEntry(const char *name, float min, float max, int initial, int steps, Scale scale)
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
    m_intValue += inc;
    if (m_intValue < 0)
        m_intValue = 0;
    if (m_intValue > m_steps)
        m_intValue = m_steps;

    float val = (float)m_intValue / (float)m_steps;
    switch(m_scale)
    {
        case LINEAR:
            m_value = (val * (m_max - m_min)) + m_min;
        break;
        case EXPONENTIAL:
            m_value = ((val * val) * (m_max -m_min)) + m_min;
            break;
        case LOGARITHMIC:
            m_value = expf((val * (m_lmax - m_lmax)) + m_lmin);
            break;
        case CUBE:
            m_value = ((val * (val * val)) * (m_max - m_min)) + m_min;
            break;
        default: break;
    }
}
