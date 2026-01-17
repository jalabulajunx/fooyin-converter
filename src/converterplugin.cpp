#include "converterplugin.h"
#include "conversionmanager.h"
#include "converterwidget.h"
#include "convertersettings.h"
#include "convertersettingspage.h"

#include <gui/widgetprovider.h>
#include <gui/trackselectioncontroller.h>
#include <gui/guiconstants.h>
#include <utils/actions/actionmanager.h>
#include <utils/actions/actioncontainer.h>
#include <utils/settings/settingsmanager.h>
#include <utils/id.h>
#include <QDebug>
#include <QAction>
#include <QDialog>
#include <QVBoxLayout>

void ConverterPlugin::initialise(const Fooyin::CorePluginContext& context)
{
    // Store settings manager from core context
    m_settings = context.settingsManager;

    // Register settings with default values
    m_settings->createSetting<ConverterSettings::DefaultCodec>(QString("flac"), "AudioConverter/DefaultCodec");
    m_settings->createSetting<ConverterSettings::WindowWidth>(600, "AudioConverter/WindowWidth");
    m_settings->createSetting<ConverterSettings::WindowHeight>(500, "AudioConverter/WindowHeight");

    qInfo() << "Audio Converter plugin: Settings registered";
}

void ConverterPlugin::initialise(const Fooyin::GuiPluginContext& context)
{
    // Initialize conversion manager
    m_manager = new ConversionManager(this);

    // Store track selection controller
    m_trackSelection = context.trackSelection;

    // Register settings page
    new ConverterSettingsPage(m_settings, m_manager);

    // Check if any codecs are available
    QStringList available = m_manager->availableCodecs();
    if (available.isEmpty()) {
        qWarning() << "Audio Converter Plugin: No audio codecs found!";
        qWarning() << "Please install one or more of: flac, lame, opusenc, oggenc";
        // Still register the widget so users can see the error message
    } else {
        qInfo() << "Audio Converter Plugin initialized successfully";
        qInfo() << "Available formats:" << available;
    }

    // Register widget with fooyin
    context.widgetProvider->registerWidget(
        "AudioConverter",
        [this]() {
            return new ConverterWidget(m_manager);
        },
        "Audio Converter"
    );

    // Create and register context menu action
    m_convertAction = new QAction(tr("Convert Audio..."), this);
    m_convertAction->setStatusTip(tr("Convert selected tracks to another format"));
    connect(m_convertAction, &QAction::triggered, this, &ConverterPlugin::showConverterDialog);

    // Register action with Fooyin's action manager
    auto* command = context.actionManager->registerAction(
        m_convertAction,
        Fooyin::Id{"AudioConverter.Convert"},
        Fooyin::Context{Fooyin::Constants::Context::Global}
    );

    if (command) {
        // Add to track selection context menu
        auto* trackMenu = context.actionManager->actionContainer(Fooyin::Id{Fooyin::Constants::Menus::Context::TrackSelection});
        if (trackMenu) {
            trackMenu->addAction(command);
            qInfo() << "Audio Converter action added to track context menu";
        } else {
            qWarning() << "Track selection menu not found";
        }
    } else {
        qWarning() << "Failed to register Audio Converter action";
    }

    qInfo() << "Audio Converter plugin GUI initialized";
}

void ConverterPlugin::showConverterDialog()
{
    if (!m_trackSelection->hasTracks()) {
        qWarning() << "No tracks selected";
        return;
    }

    // Get selected tracks
    auto tracks = m_trackSelection->selectedTracks();
    if (tracks.empty()) {
        qWarning() << "No tracks selected";
        return;
    }

    qInfo() << "Converting" << tracks.size() << "track(s)";

    // Create or reuse converter dialog
    if (!m_converterDialog) {
        m_converterDialog = new ConverterWidget(m_manager, m_settings);
        // Set window flags to make it a dialog
        m_converterDialog->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
        m_converterDialog->setWindowModality(Qt::ApplicationModal);

        // Apply window size from settings
        int width = m_settings->value<ConverterSettings::WindowWidth>();
        int height = m_settings->value<ConverterSettings::WindowHeight>();
        m_converterDialog->resize(width, height);
    }

    // Check if single or multiple tracks
    if (tracks.size() == 1) {
        // Single track conversion
        QString filepath = tracks.front().filepath();
        qInfo() << "Single track:" << filepath;
        m_converterDialog->loadTrack(filepath);
    } else {
        // Batch conversion
        QStringList filepaths;
        for (const auto& track : tracks) {
            filepaths << track.filepath();
        }
        qInfo() << "Batch conversion:" << filepaths.size() << "tracks";
        m_converterDialog->loadTracks(filepaths);
    }

    // Show the dialog
    m_converterDialog->show();
    m_converterDialog->raise();
    m_converterDialog->activateWindow();
}
