#pragma once

#include <utils/settings/settingspage.h>

namespace Fooyin {
class SettingsManager;
}

class ConversionManager;

class ConverterSettingsPageWidget : public Fooyin::SettingsPageWidget
{
    Q_OBJECT

public:
    explicit ConverterSettingsPageWidget(Fooyin::SettingsManager* settings, ConversionManager* manager);

    void load() override;
    void apply() override;
    void reset() override;

private:
    void setupUI();

    Fooyin::SettingsManager* m_settings;
    ConversionManager* m_manager;

    // UI elements
    class QSpinBox* m_windowWidthSpin;
    class QSpinBox* m_windowHeightSpin;
    class QComboBox* m_defaultCodecCombo;
};

class ConverterSettingsPage : public Fooyin::SettingsPage
{
    Q_OBJECT

public:
    explicit ConverterSettingsPage(Fooyin::SettingsManager* settings, ConversionManager* manager);
};
