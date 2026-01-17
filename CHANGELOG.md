# Changelog

## [Unreleased] - 2026-01-16

### Fixed
1. **Extension Update** - Output file extension now updates automatically when changing format
2. **Modal Dialog** - Converter now opens as a modal dialog instead of a separate window
3. **Progress Display** - Removed `--silent`/`--quiet` flags from all codec wrappers to enable progress output
   - LAME: Changed from `--silent` to `--nohist`
   - FLAC: Removed `--silent` flag
   - Opus: Removed `--quiet` flag
   - Ogg Vorbis: Removed `--quiet` flag

### Added
- `loadTrack()` method to pre-populate converter with selected track filepath
- Auto-generation of output path based on input file location

### Changed
- Dialog window flags set to `Qt::Dialog | Qt::WindowCloseButtonHint`
- Dialog window modality set to `Qt::ApplicationModal`

## TODO - Batch Conversion

### Requirements
When multiple tracks are selected:
1. **UI Changes**:
   - Show folder selector instead of file selector for input
   - Show folder selector for output directory
   - Display list of selected tracks
   - Show progress for each track individually
   - Show overall progress indicator

2. **Processing**:
   - Process tracks sequentially (one at a time)
   - Update per-track progress
   - Update overall progress
   - Handle errors per-track (continue with others)

3. **Implementation Plan**:
   - Detect if multiple tracks selected in `showConverterDialog()`
   - Create separate widget or mode for batch conversion
   - Add queue management for multiple conversions
   - Add QTableWidget to show track list with progress
   - Add overall progress bar

## Installation

```bash
sudo cp fyplugin_fooyin-converter.so /usr/lib/fooyin/plugins/
```

Then restart Fooyin.
