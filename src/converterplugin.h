#pragma once

#include <core/plugins/coreplugin.h>
#include <core/plugins/plugin.h>
#include <gui/plugins/guiplugin.h>

class QAction;

namespace Fooyin {
class TrackSelectionController;
class SettingsManager;
}

class ConversionManager;
class ConverterWidget;

class ConverterPlugin : public QObject,
                        public Fooyin::Plugin,
                        public Fooyin::CorePlugin,
                        public Fooyin::GuiPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.fooyin.fooyin.plugin/1.0" FILE "../metadata.json")
    Q_INTERFACES(Fooyin::Plugin Fooyin::CorePlugin Fooyin::GuiPlugin)

public:
    void initialise(const Fooyin::CorePluginContext& context) override;
    void initialise(const Fooyin::GuiPluginContext& context) override;

private slots:
    void showConverterDialog();

private:
    ConversionManager* m_manager{nullptr};
    ConverterWidget* m_converterDialog{nullptr};
    Fooyin::TrackSelectionController* m_trackSelection{nullptr};
    Fooyin::SettingsManager* m_settings{nullptr};
    QAction* m_convertAction{nullptr};
};
