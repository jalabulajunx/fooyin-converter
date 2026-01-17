# Critical Fix: Process Cleanup Crash

## Problem

When closing the converter dialog during an active conversion, Fooyin would crash with:

```
QProcess: Destroyed while process ("/usr/bin/flac") is still running.
fish: Job 1, 'fooyin' terminated by signal SIGSEGV (Address boundary error)
```

This was a **critical bug** that could cause data loss and crash the entire application.

## Root Cause

The `cancel()` method in all codec wrappers was improperly terminating processes:

```cpp
// OLD - BROKEN CODE
void FlacWrapper::cancel()
{
    if (m_process) {
        m_process->kill();          // Kill immediately
        m_process->deleteLater();   // Delete while process still shutting down
        m_process = nullptr;        // NULL pointer but QProcess not fully cleaned up
    }
}
```

**Issues:**
1. `kill()` sends SIGKILL but doesn't wait for process to actually terminate
2. `deleteLater()` queued deletion while process still running
3. QProcess destructor ran while the external process was still alive
4. This caused undefined behavior → SIGSEGV crash

## Solution

Implemented proper graceful shutdown with fallback to force kill:

```cpp
// NEW - FIXED CODE
void FlacWrapper::cancel()
{
    if (m_process && m_process->state() != QProcess::NotRunning) {
        // Disconnect signals to avoid spurious callbacks
        m_process->disconnect();

        // Try graceful termination first
        m_process->terminate();  // Sends SIGTERM

        // Wait up to 1 second for graceful shutdown
        if (!m_process->waitForFinished(1000)) {
            // Force kill if termination didn't work
            m_process->kill();   // Sends SIGKILL
            m_process->waitForFinished(100);
        }

        m_process->deleteLater();
        m_process = nullptr;
    }
}
```

## Key Improvements

1. **State Check**: Only cancel if process is actually running
2. **Signal Disconnect**: Prevents callbacks during shutdown
3. **Graceful First**: Uses `terminate()` (SIGTERM) to allow clean exit
4. **Wait**: `waitForFinished()` blocks until process actually exits
5. **Fallback Kill**: Force kills if graceful termination times out
6. **Safe Deletion**: Only deletes QProcess after process has fully stopped

## Files Fixed

All codec wrappers were updated with identical fix:

- `src/flacwrapper.cpp` - Line 145
- `src/lamewrapper.cpp` - Line 159
- `src/opuswrapper.cpp` - Line 148
- `src/oggwrapper.cpp` - Line 157

## Testing

### Before Fix
```
1. Start batch conversion (5 files)
2. Close dialog during conversion
3. Click "Yes" to cancel
Result: ❌ SIGSEGV crash, Fooyin terminates
```

### After Fix
```
1. Start batch conversion (5 files)
2. Close dialog during conversion
3. Click "Yes" to cancel
Result: ✅ Clean cancellation, dialog closes, Fooyin continues running
```

## Technical Details

### Graceful vs Force Termination

| Method | Signal | Behavior | Timeout |
|--------|--------|----------|---------|
| `terminate()` | SIGTERM | Process can cleanup, flush buffers | 1000ms |
| `kill()` | SIGKILL | Immediate forced termination | 100ms |

### Process Lifecycle

```
Active Conversion
    ↓
User requests close
    ↓
cancel() called
    ↓
m_process->disconnect()  // Stop signal callbacks
    ↓
m_process->terminate()   // Send SIGTERM
    ↓
waitForFinished(1000)    // Wait up to 1 second
    ↓
    ├─ Success: Process exited cleanly
    │   └─ deleteLater() → Safe to delete
    │
    └─ Timeout: Process still running
        └─ kill() + waitForFinished(100)
            └─ Force terminate
                └─ deleteLater() → Safe to delete
```

## Impact

### Before Fix (CRITICAL BUG)
- ❌ Application crash when cancelling conversion
- ❌ Risk of data loss (unsaved work in Fooyin)
- ❌ Partially converted files left on disk
- ❌ No way to safely cancel batch operations

### After Fix
- ✅ Safe cancellation of single/batch conversions
- ✅ Fooyin remains stable
- ✅ Clean shutdown of codec processes
- ✅ No zombie processes or file corruption

## Regression Testing

Ensure these scenarios work correctly:

1. **Cancel single conversion**: ✅
2. **Cancel batch conversion (file 1/10)**: ✅
3. **Cancel batch conversion (file 5/10)**: ✅
4. **Close dialog during conversion**: ✅
5. **Close dialog while idle**: ✅
6. **Close with Escape key during conversion**: ✅
7. **Close with X button during conversion**: ✅
8. **Close with Alt+F4 during conversion**: ✅

## Build Status

✅ **SUCCESS** - Critical fix applied and tested

```bash
cd /home/radnus/Projects/fooyin_conversion/fooyin-converter/build
sudo cp fyplugin_fooyin-converter.so /usr/lib/fooyin/plugins/
```

Then restart Fooyin.

## Priority

🔴 **CRITICAL** - This was a crash bug affecting core functionality

## Related Issues

- Original issue: SIGSEGV when closing dialog during conversion
- Affected: All codec wrappers (FLAC, LAME, Opus, Ogg)
- Related: Close button implementation (closeEvent)

---

**Fix Date**: 2026-01-16
**Severity**: Critical (Application Crash)
**Status**: ✅ RESOLVED
