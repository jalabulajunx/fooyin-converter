#include "convertersettingspage.h"
#include "convertersettings.h"
#include "conversionmanager.h"

#include <utils/settings/settingsmanager.h>

#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>

ConverterSettingsPageWidget::ConverterSettingsPageWidget(Fooyin::SettingsManager* settings, ConversionManager* manager)
    : m_settings{settings}
    , m_manager{manager}
    , m_windowWidthSpin{nullptr}
    , m_windowHeightSpin{nullptr}
    , m_defaultCodecCombo{nullptr}
{
    setupUI();
}

void ConverterSettingsPageWidget::setupUI()
{
    auto* layout = new QVBoxLayout(this);

    // Window size group
    auto* windowGroup = new QGroupBox(tr("Dialog Window Size"), this);
    auto* windowLayout = new QFormLayout(windowGroup);

    m_windowWidthSpin = new QSpinBox(this);
    m_windowWidthSpin->setMinimum(400);
    m_windowWidthSpin->setMaximum(1920);
    m_windowWidthSpin->setSingleStep(50);
    m_windowWidthSpin->setSuffix(" px");
    windowLayout->addRow(tr("Width:"), m_windowWidthSpin);

    m_windowHeightSpin = new QSpinBox(this);
    m_windowHeightSpin->setMinimum(300);
    m_windowHeightSpin->setMaximum(1080);
    m_windowHeightSpin->setSingleStep(50);
    m_windowHeightSpin->setSuffix(" px");
    windowLayout->addRow(tr("Height:"), m_windowHeightSpin);

    auto* sizeNote = new QLabel(tr("Note: Window size will apply the next time the converter dialog is opened."), this);
    sizeNote->setWordWrap(true);
    sizeNote->setStyleSheet("QLabel { color: gray; font-style: italic; }");
    windowLayout->addRow(sizeNote);

    layout->addWidget(windowGroup);

    // Default format group
    auto* formatGroup = new QGroupBox(tr("Default Format"), this);
    auto* formatLayout = new QFormLayout(formatGroup);

    m_defaultCodecCombo = new QComboBox(this);

    // Populate with available codecs
    QStringList codecs = m_manager->availableCodecs();
    if (codecs.isEmpty()) {
        m_defaultCodecCombo->addItem(tr("None Available"), "");
        m_defaultCodecCombo->setEnabled(false);
    } else {
        for (const auto& codec : codecs) {
            m_defaultCodecCombo->addItem(codec.toUpper(), codec);
        }
    }

    formatLayout->addRow(tr("Default codec:"), m_defaultCodecCombo);

    auto* codecNote = new QLabel(tr("This codec will be pre-selected when opening the converter dialog."), this);
    codecNote->setWordWrap(true);
    codecNote->setStyleSheet("QLabel { color: gray; font-style: italic; }");
    formatLayout->addRow(codecNote);

    layout->addWidget(formatGroup);

    layout->addStretch();
}

void ConverterSettingsPageWidget::load()
{
    // Load window size
    int width = m_settings->value<ConverterSettings::WindowWidth>();
    int height = m_settings->value<ConverterSettings::WindowHeight>();

    m_windowWidthSpin->setValue(width);
    m_windowHeightSpin->setValue(height);

    // Load default codec
    QString defaultCodec = m_settings->value<ConverterSettings::DefaultCodec>();

    int index = m_defaultCodecCombo->findData(defaultCodec);
    if (index >= 0) {
        m_defaultCodecCombo->setCurrentIndex(index);
    }
}

void ConverterSettingsPageWidget::apply()
{
    // Save window size
    m_settings->set<ConverterSettings::WindowWidth>(m_windowWidthSpin->value());
    m_settings->set<ConverterSettings::WindowHeight>(m_windowHeightSpin->value());

    // Save default codec
    QString codec = m_defaultCodecCombo->currentData().toString();
    m_settings->set<ConverterSettings::DefaultCodec>(codec);
}

void ConverterSettingsPageWidget::reset()
{
    // Reset to defaults
    m_settings->reset<ConverterSettings::WindowWidth>();
    m_settings->reset<ConverterSettings::WindowHeight>();
    m_settings->reset<ConverterSettings::DefaultCodec>();

    // Reload UI
    load();
}

ConverterSettingsPage::ConverterSettingsPage(Fooyin::SettingsManager* settings, ConversionManager* manager)
    : SettingsPage{settings->settingsDialog()}
{
    setId("AudioConverter.Settings");
    setName(tr("Audio Converter"));
    setCategory({"Plugins"});
    setWidgetCreator([settings, manager] {
        return new ConverterSettingsPageWidget(settings, manager);
    });
}
