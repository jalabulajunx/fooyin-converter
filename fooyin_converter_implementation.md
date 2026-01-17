# Fooyin Audio Converter - Implementation Guide
## Quick Start Guide with Working Code Examples

---

## Phase 1: Minimal Viable Product (MVP)

### Step 1: Create Basic Plugin Structure

```bash
# Directory structure
fooyin-converter/
├── CMakeLists.txt
├── metadata.json
├── converterplugin.h
├── converterplugin.cpp
├── converterwidget.h
├── converterwidget.cpp
├── ffmpegwrapper.h
└── ffmpegwrapper.cpp
```

### metadata.json
```json
{
    "Name": "Audio Converter",
    "Version": "1.0.0",
    "CompatVersion": "1.0.0",
    "Author": "Your Name",
    "Copyright": "© 2026",
    "License": "GPL-3.0",
    "Description": "Convert audio files between formats using FFmpeg",
    "Url": "https://github.com/yourusername/fooyin-converter",
    "Category": "Tools"
}
```

### converterplugin.h
```cpp
#pragma once

#include <core/plugins/coreplugin.h>
#include <core/plugins/plugin.h>
#include <gui/plugins/guiplugin.h>

class ConverterWidget;
class FFmpegWrapper;

class ConverterPlugin : public QObject,
                        public Fooyin::Plugin,
                        public Fooyin::GuiPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.fooyin.fooyin.plugin/1.0" FILE "metadata.json")
    Q_INTERFACES(Fooyin::Plugin Fooyin::GuiPlugin)

public:
    void initialise(const Fooyin::GuiPluginContext& context) override;

private:
    ConverterWidget* m_widget{nullptr};
    FFmpegWrapper* m_ffmpeg{nullptr};
};
```

### converterplugin.cpp
```cpp
#include "converterplugin.h"
#include "converterwidget.h"
#include "ffmpegwrapper.h"

#include <gui/widgetprovider.h>

void ConverterPlugin::initialise(const Fooyin::GuiPluginContext& context)
{
    // Initialize FFmpeg wrapper
    m_ffmpeg = new FFmpegWrapper(this);
    
    // Check if FFmpeg is available
    if (!m_ffmpeg->isAvailable()) {
        qWarning() << "FFmpeg not found! Converter plugin disabled.";
        return;
    }
    
    // Register widget with fooyin
    context.widgetProvider->registerWidget(
        "AudioConverter",
        [this]() { 
            return new ConverterWidget(m_ffmpeg); 
        },
        "Audio Converter"
    );
    
    qInfo() << "Audio Converter plugin initialized";
    qInfo() << "FFmpeg version:" << m_ffmpeg->version();
}
```

---

## Phase 2: FFmpeg Wrapper Implementation

### ffmpegwrapper.h
```cpp
#pragma once

#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>

struct ConversionOptions {
    QString format;        // "mp3", "flac", "opus"
    int bitrate{320};     // kbps
    int quality{-1};      // For VBR (-1 = use bitrate)
    int sampleRate{0};    // 0 = preserve original
    int channels{0};      // 0 = preserve original
};

class FFmpegWrapper : public QObject
{
    Q_OBJECT
    
public:
    explicit FFmpegWrapper(QObject* parent = nullptr);
    
    // Check FFmpeg availability
    bool isAvailable() const;
    QString version() const;
    QString path() const;
    
    // Conversion
    bool convert(
        const QString& inputPath,
        const QString& outputPath,
        const ConversionOptions& options
    );
    
    // Async conversion with progress
    void convertAsync(
        const QString& inputPath,
        const QString& outputPath,
        const ConversionOptions& options
    );
    
    void cancel();
    
signals:
    void progressChanged(int percent);
    void conversionFinished(bool success, const QString& error);
    
private:
    QString buildArguments(
        const QString& inputPath,
        const QString& outputPath,
        const ConversionOptions& options
    );
    
    void parseProgress(const QString& output);
    
    QString m_ffmpegPath;
    QProcess* m_process{nullptr};
    qint64 m_duration{0};
};
```

### ffmpegwrapper.cpp
```cpp
#include "ffmpegwrapper.h"
#include <QStandardPaths>
#include <QRegularExpression>
#include <QDebug>

FFmpegWrapper::FFmpegWrapper(QObject* parent)
    : QObject(parent)
{
    // Try to find ffmpeg in system PATH
    m_ffmpegPath = QStandardPaths::findExecutable("ffmpeg");
    
    if (m_ffmpegPath.isEmpty()) {
        qWarning() << "FFmpeg not found in PATH";
    }
}

bool FFmpegWrapper::isAvailable() const
{
    return !m_ffmpegPath.isEmpty();
}

QString FFmpegWrapper::version() const
{
    if (!isAvailable()) {
        return "Not found";
    }
    
    QProcess process;
    process.start(m_ffmpegPath, {"-version"});
    process.waitForFinished();
    
    QString output = process.readAllStandardOutput();
    QRegularExpression versionRegex("ffmpeg version ([^\\s]+)");
    QRegularExpressionMatch match = versionRegex.match(output);
    
    if (match.hasMatch()) {
        return match.captured(1);
    }
    
    return "Unknown";
}

QString FFmpegWrapper::path() const
{
    return m_ffmpegPath;
}

QString FFmpegWrapper::buildArguments(
    const QString& inputPath,
    const QString& outputPath,
    const ConversionOptions& options)
{
    QStringList args;
    
    // Input
    args << "-i" << inputPath;
    
    // Overwrite without asking
    args << "-y";
    
    // Audio codec based on format
    if (options.format == "mp3") {
        args << "-c:a" << "libmp3lame";
        if (options.quality >= 0) {
            args << "-q:a" << QString::number(options.quality);
        } else {
            args << "-b:a" << QString::number(options.bitrate) + "k";
        }
    }
    else if (options.format == "flac") {
        args << "-c:a" << "flac";
        args << "-compression_level" << "8";
    }
    else if (options.format == "opus") {
        args << "-c:a" << "libopus";
        args << "-b:a" << QString::number(options.bitrate) + "k";
    }
    else if (options.format == "aac" || options.format == "m4a") {
        args << "-c:a" << "aac";
        args << "-b:a" << QString::number(options.bitrate) + "k";
    }
    else if (options.format == "ogg") {
        args << "-c:a" << "libvorbis";
        if (options.quality >= 0) {
            args << "-q:a" << QString::number(options.quality);
        } else {
            args << "-b:a" << QString::number(options.bitrate) + "k";
        }
    }
    else if (options.format == "wav") {
        args << "-c:a" << "pcm_s16le";
    }
    
    // Sample rate
    if (options.sampleRate > 0) {
        args << "-ar" << QString::number(options.sampleRate);
    }
    
    // Channels
    if (options.channels > 0) {
        args << "-ac" << QString::number(options.channels);
    }
    
    // Preserve metadata
    args << "-map_metadata" << "0";
    args << "-id3v2_version" << "3";
    
    // Progress reporting
    args << "-progress" << "pipe:1";
    args << "-nostats";
    
    // Output
    args << outputPath;
    
    return args.join(" ");
}

bool FFmpegWrapper::convert(
    const QString& inputPath,
    const QString& outputPath,
    const ConversionOptions& options)
{
    if (!isAvailable()) {
        qWarning() << "FFmpeg not available";
        return false;
    }
    
    QStringList args = buildArguments(inputPath, outputPath, options).split(" ");
    
    QProcess process;
    process.start(m_ffmpegPath, args);
    
    if (!process.waitForStarted()) {
        qWarning() << "Failed to start FFmpeg";
        return false;
    }
    
    if (!process.waitForFinished(-1)) {
        qWarning() << "FFmpeg process error";
        return false;
    }
    
    if (process.exitCode() != 0) {
        QString error = process.readAllStandardError();
        qWarning() << "Conversion failed:" << error;
        return false;
    }
    
    return true;
}

void FFmpegWrapper::convertAsync(
    const QString& inputPath,
    const QString& outputPath,
    const ConversionOptions& options)
{
    if (m_process) {
        qWarning() << "Conversion already in progress";
        return;
    }
    
    m_process = new QProcess(this);
    m_duration = 0;
    
    // Connect progress monitoring
    connect(m_process, &QProcess::readyReadStandardOutput, this, [this]() {
        QString output = m_process->readAllStandardOutput();
        parseProgress(output);
    });
    
    // Connect completion
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus status) {
        bool success = (exitCode == 0 && status == QProcess::NormalExit);
        QString error;
        
        if (!success) {
            error = m_process->readAllStandardError();
        }
        
        m_process->deleteLater();
        m_process = nullptr;
        
        emit conversionFinished(success, error);
    });
    
    // Start conversion
    QStringList args = buildArguments(inputPath, outputPath, options).split(" ");
    m_process->start(m_ffmpegPath, args);
}

void FFmpegWrapper::cancel()
{
    if (m_process) {
        m_process->kill();
        m_process->deleteLater();
        m_process = nullptr;
    }
}

void FFmpegWrapper::parseProgress(const QString& output)
{
    // Parse FFmpeg progress output
    // Format: out_time_us=12345678
    
    static QRegularExpression durationRegex("Duration: (\\d{2}):(\\d{2}):(\\d{2})\\.(\\d{2})");
    static QRegularExpression timeRegex("out_time_us=(\\d+)");
    
    // Extract duration (only once)
    if (m_duration == 0) {
        QRegularExpressionMatch durationMatch = durationRegex.match(output);
        if (durationMatch.hasMatch()) {
            int hours = durationMatch.captured(1).toInt();
            int minutes = durationMatch.captured(2).toInt();
            int seconds = durationMatch.captured(3).toInt();
            int centiseconds = durationMatch.captured(4).toInt();
            
            m_duration = (hours * 3600 + minutes * 60 + seconds) * 1000000LL 
                       + centiseconds * 10000LL;
        }
    }
    
    // Parse current time
    QRegularExpressionMatch timeMatch = timeRegex.match(output);
    if (timeMatch.hasMatch() && m_duration > 0) {
        qint64 currentTime = timeMatch.captured(1).toLongLong();
        int percent = static_cast<int>((currentTime * 100) / m_duration);
        emit progressChanged(qBound(0, percent, 100));
    }
}
```

---

## Phase 3: Simple Widget Implementation

### converterwidget.h
```cpp
#pragma once

#include <QWidget>

class FFmpegWrapper;
class QLineEdit;
class QComboBox;
class QProgressBar;
class QPushButton;
class QLabel;

class ConverterWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit ConverterWidget(FFmpegWrapper* ffmpeg, QWidget* parent = nullptr);
    
private slots:
    void browseInput();
    void browseOutput();
    void startConversion();
    void onProgress(int percent);
    void onFinished(bool success, const QString& error);
    
private:
    void setupUI();
    QString getOutputExtension() const;
    
    FFmpegWrapper* m_ffmpeg;
    
    QLineEdit* m_inputEdit;
    QLineEdit* m_outputEdit;
    QComboBox* m_formatCombo;
    QComboBox* m_qualityCombo;
    QProgressBar* m_progressBar;
    QPushButton* m_convertButton;
    QLabel* m_statusLabel;
    
    bool m_converting{false};
};
```

### converterwidget.cpp
```cpp
#include "converterwidget.h"
#include "ffmpegwrapper.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QProgressBar>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>

ConverterWidget::ConverterWidget(FFmpegWrapper* ffmpeg, QWidget* parent)
    : QWidget(parent)
    , m_ffmpeg(ffmpeg)
{
    setupUI();
    
    // Connect FFmpeg signals
    connect(m_ffmpeg, &FFmpegWrapper::progressChanged,
            this, &ConverterWidget::onProgress);
    connect(m_ffmpeg, &FFmpegWrapper::conversionFinished,
            this, &ConverterWidget::onFinished);
}

void ConverterWidget::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    
    // Input section
    auto* inputLayout = new QHBoxLayout();
    m_inputEdit = new QLineEdit();
    m_inputEdit->setPlaceholderText("Select input audio file...");
    auto* inputButton = new QPushButton("Browse...");
    inputLayout->addWidget(new QLabel("Input:"));
    inputLayout->addWidget(m_inputEdit);
    inputLayout->addWidget(inputButton);
    connect(inputButton, &QPushButton::clicked, this, &ConverterWidget::browseInput);
    
    // Output section
    auto* outputLayout = new QHBoxLayout();
    m_outputEdit = new QLineEdit();
    m_outputEdit->setPlaceholderText("Output file path...");
    auto* outputButton = new QPushButton("Browse...");
    outputLayout->addWidget(new QLabel("Output:"));
    outputLayout->addWidget(m_outputEdit);
    outputLayout->addWidget(outputButton);
    connect(outputButton, &QPushButton::clicked, this, &ConverterWidget::browseOutput);
    
    // Format selection
    auto* formatLayout = new QHBoxLayout();
    m_formatCombo = new QComboBox();
    m_formatCombo->addItem("MP3", "mp3");
    m_formatCombo->addItem("FLAC", "flac");
    m_formatCombo->addItem("Opus", "opus");
    m_formatCombo->addItem("AAC/M4A", "m4a");
    m_formatCombo->addItem("Ogg Vorbis", "ogg");
    m_formatCombo->addItem("WAV", "wav");
    formatLayout->addWidget(new QLabel("Format:"));
    formatLayout->addWidget(m_formatCombo);
    
    // Quality/Bitrate selection
    m_qualityCombo = new QComboBox();
    m_qualityCombo->addItem("320 kbps (High)", 320);
    m_qualityCombo->addItem("256 kbps", 256);
    m_qualityCombo->addItem("192 kbps (Good)", 192);
    m_qualityCombo->addItem("128 kbps (Medium)", 128);
    m_qualityCombo->addItem("96 kbps (Low)", 96);
    formatLayout->addWidget(new QLabel("Quality:"));
    formatLayout->addWidget(m_qualityCombo);
    
    // Progress
    m_progressBar = new QProgressBar();
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    
    // Status
    m_statusLabel = new QLabel("Ready");
    
    // Convert button
    m_convertButton = new QPushButton("Convert");
    m_convertButton->setEnabled(false);
    connect(m_convertButton, &QPushButton::clicked, this, &ConverterWidget::startConversion);
    
    // Auto-update output path when input changes
    connect(m_inputEdit, &QLineEdit::textChanged, this, [this](const QString& path) {
        if (!path.isEmpty() && m_outputEdit->text().isEmpty()) {
            QFileInfo info(path);
            QString outputPath = info.absolutePath() + "/" 
                               + info.completeBaseName() 
                               + "." + getOutputExtension();
            m_outputEdit->setText(outputPath);
        }
        m_convertButton->setEnabled(!path.isEmpty() && !m_outputEdit->text().isEmpty());
    });
    
    // Update output extension when format changes
    connect(m_formatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this]() {
        QString input = m_inputEdit->text();
        if (!input.isEmpty()) {
            QFileInfo info(input);
            QString outputPath = info.absolutePath() + "/" 
                               + info.completeBaseName() 
                               + "." + getOutputExtension();
            m_outputEdit->setText(outputPath);
        }
    });
    
    // Add to layout
    mainLayout->addLayout(inputLayout);
    mainLayout->addLayout(outputLayout);
    mainLayout->addLayout(formatLayout);
    mainLayout->addWidget(m_progressBar);
    mainLayout->addWidget(m_statusLabel);
    mainLayout->addWidget(m_convertButton);
    mainLayout->addStretch();
}

void ConverterWidget::browseInput()
{
    QString filter = "Audio Files (*.mp3 *.flac *.wav *.ogg *.opus *.m4a *.aac);;All Files (*)";
    QString path = QFileDialog::getOpenFileName(this, "Select Audio File", QString(), filter);
    
    if (!path.isEmpty()) {
        m_inputEdit->setText(path);
    }
}

void ConverterWidget::browseOutput()
{
    QString ext = getOutputExtension();
    QString filter = QString("%1 Files (*.%2);;All Files (*)").arg(ext.toUpper(), ext);
    QString path = QFileDialog::getSaveFileName(this, "Save Output File", 
                                                 m_outputEdit->text(), filter);
    
    if (!path.isEmpty()) {
        m_outputEdit->setText(path);
    }
}

QString ConverterWidget::getOutputExtension() const
{
    return m_formatCombo->currentData().toString();
}

void ConverterWidget::startConversion()
{
    QString input = m_inputEdit->text();
    QString output = m_outputEdit->text();
    
    if (input.isEmpty() || output.isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "Please select input and output files.");
        return;
    }
    
    if (!QFile::exists(input)) {
        QMessageBox::warning(this, "File Not Found", "Input file does not exist.");
        return;
    }
    
    // Build conversion options
    ConversionOptions options;
    options.format = getOutputExtension();
    options.bitrate = m_qualityCombo->currentData().toInt();
    options.quality = -1; // Use bitrate
    options.sampleRate = 0; // Preserve
    options.channels = 0; // Preserve
    
    // Start conversion
    m_converting = true;
    m_convertButton->setEnabled(false);
    m_statusLabel->setText("Converting...");
    m_progressBar->setValue(0);
    
    m_ffmpeg->convertAsync(input, output, options);
}

void ConverterWidget::onProgress(int percent)
{
    m_progressBar->setValue(percent);
    m_statusLabel->setText(QString("Converting... %1%").arg(percent));
}

void ConverterWidget::onFinished(bool success, const QString& error)
{
    m_converting = false;
    m_convertButton->setEnabled(true);
    
    if (success) {
        m_statusLabel->setText("Conversion completed successfully!");
        m_progressBar->setValue(100);
        QMessageBox::information(this, "Success", "File converted successfully!");
    } else {
        m_statusLabel->setText("Conversion failed!");
        m_progressBar->setValue(0);
        QMessageBox::critical(this, "Error", "Conversion failed:\n" + error);
    }
}
```

---

## Phase 4: CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.14)

project(
    fooyin-converter
    VERSION 1.0.0
    DESCRIPTION "Audio Converter Plugin for Fooyin"
    HOMEPAGE_URL "https://github.com/yourusername/fooyin-converter"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# Find dependencies
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(Fooyin REQUIRED)

# Source files
set(SOURCES
    converterplugin.cpp
    converterplugin.h
    converterwidget.cpp
    converterwidget.h
    ffmpegwrapper.cpp
    ffmpegwrapper.h
)

# Create plugin
create_fooyin_plugin(
    fooyin-converter
    DEPENDS Fooyin::Core Fooyin::Gui Qt6::Widgets
    SOURCES ${SOURCES}
)

# Copy metadata
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/metadata.json"
    "${CMAKE_CURRENT_BINARY_DIR}/metadata.json"
    COPYONLY
)
```

---

## Phase 5: Build & Install

```bash
# Create build directory
mkdir build && cd build

# Configure
cmake .. -DCMAKE_PREFIX_PATH=/path/to/fooyin/install

# Build
cmake --build .

# Install
sudo cmake --install .

# Or install to user directory
cmake --install . --prefix ~/.local
```

---

## Testing the Plugin

### 1. Launch fooyin
```bash
fooyin
```

### 2. Enable the plugin
1. Go to Settings → Plugins
2. Find "Audio Converter"
3. Enable it
4. Restart fooyin

### 3. Add the widget
1. Enter Layout Editing Mode (View → Layout Editing)
2. Right-click → Add Widget → Audio Converter
3. Place it in your layout

### 4. Test conversion
1. Browse for an audio file
2. Select output format
3. Choose quality
4. Click Convert
5. Watch progress bar

---

## Next Steps for Full Implementation

After MVP is working:

1. **Add batch conversion** - Queue multiple files
2. **Integrate with fooyin tracks** - Right-click → Convert
3. **Preset system** - Save/load conversion presets
4. **Advanced options** - Sample rate, channels, custom FFmpeg args
5. **Error handling** - Better validation and user feedback
6. **Settings page** - Plugin configuration in fooyin settings
7. **Threading** - Parallel conversions
8. **Cue sheet support** - Split/convert cue-based albums

---

## Troubleshooting

### FFmpeg not found
```bash
# Install FFmpeg
sudo apt install ffmpeg  # Debian/Ubuntu
sudo dnf install ffmpeg  # Fedora
sudo pacman -S ffmpeg    # Arch

# Verify installation
which ffmpeg
ffmpeg -version
```

### Plugin not loading
- Check fooyin logs: `~/.config/fooyin/logs/`
- Verify metadata.json is correct
- Ensure all Qt/Fooyin dependencies are met
- Try rebuilding with verbose output: `cmake --build . --verbose`

### Conversion failures
- Check input file is valid
- Ensure output directory exists and is writable
- Verify FFmpeg supports the codec
- Check FFmpeg error output in widget

---

**Ready to start coding!** Begin with Phase 1 and test each component before moving to the next phase.
