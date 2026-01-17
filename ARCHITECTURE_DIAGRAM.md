# Architecture Diagram

## System Overview

```
┌─────────────────────────────────────────────────────────────┐
│                    Fooyin Music Player                       │
│                                                              │
│  ┌────────────────────────────────────────────────────────┐ │
│  │                  Plugin System                          │ │
│  │                                                          │ │
│  │   ┌──────────────────────────────────────────────┐     │ │
│  │   │          ConverterPlugin                      │     │ │
│  │   │   (CorePlugin + GuiPlugin Entry Point)        │     │ │
│  │   │  - initialise(CorePluginContext)              │     │ │
│  │   │    → Registers settings                       │     │ │
│  │   │  - initialise(GuiPluginContext)               │     │ │
│  │   │    → Registers widget & context menu          │     │ │
│  │   └────────────┬─────────────────────────────────┘     │ │
│  │                │                                         │ │
│  │                │ creates & owns                          │ │
│  │                ▼                                         │ │
│  │   ┌──────────────────────────────────────────────┐     │ │
│  │   │       ConversionManager                       │     │ │
│  │   │  - Detects & manages codec wrappers           │     │ │
│  │   │  - Routes conversion requests                 │     │ │
│  │   │  - Emits progress/completion signals          │     │ │
│  │   └────────────┬─────────────────────────────────┘     │ │
│  │                │                                         │ │
│  │                │ used by                                 │ │
│  │                ▼                                         │ │
│  │   ┌──────────────────────────────────────────────┐     │ │
│  │   │          ConverterWidget                      │     │ │
│  │   │     (Qt UI Widget + Dialog)                   │     │ │
│  │   │  - Single/batch track loading                 │     │ │
│  │   │  - Format/quality controls                    │     │ │
│  │   │  - Progress display with reset                │     │ │
│  │   │  - Sequential batch processing                │     │ │
│  │   │  - Applies settings (size, default codec)     │     │ │
│  │   └───────────────────────────────────────────────┘     │ │
│  │                                                          │ │
│  │   ┌──────────────────────────────────────────────┐     │ │
│  │   │       ConverterSettingsPage                   │     │ │
│  │   │  - Window size configuration                  │     │ │
│  │   │  - Default codec selection                    │     │ │
│  │   │  - Located in: Plugins → Audio Converter      │     │ │
│  │   └───────────────────────────────────────────────┘     │ │
│  │                                                          │ │
│  └──────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
                            │
                            │ Qt Signals/Slots
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                   ConversionManager                          │
│                                                              │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐   │
│  │  Codec   │  │  Codec   │  │  Codec   │  │  Codec   │   │
│  │   Map    │  │   Map    │  │   Map    │  │   Map    │   │
│  │ "flac"   │  │  "mp3"   │  │ "opus"   │  │  "ogg"   │   │
│  │    ↓     │  │    ↓     │  │    ↓     │  │    ↓     │   │
│  │ ┌──────┐ │  │ ┌──────┐ │  │ ┌──────┐ │  │ ┌──────┐ │   │
│  │ │ FLAC │ │  │ │ LAME │ │  │ │ Opus │ │  │ │ Ogg  │ │   │
│  │ │Wrapper│ │  │ │Wrapper│ │  │ │Wrapper│ │  │ │Wrapper│ │   │
│  │ └──────┘ │  │ └──────┘ │  │ └──────┘ │  │ └──────┘ │   │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘   │
└────────┬──────────┬──────────┬──────────┬──────────────────┘
         │          │          │          │
         │ QProcess │ QProcess │ QProcess │ QProcess
         ▼          ▼          ▼          ▼
┌─────────────────────────────────────────────────────────────┐
│        External Codec Executables (Dedicated Binaries)      │
│          (Recommended by Naren over FFmpeg)                  │
│                                                              │
│    ┌──────┐    ┌──────┐    ┌──────┐    ┌──────┐           │
│    │ flac │    │ lame │    │opusenc│   │oggenc│           │
│    │ v1.x │    │ v3.x │    │ v0.x  │   │ v1.x │           │
│    │ CLI  │    │ CLI  │    │ CLI   │   │ CLI  │           │
│    └──────┘    └──────┘    └──────┘    └──────┘           │
│                                                              │
│  Benefits: Better quality (esp. LAME MP3), simpler debug,   │
│  predictable behavior, direct control, Unix philosophy       │
└─────────────────────────────────────────────────────────────┘
```

## Class Hierarchy

```
CodecWrapper (abstract base)
│
├── FlacWrapper
│   ├── isAvailable() → checks for 'flac' executable
│   ├── version() → queries flac --version
│   ├── convert() → builds args, runs synchronously
│   ├── convertAsync() → creates QProcess, monitors progress
│   └── parseProgress() → extracts "XX% complete"
│
├── LameWrapper
│   ├── isAvailable() → checks for 'lame' executable
│   ├── version() → queries lame --version
│   ├── convert() → builds args, runs synchronously
│   ├── convertAsync() → creates QProcess, monitors progress
│   └── parseProgress() → extracts "(XX%)"
│
├── OpusWrapper
│   ├── isAvailable() → checks for 'opusenc' executable
│   ├── version() → queries opusenc --version
│   ├── convert() → builds args, runs synchronously
│   ├── convertAsync() → creates QProcess, monitors progress
│   └── parseProgress() → extracts "[XX%]"
│
└── OggWrapper
    ├── isAvailable() → checks for 'oggenc' executable
    ├── version() → queries oggenc --version
    ├── convert() → builds args, runs synchronously
    ├── convertAsync() → creates QProcess, monitors progress
    └── parseProgress() → extracts "[XX.X%]"
```

## Data Flow: User Conversion Request

### Single Track Conversion
```
User right-clicks track → "Convert Audio..."
        │
        ▼
ConverterPlugin::showConverterDialog()
        │
        ├─── Get selected track(s) from TrackSelectionController
        ├─── Create/reuse ConverterWidget with SettingsManager
        ├─── Apply window size from settings
        ├─── Call loadTrack(filepath) for single track
        │
        ▼
ConverterWidget::loadTrack(filepath)
        │
        ├─── Set input path
        ├─── Generate output path (same dir, new extension)
        ├─── Apply default codec from settings
        ├─── Reset progress bar
        │
        ▼
User clicks "Convert"
        │
        ▼
ConverterWidget::startConversion()
        │
        ├─── Validate input/output paths
        ├─── Build ConversionOptions struct
        │    {format, bitrate, quality, sampleRate, channels}
        │
        ▼
ConversionManager::convertAsync(input, output, options)
        │
        ├─── Get codec wrapper for format
        ├─── Check codec availability
        ├─── Connect progress/completion signals
        │
        ▼
CodecWrapper::convertAsync(input, output, options)
        │
        ├─── Build command-line arguments
        ├─── Create QProcess
        ├─── Connect stdout/stderr monitoring
        ├─── Start external process
        │
        ▼
External codec process (e.g., lame)
        │
        ├─── Reads input file
        ├─── Encodes audio
        ├─── Writes output file
        ├─── Outputs progress to stderr
        │
        ▼
CodecWrapper::parseProgress()
        │
        └─── Extract percentage
             │
             ▼
        emit progressChanged(percent)
             │
             ▼
ConversionManager (relays signal)
             │
             ▼
        emit progressChanged(percent)
             │
             ▼
ConverterWidget::onProgress(percent)
             │
             └─── Update progress bar
                  Update status label

When process completes:
        │
        ▼
CodecWrapper::finished slot
        │
        ├─── Check exit code
        ├─── Read stderr if error
        │
        ▼
        emit conversionFinished(success, error)
             │
             ▼
ConversionManager (relays signal)
             │
             ▼
        emit conversionFinished(success, error)
             │
             ▼
ConverterWidget::onFinished(success, error)
             │
             └─── Show success/error dialog
                  Reset UI state
```

### Batch Track Conversion
```
User selects multiple tracks → right-click → "Convert Audio..."
        │
        ▼
ConverterPlugin::showConverterDialog()
        │
        ├─── Detect tracks.size() > 1
        ├─── Build QStringList of filepaths
        ├─── Call loadTracks(filepaths) for batch
        │
        ▼
ConverterWidget::loadTracks(filepaths)
        │
        ├─── Store tracks in m_trackQueue
        ├─── Set m_totalTracks = filepaths.size()
        ├─── Set m_currentTrackIndex = -1
        ├─── Show "X files selected" in input
        ├─── Show output directory (from first file)
        │
        ▼
User clicks "Convert"
        │
        ▼
ConverterWidget::startConversion()
        │
        ├─── Detect batch mode (m_trackQueue not empty)
        ├─── Call processNextTrack()
        │
        ▼
ConverterWidget::processNextTrack()
        │
        ├─── Increment m_currentTrackIndex
        ├─── Get filepath from m_trackQueue[index]
        ├─── Generate output path in output directory
        ├─── Update status: "Converting X of Y: filename"
        ├─── Build ConversionOptions
        ├─── Start async conversion
        │
        ▼
ConversionManager::convertAsync(...)
        │
        └─── [same codec process as single track]
             │
             ▼
        When conversion completes:
             │
             ▼
ConverterWidget::onFinished(success, error)
        │
        ├─── Check if batch mode (m_trackQueue not empty)
        ├─── Log error if failed (but continue)
        ├─── Call processNextTrack() for next file
        │
        └─── If all done (index >= queue.size()):
             └─── Show "Batch conversion completed!" dialog
```

## Signal/Slot Connections

```
CodecWrapper
    │
    ├─ progressChanged(int) ────────────┐
    │                                    │
    └─ conversionFinished(bool, QString)┤
                                         │
                                         ▼
                            ConversionManager
                                         │
                            ├─ progressChanged(int) ──────┐
                            │                             │
                            ├─ conversionFinished(...)─────┤
                            │                             │
                            └─ conversionStarted() ───────┤
                                                          │
                                                          ▼
                                              ConverterWidget
                                                          │
                                              ├─ onProgress(int)
                                              ├─ onFinished(bool, QString)
                                              └─ onStarted()
                                                          │
                                                          ▼
                                                    UI Updates
```

## File Organization

```
fooyin-converter/
│
├── metadata.json                    # Plugin metadata
├── CMakeLists.txt                   # Build system
│
└── src/
    │
    ├── converterplugin.{h,cpp}         # Fooyin plugin interface
    │   ├── CorePlugin::initialise() → registers settings
    │   └── GuiPlugin::initialise() → registers widget & menu
    │
    ├── convertersettings.h             # Settings enum definitions
    │   └── DefaultCodec, WindowWidth, WindowHeight
    │
    ├── convertersettingspage.{h,cpp}   # Settings UI page
    │   ├── Window size controls
    │   ├── Default codec selector
    │   └── Apply/reset handlers
    │
    ├── converterwidget.{h,cpp}         # Qt UI widget + dialog
    │   ├── setupUI() → builds interface
    │   ├── loadTrack() → single track mode
    │   ├── loadTracks() → batch mode
    │   ├── processNextTrack() → sequential batch processing
    │   ├── startConversion() → validates & starts
    │   ├── applyDefaultCodec() → applies setting
    │   └── signal handlers → updates UI
    │
    ├── conversionmanager.{h,cpp}       # Conversion coordinator
    │   ├── Owns all codec wrappers
    │   ├── Detects installed codecs
    │   ├── Routes requests to appropriate codec
    │   └── Relays signals to widget
    │
    ├── codecwrapper.{h,cpp}            # Abstract base class
    │   ├── Virtual interface for codecs
    │   └── Common utility functions
    │
    ├── flacwrapper.{h,cpp}             # FLAC via 'flac' binary
    ├── lamewrapper.{h,cpp}             # MP3 via 'lame' binary
    ├── opuswrapper.{h,cpp}             # Opus via 'opusenc' binary
    └── oggwrapper.{h,cpp}              # Ogg Vorbis via 'oggenc' binary
```

## Memory Management

```
ConverterPlugin (QObject)
    │
    └─ owns → ConversionManager (QObject)
                   │
                   ├─ owns → FlacWrapper (QObject)
                   ├─ owns → LameWrapper (QObject)
                   ├─ owns → OpusWrapper (QObject)
                   └─ owns → OggWrapper (QObject)
                                    │
                                    └─ creates → QProcess (temporary)
                                                      │
                                                      └─ deleteLater() on completion

ConverterWidget (created by Fooyin's widget factory)
    │
    └─ references (not owns) → ConversionManager
```

## Thread Safety

```
Main Thread (Qt Event Loop)
    │
    ├─ UI events (button clicks)
    ├─ Signal/slot connections
    └─ QProcess management
         │
         └─── Spawns external process
                   │
                   └─ Codec executable (separate process, not thread)
                        │
                        └─ Communicates via stdout/stderr
                             │
                             └─ Parsed in main thread via signals
```

## Plugin Lifecycle

```
Fooyin starts
    │
    └─ Loads enabled plugins
         │
         └─ ConverterPlugin (CorePlugin + GuiPlugin)
              │
              ├─ CorePlugin::initialise(CorePluginContext)
              │   │
              │   ├─ Get SettingsManager reference
              │   ├─ Register settings:
              │   │   - DefaultCodec (String, default: "flac")
              │   │   - WindowWidth (Int, default: 600)
              │   │   - WindowHeight (Int, default: 500)
              │   └─ Log: "Settings registered"
              │
              └─ GuiPlugin::initialise(GuiPluginContext)
                   │
                   ├─ Create ConversionManager
                   ├─ Check codec availability (which flac, lame, etc.)
                   ├─ Log available codecs
                   ├─ Register ConverterSettingsPage
                   ├─ Register widget factory
                   └─ Register context menu action
                        │
                        └─ "Convert Audio..." in track selection menu

User right-clicks track(s) → "Convert Audio..."
    │
    └─ ConverterPlugin::showConverterDialog()
         │
         ├─ Get selected tracks from TrackSelectionController
         ├─ Create/reuse ConverterWidget (with SettingsManager)
         ├─ Apply window size from settings
         ├─ Apply default codec from settings
         ├─ Load track(s) - single or batch mode
         └─ Show dialog

Fooyin exits
    │
    └─ Destroys plugins
         │
         └─ ~ConverterPlugin()
              │
              └─ Qt parent-child cleanup
                   │
                   └─ All child QObjects destroyed
```

## Configuration and State

### Persistent Configuration
```
Settings stored in: ~/.config/fooyin/fooyin.conf

[AudioConverter]
DefaultCodec=flac          # Pre-selected format (flac/mp3/opus/ogg)
WindowWidth=600            # Dialog width in pixels (400-1920)
WindowHeight=500           # Dialog height in pixels (300-1080)
```

### Runtime State (in memory)
```
ConversionManager
    │
    ├─ m_codecMap (format → wrapper)
    ├─ m_currentCodec (active during conversion)
    └─ m_converting (boolean flag)

ConverterWidget
    │
    ├─ Single Track Mode:
    │   ├─ Current input path
    │   ├─ Current output path
    │   ├─ Selected format
    │   ├─ Selected quality
    │   ├─ Sample rate
    │   └─ Channel config
    │
    └─ Batch Mode (additional):
        ├─ m_trackQueue (QStringList of filepaths)
        ├─ m_currentTrackIndex (int, -1 when not active)
        └─ m_totalTracks (int, total count)
```

## Error Handling Flow

```
Error occurs
    │
    ├─ Codec not found
    │   └─ Checked at initialization
    │       └─ Logged to console
    │           └─ Format not shown in UI
    │
    ├─ Invalid input file
    │   └─ Checked in validateInput()
    │       └─ QMessageBox shown
    │           └─ Conversion not started
    │
    ├─ Process fails to start
    │   └─ QProcess::waitForStarted() fails
    │       └─ Emit conversionFinished(false, error)
    │           └─ Error dialog shown
    │
    └─ Process exits with error
        └─ Exit code != 0
            └─ Read stderr
                └─ Emit conversionFinished(false, error)
                    └─ Error dialog shown
```

## Implementation Notes

**Design Decision: Dedicated Codec Binaries over FFmpeg**

This plugin uses individual codec executables (flac, lame, opusenc, oggenc) rather than FFmpeg, based on the recommendation by **Naren**. This approach offers several key advantages:

1. **Superior Quality**: LAME encoder produces better MP3 quality than FFmpeg's built-in MP3 encoder
2. **Simpler Debugging**: Individual tools are easier to troubleshoot and test
3. **Predictable Behavior**: Each tool has well-documented, stable behavior
4. **Direct Control**: Access to all codec-specific options and features
5. **Unix Philosophy**: Follows the "do one thing well" principle
6. **Easier Distribution**: System package managers already provide these tools
7. **Better Error Messages**: Codec-specific error output is clearer

This architecture provides:
- **Modularity**: Each codec is independent
- **Extensibility**: Easy to add new codecs (just add another wrapper)
- **Maintainability**: Clear separation of concerns
- **Testability**: Each component can be tested in isolation
- **Reliability**: Proper error handling and state management
- **Configurability**: User preferences for window size and default codec
- **Batch Processing**: Sequential multi-file conversion support
