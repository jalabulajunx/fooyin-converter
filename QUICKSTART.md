# Quick Start Guide

## Prerequisites

Install the codec tools you want to use:

```bash
# Install all codecs (recommended)
sudo apt install flac lame opus-tools vorbis-tools  # Debian/Ubuntu
sudo dnf install flac lame opus-tools vorbis-tools  # Fedora
sudo pacman -S flac lame opus-tools vorbis-tools    # Arch
```

Verify installation:
```bash
which flac lame opusenc oggenc
```

## Build Steps

```bash
# 1. Navigate to plugin directory
cd fooyin-converter

# 2. Create build directory
mkdir build && cd build

# 3. Configure (assuming Fooyin is in /usr or /usr/local)
cmake .. -DCMAKE_BUILD_TYPE=Release

# 4. Build
cmake --build .

# 5. Install (may need sudo)
sudo cmake --install .
# OR for user install:
# cmake --install . --prefix ~/.local
```

## Enable in Fooyin

1. Launch Fooyin
2. Settings → Plugins
3. Enable "Audio Converter"
4. Restart Fooyin

## Add Widget

1. View → Layout Editing
2. Right-click → Add Widget → Audio Converter
3. Position it in your layout
4. Exit Layout Editing

## First Conversion

1. Click "Browse" to select an audio file
2. Choose output format (e.g., MP3)
3. Select quality (e.g., 320 kbps)
4. Click "Convert"
5. Done!

## Troubleshooting

### Plugin doesn't appear in Settings
- Check build completed without errors
- Verify plugin was installed to correct directory
- Check Fooyin logs: `~/.config/fooyin/logs/fooyin.log`

### "No codecs available"
- Install codec tools (see Prerequisites)
- Restart Fooyin after installing codecs
- Check with: `which flac lame opusenc oggenc`

### Build fails with "Fooyin not found"
```bash
# Specify Fooyin location explicitly
cmake .. -DCMAKE_PREFIX_PATH=/path/to/fooyin/install
```

### Permission denied during install
```bash
# Either use sudo:
sudo cmake --install .

# Or install to user directory:
cmake --install . --prefix ~/.local
```

## Need Help?

- Read full [README.md](README.md)
- Check [architecture documentation](../fooyin_converter_architecture.md)
- Report issues on GitHub
