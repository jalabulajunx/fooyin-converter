# Batch Progress Update - Filename Display

## Change Summary

Updated the batch conversion progress display to show the current filename being converted alongside the progress percentage.

## What Changed

### Before
During batch conversion, the status label would show:
```
Converting... 45%
```

This made it unclear which file was being converted, as the filename was only shown initially when starting each track.

### After
During batch conversion, the status label now shows:
```
Converting 3 of 10: my-song.flac (45%)
```

The filename remains visible throughout the conversion, updating with the progress percentage.

## Implementation Details

### Files Modified

**converterwidget.h**
- Added `QString m_currentFilename` member variable to store the current file being converted

**converterwidget.cpp**

1. **processNextTrack()** - Line ~533:
   ```cpp
   // Store current filename for progress updates
   m_currentFilename = info.fileName();
   ```
   Stores the filename when starting each track conversion.

2. **onProgress()** - Line ~411:
   ```cpp
   void ConverterWidget::onProgress(int percent)
   {
       m_progressBar->setValue(percent);

       // In batch mode, show the filename with progress
       if (!m_trackQueue.isEmpty() && m_currentTrackIndex >= 0) {
           m_statusLabel->setText(QString("Converting %1 of %2: %3 (%4%)")
               .arg(m_currentTrackIndex + 1)
               .arg(m_totalTracks)
               .arg(m_currentFilename)
               .arg(percent));
       } else {
           // Single file mode
           m_statusLabel->setText(QString("Converting... %1%").arg(percent));
       }
   }
   ```
   Updates the status label to include the filename during progress updates in batch mode.

## Status Display Format

### Single Track Mode (unchanged)
```
Converting... 45%
```

### Batch Mode (updated)
```
Converting [current] of [total]: [filename] ([percent]%)
```

**Example:**
```
Converting 3 of 10: 04-song-title.flac (67%)
```

## Benefits

1. **Better User Feedback**: Users can see which file is currently being converted
2. **Progress Tracking**: Easy to identify if conversion is stuck on a particular file
3. **File Identification**: Helps users verify the correct files are being processed
4. **Consistent Information**: Filename remains visible throughout the entire conversion, not just at the start

## Installation

```bash
cd /home/radnus/Projects/fooyin_conversion/fooyin-converter/build
sudo cp fyplugin_fooyin-converter.so /usr/lib/fooyin/plugins/
```

Then restart Fooyin.

## Testing

To test the improved batch progress:

1. Select 5-10 tracks in Fooyin
2. Right-click → Convert Audio...
3. Choose format and quality
4. Click Convert
5. Observe the status label showing:
   - Track number (e.g., "3 of 10")
   - Filename (e.g., "my-song.flac")
   - Progress percentage (e.g., "67%")
   - All in one line: "Converting 3 of 10: my-song.flac (67%)"

---

**Update**: ✅ COMPLETE
**Build Status**: ✅ SUCCESS
**Plugin Size**: 272 KB
**Ready For**: Installation and testing
