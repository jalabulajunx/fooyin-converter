# Fooyin Audio Converter Architecture
## Design Document for Audio Conversion Integration

**Author:** Architecture based on foo_converter.dll analysis  
**Date:** January 16, 2026  
**Purpose:** Bridge audio conversion functionality to fooyin music player

---

## Executive Summary

This document outlines the architecture for creating an audio conversion system compatible with fooyin, inspired by foobar2000's foo_converter component. The solution combines external conversion engines (FFmpeg/GNAC) with a fooyin plugin interface.

---

## 1. Technology Assessment: FFmpeg vs GNAC

### FFmpeg
**Advantages:**
- Universal format support (500+ codecs)
- Command-line interface (scriptable)
- Advanced features (filters, encoding presets, streaming)
- Multi-threaded encoding built-in
- Highly optimized
- Active development

**Disadvantages:**
- CLI-only (no native GUI)
- Complex parameter syntax
- Requires wrapper layer

**Best For:** Batch conversion, automated workflows, maximum format support

### GNAC
**Advantages:**
- GStreamer-based (consistent with Linux audio stack)
- GUI available (user-friendly presets)
- Profile management system
- Simpler for basic tasks

**Disadvantages:**
- Limited to GStreamer codecs
- Less flexible than FFmpeg
- Slower for batch operations
- May have GUI dependencies

**Best For:** Interactive use, simple conversions, GStreamer ecosystem integration

### **RECOMMENDATION: FFmpeg**
For a fooyin integration, FFmpeg is superior due to:
1. Better scriptability
2. No GUI dependencies (headless operation)
3. More comprehensive format support
4. Better batch processing performance
5. Industry-standard tool with extensive documentation

---

## 2. System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     Fooyin Music Player                      │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  ┌──────────────────────────────────────────────────────┐  │
│  │         Fooyin Converter Plugin (C++)                 │  │
│  │  - User Interface Widget                              │  │
│  │  - Conversion Queue Manager                           │  │
│  │  - Preset/Profile System                              │  │
│  │  - Progress Tracking                                   │  │
│  └────────────────┬─────────────────────────────────────┘  │
│                   │                                          │
└───────────────────┼──────────────────────────────────────────┘
                    │
                    │ (IPC / Process Management)
                    │
┌───────────────────▼──────────────────────────────────────────┐
│              Conversion Engine Layer                          │
│  ┌───────────────────────────────────────────────────────┐  │
│  │      FFmpeg Wrapper (Python/C++ subprocess)            │  │
│  │  - Command Builder                                     │  │
│  │  - Process Pool Manager                                │  │
│  │  - Output Parser                                       │  │
│  └───────────────────────────────────────────────────────┘  │
└───────────────────┬──────────────────────────────────────────┘
                    │
                    │ (subprocess / exec)
                    │
┌───────────────────▼──────────────────────────────────────────┐
│                   FFmpeg Binary                               │
│  - Audio decoding                                             │
│  - Format conversion                                          │
│  - Encoding with codec parameters                            │
└───────────────────────────────────────────────────────────────┘
```

---

## 3. Component Design

### 3.1 Fooyin Plugin Structure

Based on fooyin's plugin API, create a GUI plugin:

```cpp
// converterPlugin.h
#pragma once

#include <core/plugins/coreplugin.h>
#include <core/plugins/plugin.h>
#include <gui/plugins/guiplugin.h>

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
    ConversionManager* m_conversionManager;
    ConverterWidget* m_widget;
};
```

### 3.2 Conversion Manager

The core logic handler:

```cpp
// ConversionManager.h (Pseudocode)
class ConversionManager {
public:
    struct ConversionJob {
        QString inputPath;
        QString outputPath;
        ConversionPreset preset;
        ConversionStatus status;
        int progress; // 0-100
        QString errorMessage;
    };
    
    struct ConversionPreset {
        QString name;
        QString outputFormat;      // "mp3", "flac", "opus", etc.
        QString codecOptions;      // encoder-specific
        int bitrate;               // for lossy
        int quality;               // VBR quality
        int sampleRate;            // 44100, 48000, etc.
        int channels;              // 1=mono, 2=stereo
        bool preserveMetadata;
        QString fileNamingPattern; // "%artist% - %title%"
    };
    
public:
    // Core Operations
    void addJobFromTrack(const Fooyin::Track& track, const ConversionPreset& preset);
    void addJobBatch(const QList<Fooyin::Track>& tracks, const ConversionPreset& preset);
    
    void startConversion();
    void pauseConversion();
    void cancelConversion();
    void clearQueue();
    
    // Preset Management
    void loadPresets();
    void savePreset(const ConversionPreset& preset);
    void deletePreset(const QString& name);
    QList<ConversionPreset> getPresets();
    
    // Status Queries
    QList<ConversionJob> getQueue();
    ConversionStats getStats();
    
signals:
    void jobStarted(int jobId);
    void jobProgress(int jobId, int progress);
    void jobCompleted(int jobId, bool success);
    void queueStatusChanged(int pending, int processing, int completed);
    
private:
    QQueue<ConversionJob> m_jobQueue;
    QThreadPool m_threadPool;
    FFmpegWrapper* m_ffmpeg;
    PresetDatabase* m_presetDb;
};
```

### 3.3 FFmpeg Wrapper

Abstraction layer for ffmpeg operations:

```cpp
// FFmpegWrapper.h (Pseudocode)
class FFmpegWrapper {
public:
    struct FFmpegCommand {
        QString inputFile;
        QString outputFile;
        QStringList arguments;
    };
    
    struct ConversionResult {
        bool success;
        QString outputPath;
        QString errorMessage;
        int exitCode;
    };
    
public:
    // Command Construction
    FFmpegCommand buildCommand(
        const QString& inputPath,
        const QString& outputPath,
        const ConversionManager::ConversionPreset& preset
    );
    
    // Execution
    ConversionResult execute(
        const FFmpegCommand& command,
        std::function<void(int)> progressCallback
    );
    
    // Utility
    bool isFFmpegAvailable();
    QString getFFmpegVersion();
    QStringList getSupportedFormats();
    QStringList getSupportedCodecs(const QString& format);
    
private:
    QString parseProgress(const QString& ffmpegOutput);
    void handleProcessOutput(QProcess* process);
    
    QString m_ffmpegPath;
    QProcess* m_currentProcess;
};
```

---

## 4. Detailed Pseudo-Code Implementation

### 4.1 Command Builder

```cpp
FFmpegWrapper::FFmpegCommand FFmpegWrapper::buildCommand(
    const QString& inputPath,
    const QString& outputPath,
    const ConversionPreset& preset)
{
    FFmpegCommand cmd;
    cmd.inputFile = inputPath;
    cmd.outputFile = outputPath;
    
    // Base arguments
    cmd.arguments << "-i" << inputPath;
    cmd.arguments << "-y"; // Overwrite output
    
    // Codec selection based on format
    if (preset.outputFormat == "mp3") {
        cmd.arguments << "-c:a" << "libmp3lame";
        if (preset.bitrate > 0) {
            cmd.arguments << "-b:a" << QString::number(preset.bitrate) + "k";
        } else {
            cmd.arguments << "-q:a" << QString::number(preset.quality);
        }
    }
    else if (preset.outputFormat == "flac") {
        cmd.arguments << "-c:a" << "flac";
        cmd.arguments << "-compression_level" << "8";
    }
    else if (preset.outputFormat == "opus") {
        cmd.arguments << "-c:a" << "libopus";
        cmd.arguments << "-b:a" << QString::number(preset.bitrate) + "k";
    }
    else if (preset.outputFormat == "aac") {
        cmd.arguments << "-c:a" << "aac";
        cmd.arguments << "-b:a" << QString::number(preset.bitrate) + "k";
    }
    else if (preset.outputFormat == "ogg") {
        cmd.arguments << "-c:a" << "libvorbis";
        cmd.arguments << "-q:a" << QString::number(preset.quality);
    }
    
    // Sample rate
    if (preset.sampleRate > 0) {
        cmd.arguments << "-ar" << QString::number(preset.sampleRate);
    }
    
    // Channels
    if (preset.channels > 0) {
        cmd.arguments << "-ac" << QString::number(preset.channels);
    }
    
    // Metadata handling
    if (preset.preserveMetadata) {
        cmd.arguments << "-map_metadata" << "0";
        cmd.arguments << "-id3v2_version" << "3";
    } else {
        cmd.arguments << "-map_metadata" << "-1";
    }
    
    // Progress reporting
    cmd.arguments << "-progress" << "pipe:1";
    
    // Output file
    cmd.arguments << outputPath;
    
    return cmd;
}
```

### 4.2 Conversion Execution

```cpp
ConversionResult FFmpegWrapper::execute(
    const FFmpegCommand& command,
    std::function<void(int)> progressCallback)
{
    ConversionResult result;
    result.success = false;
    
    // Create process
    QProcess process;
    process.setProgram(m_ffmpegPath);
    process.setArguments(command.arguments);
    
    // Connect progress monitoring
    connect(&process, &QProcess::readyReadStandardOutput, [&]() {
        QString output = process.readAllStandardOutput();
        int progress = parseProgress(output);
        if (progress >= 0 && progressCallback) {
            progressCallback(progress);
        }
    });
    
    // Start conversion
    process.start();
    
    if (!process.waitForStarted(5000)) {
        result.errorMessage = "Failed to start FFmpeg process";
        return result;
    }
    
    // Wait for completion
    if (!process.waitForFinished(-1)) {
        result.errorMessage = "Conversion timeout or error";
        return result;
    }
    
    result.exitCode = process.exitCode();
    
    if (result.exitCode == 0) {
        result.success = true;
        result.outputPath = command.outputFile;
    } else {
        result.errorMessage = process.readAllStandardError();
    }
    
    return result;
}
```

### 4.3 Progress Parsing

```cpp
QString FFmpegWrapper::parseProgress(const QString& ffmpegOutput)
{
    // FFmpeg progress format:
    // out_time_us=12345678
    // progress=continue/end
    
    static QRegularExpression timeRegex("out_time_us=(\\d+)");
    static qint64 totalDuration = 0;
    
    QRegularExpressionMatch match = timeRegex.match(ffmpegOutput);
    if (match.hasMatch()) {
        qint64 currentTime = match.captured(1).toLongLong();
        
        // Calculate percentage (requires duration from input)
        // This is simplified - real implementation needs duration extraction
        if (totalDuration > 0) {
            int percentage = (currentTime * 100) / totalDuration;
            return QString::number(qBound(0, percentage, 100));
        }
    }
    
    if (ffmpegOutput.contains("progress=end")) {
        return "100";
    }
    
    return "-1"; // Unknown progress
}
```

### 4.4 Widget Integration

```cpp
// ConverterWidget.cpp (Pseudocode)
void ConverterWidget::setupUI()
{
    // File Selection
    m_fileListWidget = new QListWidget();
    m_addFilesButton = new QPushButton("Add Files");
    m_addFolderButton = new QPushButton("Add Folder");
    
    // Preset Selection
    m_presetComboBox = new QComboBox();
    m_editPresetButton = new QPushButton("Edit Presets...");
    
    // Output Configuration
    m_outputDirEdit = new QLineEdit();
    m_browseButton = new QPushButton("Browse...");
    m_namingPatternEdit = new QLineEdit("%artist% - %title%");
    
    // Conversion Controls
    m_convertButton = new QPushButton("Convert");
    m_pauseButton = new QPushButton("Pause");
    m_stopButton = new QPushButton("Stop");
    
    // Progress Display
    m_progressBar = new QProgressBar();
    m_statusLabel = new QLabel();
    m_queueTable = new QTableWidget();
    
    // Connections
    connect(m_convertButton, &QPushButton::clicked, 
            this, &ConverterWidget::startConversion);
    connect(m_addFilesButton, &QPushButton::clicked,
            this, &ConverterWidget::addFilesToQueue);
    
    // Layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(createSourcePanel());
    mainLayout->addWidget(createPresetPanel());
    mainLayout->addWidget(createOutputPanel());
    mainLayout->addWidget(createProgressPanel());
    mainLayout->addWidget(createControlPanel());
}

void ConverterWidget::startConversion()
{
    ConversionPreset preset = m_presetComboBox->currentData()
                                   .value<ConversionPreset>();
    
    // Get selected tracks from fooyin
    QList<Fooyin::Track> tracks = getSelectedTracks();
    
    for (const auto& track : tracks) {
        QString outputPath = generateOutputPath(track, preset);
        m_conversionManager->addJobFromTrack(track, preset, outputPath);
    }
    
    m_conversionManager->startConversion();
}

QString ConverterWidget::generateOutputPath(
    const Fooyin::Track& track,
    const ConversionPreset& preset)
{
    QString pattern = m_namingPatternEdit->text();
    QString outputDir = m_outputDirEdit->text();
    
    // Substitute FooScript variables
    // %artist%, %album%, %title%, %track%, etc.
    QString filename = pattern;
    filename.replace("%artist%", track.artist());
    filename.replace("%album%", track.album());
    filename.replace("%title%", track.title());
    filename.replace("%track%", QString::number(track.trackNumber()));
    
    // Sanitize filename
    filename = sanitizeFilename(filename);
    
    // Add extension
    filename += "." + preset.outputFormat;
    
    return QDir(outputDir).filePath(filename);
}
```

### 4.5 Preset Management

```cpp
// PresetDatabase.cpp (Pseudocode)
class PresetDatabase {
public:
    void loadPresets() {
        // Load from JSON or SQLite
        QFile file(presetsPath());
        if (!file.open(QIODevice::ReadOnly)) {
            createDefaultPresets();
            return;
        }
        
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        QJsonArray presetsArray = doc.array();
        
        m_presets.clear();
        for (const QJsonValue& value : presetsArray) {
            ConversionPreset preset = presetFromJson(value.toObject());
            m_presets.append(preset);
        }
    }
    
    void savePresets() {
        QJsonArray presetsArray;
        for (const auto& preset : m_presets) {
            presetsArray.append(presetToJson(preset));
        }
        
        QJsonDocument doc(presetsArray);
        QFile file(presetsPath());
        if (file.open(QIODevice::WriteOnly)) {
            file.write(doc.toJson(QJsonDocument::Indented));
        }
    }
    
private:
    void createDefaultPresets() {
        // MP3 320kbps CBR
        m_presets.append(ConversionPreset{
            "MP3 320kbps (High Quality)",
            "mp3",
            "-b:a 320k",
            320,
            0,
            44100,
            2,
            true,
            "%artist% - %title%"
        });
        
        // MP3 V0 VBR
        m_presets.append(ConversionPreset{
            "MP3 V0 (Variable Bitrate)",
            "mp3",
            "-q:a 0",
            0,
            0,
            44100,
            2,
            true,
            "%artist% - %title%"
        });
        
        // FLAC Lossless
        m_presets.append(ConversionPreset{
            "FLAC (Lossless)",
            "flac",
            "-compression_level 8",
            0,
            8,
            0, // preserve original
            0, // preserve original
            true,
            "%artist% - %title%"
        });
        
        // Opus 128kbps
        m_presets.append(ConversionPreset{
            "Opus 128kbps",
            "opus",
            "-b:a 128k",
            128,
            0,
            48000,
            2,
            true,
            "%artist% - %title%"
        });
        
        savePresets();
    }
    
    QList<ConversionPreset> m_presets;
};
```

---

## 5. Threading and Performance

### 5.1 Conversion Queue Worker

```cpp
class ConversionWorker : public QRunnable {
public:
    ConversionWorker(
        ConversionJob job,
        FFmpegWrapper* ffmpeg,
        ConversionManager* manager)
        : m_job(job)
        , m_ffmpeg(ffmpeg)
        , m_manager(manager)
    {}
    
    void run() override {
        emit m_manager->jobStarted(m_job.id);
        
        FFmpegCommand cmd = m_ffmpeg->buildCommand(
            m_job.inputPath,
            m_job.outputPath,
            m_job.preset
        );
        
        auto progressCallback = [this](int progress) {
            emit m_manager->jobProgress(m_job.id, progress);
        };
        
        ConversionResult result = m_ffmpeg->execute(cmd, progressCallback);
        
        emit m_manager->jobCompleted(m_job.id, result.success);
        
        if (!result.success) {
            qWarning() << "Conversion failed:" << result.errorMessage;
        }
    }
    
private:
    ConversionJob m_job;
    FFmpegWrapper* m_ffmpeg;
    ConversionManager* m_manager;
};

// In ConversionManager
void ConversionManager::processQueue() {
    while (!m_jobQueue.isEmpty() && m_threadPool.activeThreadCount() < m_maxConcurrent) {
        ConversionJob job = m_jobQueue.dequeue();
        
        ConversionWorker* worker = new ConversionWorker(job, m_ffmpeg, this);
        m_threadPool.start(worker);
    }
}
```

### 5.2 Performance Configuration

```cpp
struct PerformanceSettings {
    int maxConcurrentJobs;     // 2-8 depending on CPU
    int threadPriority;         // QThread::LowPriority for background
    int readBufferSizeMB;       // FFmpeg buffer
    bool useHardwareAccel;      // VAAPI/NVENC if available
    
    static PerformanceSettings optimal() {
        PerformanceSettings settings;
        int cpuCores = QThread::idealThreadCount();
        settings.maxConcurrentJobs = qMax(1, cpuCores / 2);
        settings.threadPriority = QThread::LowPriority;
        settings.readBufferSizeMB = 32;
        settings.useHardwareAccel = detectHardwareAccel();
        return settings;
    }
};
```

---

## 6. Integration with Fooyin

### 6.1 Plugin Initialization

```cpp
void ConverterPlugin::initialise(const Fooyin::GuiPluginContext& context)
{
    // Create conversion manager
    m_conversionManager = new ConversionManager(this);
    
    // Create and register widget
    m_widget = new ConverterWidget(m_conversionManager, this);
    context.widgetProvider->registerWidget(
        "AudioConverter",
        [this]() { return new ConverterWidget(m_conversionManager); },
        "Audio Converter"
    );
    
    // Register context menu action
    auto* actionHandler = context.actionHandler;
    actionHandler->addAction(
        "ConvertSelection",
        tr("Convert Selection..."),
        [this](const Fooyin::TrackList& tracks) {
            m_widget->loadTracks(tracks);
            m_widget->show();
        }
    );
    
    // Load settings
    m_conversionManager->loadSettings(context.settingsManager);
}
```

### 6.2 Track Integration

```cpp
void ConverterWidget::loadTracksFromFooyin() {
    // Get selected tracks from fooyin's selection context
    Fooyin::PlaylistHandler* playlistHandler = m_context.playlistHandler;
    Fooyin::TrackList tracks = playlistHandler->selectedTracks();
    
    loadTracks(tracks);
}

void ConverterWidget::loadTracks(const Fooyin::TrackList& tracks) {
    m_fileListWidget->clear();
    
    for (const auto& track : tracks) {
        QListWidgetItem* item = new QListWidgetItem();
        item->setText(track.title() + " - " + track.artist());
        item->setData(Qt::UserRole, QVariant::fromValue(track));
        m_fileListWidget->addItem(item);
    }
    
    updateStats();
}
```

---

## 7. Configuration & Persistence

### 7.1 Settings Structure

```json
{
  "converter": {
    "defaultPreset": "MP3 320kbps",
    "outputDirectory": "/home/user/Music/Converted",
    "namingPattern": "%artist%/%album%/%track% - %title%",
    "preserveDirectory": false,
    "deleteOriginal": false,
    "performance": {
      "maxConcurrentJobs": 4,
      "threadPriority": "low",
      "hardwareAcceleration": true
    },
    "warnings": {
      "lossyToLossy": true,
      "lossyToLossless": true,
      "dsdConversion": true
    },
    "presets": [
      {
        "name": "MP3 320kbps",
        "format": "mp3",
        "codec": "libmp3lame",
        "bitrate": 320,
        "quality": -1,
        "sampleRate": 44100,
        "channels": 2
      }
    ]
  }
}
```

---

## 8. Error Handling & Validation

### 8.1 Pre-Conversion Validation

```cpp
QList<ValidationError> ConversionManager::validateJob(const ConversionJob& job) {
    QList<ValidationError> errors;
    
    // Check input file exists
    if (!QFile::exists(job.inputPath)) {
        errors.append(ValidationError{
            job.id,
            "Input file not found: " + job.inputPath
        });
    }
    
    // Check output directory writable
    QFileInfo outputInfo(job.outputPath);
    QDir outputDir = outputInfo.dir();
    if (!outputDir.exists() && !outputDir.mkpath(".")) {
        errors.append(ValidationError{
            job.id,
            "Cannot create output directory: " + outputDir.path()
        });
    }
    
    // Check disk space
    qint64 estimatedSize = estimateOutputSize(job);
    qint64 availableSpace = getAvailableDiskSpace(job.outputPath);
    if (estimatedSize > availableSpace) {
        errors.append(ValidationError{
            job.id,
            "Insufficient disk space"
        });
    }
    
    // Warn about lossy -> lossy conversion
    if (isLossyFormat(job.inputFormat) && isLossyFormat(job.preset.outputFormat)) {
        if (m_settings.warnings.lossyToLossy) {
            errors.append(ValidationError{
                job.id,
                "Warning: Converting from lossy to lossy format",
                ValidationError::Warning
            });
        }
    }
    
    return errors;
}
```

---

## 9. Build Configuration

### 9.1 CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.14)

project(
    fooyin-converter
    VERSION 1.0.0
    DESCRIPTION "Audio Converter Plugin for Fooyin"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(Fooyin REQUIRED)

set(SOURCES
    converterplugin.cpp
    converterplugin.h
    converterwidget.cpp
    converterwidget.h
    conversionmanager.cpp
    conversionmanager.h
    ffmpegwrapper.cpp
    ffmpegwrapper.h
    presetdatabase.cpp
    presetdatabase.h
    preseteditor.cpp
    preseteditor.h
)

create_fooyin_plugin(
    fooyin-converter
    DEPENDS Fooyin::Core Fooyin::Gui Qt6::Widgets
    SOURCES ${SOURCES}
)

# Install FFmpeg detection helper
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/detect_ffmpeg.sh"
    "${CMAKE_CURRENT_BINARY_DIR}/detect_ffmpeg.sh"
    COPYONLY
)
```

---

## 10. Deployment & Dependencies

### 10.1 Dependency Detection

```bash
#!/bin/bash
# detect_ffmpeg.sh

# Check for ffmpeg binary
if command -v ffmpeg &> /dev/null; then
    FFMPEG_PATH=$(which ffmpeg)
    FFMPEG_VERSION=$(ffmpeg -version | head -n1)
    echo "FFmpeg found: $FFMPEG_PATH"
    echo "Version: $FFMPEG_VERSION"
    exit 0
else
    echo "ERROR: FFmpeg not found in PATH"
    echo "Please install FFmpeg:"
    echo "  Ubuntu/Debian: sudo apt install ffmpeg"
    echo "  Fedora: sudo dnf install ffmpeg"
    echo "  Arch: sudo pacman -S ffmpeg"
    exit 1
fi
```

### 10.2 Runtime Requirements

**Required:**
- FFmpeg 4.0+ (5.0+ recommended)
- Qt 6.2+
- Fooyin 0.8+

**Optional:**
- VAAPI drivers (hardware acceleration)
- NVENC support (NVIDIA GPUs)

---

## 11. Future Enhancements

1. **Parallel Encoding:** Multiple files simultaneously
2. **Cue Sheet Support:** Split/convert cue-based albums
3. **ReplayGain:** Calculate and embed during conversion
4. **Cloud Integration:** Upload to cloud storage post-conversion
5. **Scheduling:** Timed/background conversions
6. **Format Detection:** Auto-select best preset
7. **Dry Run:** Preview operations before execution
8. **Verification:** Post-conversion integrity checks

---

## 12. Implementation Checklist

- [ ] Set up build environment with Fooyin SDK
- [ ] Implement FFmpegWrapper base class
- [ ] Create ConversionManager with queue system
- [ ] Design ConverterWidget UI
- [ ] Implement PresetDatabase with defaults
- [ ] Add threading support
- [ ] Integrate with fooyin plugin system
- [ ] Add progress reporting
- [ ] Implement error handling
- [ ] Create preset editor dialog
- [ ] Add validation logic
- [ ] Test with various audio formats
- [ ] Write documentation
- [ ] Package for distribution

---

## Appendix A: FFmpeg Command Examples

```bash
# MP3 320kbps
ffmpeg -i input.flac -c:a libmp3lame -b:a 320k -id3v2_version 3 output.mp3

# MP3 V0 VBR
ffmpeg -i input.flac -c:a libmp3lame -q:a 0 output.mp3

# FLAC
ffmpeg -i input.wav -c:a flac -compression_level 8 output.flac

# Opus 128kbps
ffmpeg -i input.flac -c:a libopus -b:a 128k output.opus

# AAC 256kbps
ffmpeg -i input.flac -c:a aac -b:a 256k output.m4a

# Batch with progress
ffmpeg -i input.flac -progress pipe:1 -c:a libmp3lame -b:a 320k output.mp3
```

---

## Appendix B: Format Support Matrix

| Input → Output | MP3 | FLAC | Opus | AAC | Ogg | WAV |
|----------------|-----|------|------|-----|-----|-----|
| **FLAC**       | ✓   | ✓    | ✓    | ✓   | ✓   | ✓   |
| **MP3**        | ✓   | ✓    | ✓    | ✓   | ✓   | ✓   |
| **WAV**        | ✓   | ✓    | ✓    | ✓   | ✓   | ✓   |
| **AAC**        | ✓   | ✓    | ✓    | ✓   | ✓   | ✓   |
| **Opus**       | ✓   | ✓    | ✓    | ✓   | ✓   | ✓   |
| **Ogg**        | ✓   | ✓    | ✓    | ✓   | ✓   | ✓   |

All conversions supported via FFmpeg

---

**Document Version:** 1.0  
**Last Updated:** January 16, 2026
