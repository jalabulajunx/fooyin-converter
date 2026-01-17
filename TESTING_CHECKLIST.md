# Testing and Deployment Checklist

## Pre-Build Checklist

### Environment Setup
- [ ] Qt 6.2+ installed and in PATH
- [ ] CMake 3.18+ installed
- [ ] Fooyin installed with development headers
- [ ] C++ compiler supporting C++20 (GCC 10+, Clang 11+)
- [ ] At least one codec tool installed

### Codec Tools Installation
- [ ] FLAC encoder (`flac` command available)
- [ ] LAME MP3 encoder (`lame` command available)
- [ ] Opus encoder (`opusenc` command available)
- [ ] Ogg Vorbis encoder (`oggenc` command available)

Verify with:
```bash
which flac lame opusenc oggenc
flac --version
lame --version
opusenc --version
oggenc --version
```

## Build Testing

### Compilation
- [ ] Clean build succeeds without errors
- [ ] No compiler warnings
- [ ] All source files compile
- [ ] MOC files generated correctly
- [ ] Plugin shared library created (.so, .dylib, or .dll)

```bash
cd fooyin-converter
rm -rf build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build . --verbose
```

### Installation
- [ ] Plugin installs to correct directory
- [ ] metadata.json copied alongside plugin
- [ ] No permission errors during install
- [ ] Plugin file has correct permissions

```bash
sudo cmake --install .
# Verify installation
ls -la /usr/local/lib/fooyin/plugins/ | grep converter
```

## Fooyin Integration Testing

### Plugin Loading
- [ ] Fooyin starts without errors
- [ ] Plugin appears in Settings → Plugins
- [ ] Plugin can be enabled
- [ ] Restart after enabling works
- [ ] Plugin initializes successfully

Check logs:
```bash
tail -f ~/.config/fooyin/logs/fooyin.log
```

Expected log lines:
```
Audio Converter - Available codecs:
  flac - X.X.X
  mp3 - X.X.X
  ...
Audio Converter Plugin initialized successfully
Audio Converter widget registered
```

### Widget Integration
- [ ] Widget appears in Add Widget menu
- [ ] Widget can be added to layout
- [ ] Widget displays correctly
- [ ] Widget is resizable
- [ ] Widget persists across Fooyin restarts

## Functional Testing

### UI Functionality
- [ ] Input browse button opens file dialog
- [ ] Input field accepts drag-and-drop
- [ ] Output path auto-generates correctly
- [ ] Output browse button opens save dialog
- [ ] Format dropdown shows available codecs only
- [ ] Quality options change based on format
- [ ] Sample rate spinner works (0 = Original)
- [ ] Channels dropdown works
- [ ] Convert button enables when inputs valid
- [ ] Cancel button only enabled during conversion

### Format-Specific Testing

#### FLAC Conversion
Input: Any audio file (WAV, MP3, etc.)
- [ ] FLAC output created successfully
- [ ] File is valid FLAC (check with `flac -t output.flac`)
- [ ] Compression level 0 (fastest) works
- [ ] Compression level 8 (best) works
- [ ] Original sample rate preserved when set to 0
- [ ] Metadata copied correctly

#### MP3 Conversion
Input: FLAC or WAV file
- [ ] MP3 320 kbps CBR works
- [ ] MP3 192 kbps CBR works
- [ ] MP3 V0 VBR works
- [ ] MP3 V2 VBR works
- [ ] ID3 tags preserved
- [ ] File playable in other players

#### Opus Conversion
Input: Any audio file
- [ ] Opus 256 kbps works
- [ ] Opus 128 kbps works
- [ ] Opus 64 kbps works
- [ ] Output is valid Opus file
- [ ] Resampling to 48kHz works automatically
- [ ] Metadata preserved

#### Ogg Vorbis Conversion
Input: Any audio file
- [ ] Ogg quality 10 works
- [ ] Ogg quality 6 works
- [ ] Ogg quality 2 works
- [ ] Output is valid Ogg file
- [ ] Metadata preserved

### Progress Tracking
- [ ] Progress bar updates during conversion
- [ ] Status label shows percentage
- [ ] Progress appears smooth (not jumpy)
- [ ] Progress reaches 100% on completion
- [ ] Progress resets between conversions

### Error Handling

#### Input Validation
- [ ] Empty input shows error
- [ ] Non-existent file shows error
- [ ] Invalid audio file handled gracefully
- [ ] Locked/permission-denied file shows error

#### Output Validation
- [ ] Empty output shows error
- [ ] Same input/output shows error
- [ ] Read-only directory shows error
- [ ] Insufficient disk space detected (if possible)

#### Codec Unavailability
- [ ] Missing codec shows error message
- [ ] Error message suggests installing codec
- [ ] Format not shown if codec unavailable

### Cancellation
- [ ] Cancel button works during conversion
- [ ] Partial output file cleaned up
- [ ] UI returns to ready state
- [ ] Can start new conversion after cancel

### Edge Cases
- [ ] Very large file (>1GB) converts successfully
- [ ] Very small file (<100KB) converts successfully
- [ ] File with special characters in name
- [ ] File with spaces in path
- [ ] Unicode characters in metadata
- [ ] Empty metadata tags
- [ ] Corrupted input file (should fail gracefully)

## Performance Testing

### Conversion Speed
- [ ] FLAC encoding uses multiple cores
- [ ] No UI freezing during conversion
- [ ] Memory usage remains reasonable
- [ ] CPU usage appropriate for encoding

### Resource Usage
Monitor with:
```bash
# During conversion
top -p $(pgrep fooyin)
```

Expected:
- [ ] No memory leaks (check with valgrind if needed)
- [ ] CPU usage high (expected for encoding)
- [ ] No excessive I/O wait
- [ ] Responsive UI during conversion

## Cross-Platform Testing (if applicable)

### Linux
- [ ] Works on Ubuntu/Debian
- [ ] Works on Fedora/RHEL
- [ ] Works on Arch Linux
- [ ] Codec detection works correctly

### macOS (if supported)
- [ ] Builds with Homebrew Qt
- [ ] Codec detection works
- [ ] File dialogs work correctly

### Windows (if supported)
- [ ] Builds with MSVC or MinGW
- [ ] Codec detection works
- [ ] Path handling works (backslashes)

## Regression Testing

After any code changes:
- [ ] All previous tests still pass
- [ ] No new warnings or errors
- [ ] Plugin still loads correctly
- [ ] Conversions still work

## Documentation Testing

### README.md
- [ ] All commands work as written
- [ ] Links are valid
- [ ] Examples are accurate
- [ ] Screenshots (if any) are current

### QUICKSTART.md
- [ ] Can follow from start to finish
- [ ] No missing steps
- [ ] Troubleshooting section helpful

### Code Comments
- [ ] Public APIs documented
- [ ] Complex logic explained
- [ ] Header files have class descriptions

## Deployment Checklist

### Pre-Release
- [ ] Version number updated in metadata.json
- [ ] Version number updated in CMakeLists.txt
- [ ] CHANGELOG.md created (if not exists)
- [ ] All tests passing
- [ ] No known critical bugs
- [ ] Documentation up to date

### Release Package
- [ ] Source tarball created
- [ ] Installation instructions tested
- [ ] LICENSE file included
- [ ] README and docs included
- [ ] Git tags created

### Post-Release
- [ ] Announce on Fooyin Discord/forums
- [ ] Submit to package managers (AUR, etc.)
- [ ] Monitor for bug reports
- [ ] Prepare fixes for common issues

## Bug Report Template

When issues are found, document:
```
**Environment:**
- OS:
- Fooyin version:
- Plugin version:
- Codec version:

**Steps to reproduce:**
1.
2.
3.

**Expected behavior:**

**Actual behavior:**

**Logs:**
```

## Performance Benchmarks

Document conversion times for reference:

```
Test File: 5 minute audio (44.1kHz stereo FLAC)
System: [CPU, RAM]

Format              Time     Output Size
------              ----     -----------
FLAC level 8        XXs      XX MB
FLAC level 0        XXs      XX MB
MP3 320k            XXs      XX MB
MP3 V0              XXs      XX MB
Opus 128k           XXs      XX MB
Ogg quality 8       XXs      XX MB
```

## Sign-Off

- [ ] All critical tests passed
- [ ] Documentation complete
- [ ] Ready for user testing
- [ ] Ready for beta release
- [ ] Ready for stable release

**Tested by:** _______________
**Date:** _______________
**Version:** _______________
**Platform:** _______________
