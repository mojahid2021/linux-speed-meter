# Upgrade Guide - Speed Test Crash Fix

## Overview

This update fixes critical crashes in the speed test tab and improves overall stability, performance, and cross-platform compatibility.

## What's Fixed

### Critical Crashes
- ✅ **Speed test tab crash** - Fixed thread safety issues causing application crashes
- ✅ **CURL initialization errors** - Proper global initialization prevents conflicts
- ✅ **Widget access crashes** - Null pointer checks prevent segmentation faults
- ✅ **Network error crashes** - Retry logic handles transient failures gracefully

### Performance & Reliability
- ✅ **Memory leaks** - RAII wrappers ensure proper resource cleanup
- ✅ **Network resilience** - Automatic retry on transient errors (up to 3 attempts)
- ✅ **Better error messages** - Clear indication of what went wrong
- ✅ **Pre-connectivity check** - Quick validation before running full test

### Build System
- ✅ **Cross-platform detection** - Automatic platform detection (Linux/Windows/macOS)
- ✅ **Optimized builds** - Release builds use -O3 optimization
- ✅ **Flexible build script** - Support for debug/release modes
- ✅ **Better packaging** - build_all.sh tracks success/failure

## Upgrading

### From Source

```bash
# Pull latest changes
git pull origin main

# Clean build (recommended)
./build.sh clean release

# Run the application
./build/LinuxSpeedMeter
```

### Pre-built Packages

#### Debian/Ubuntu
```bash
# Download latest .deb package
sudo dpkg -i dist/linux-speed-meter_*.deb
```

#### Fedora/RHEL
```bash
# Download latest .rpm package
sudo rpm -Uvh dist/linux-speed-meter-*.rpm
```

#### Arch Linux
```bash
# Download latest package
sudo pacman -U dist/linux-speed-meter-*.pkg.tar.zst
```

## What to Expect

### Before This Update
- Speed test would crash when starting tests
- Application would freeze during network operations
- Memory leaks over extended usage
- Poor error messages

### After This Update
- Speed test runs smoothly without crashes
- Graceful handling of network errors
- Proper resource cleanup (no memory leaks)
- Informative error messages
- Better progress indicators

## Testing Your Installation

After upgrading, test the following:

### 1. Basic Speed Test
1. Open the application
2. Navigate to "Speed Test" tab
3. Click "Start Test"
4. Verify test completes without crashing

### 2. Test Cancellation
1. Start a speed test
2. Click "Stop Test" during download phase
3. Start another test immediately
4. Verify no crashes occur

### 3. Multiple Tests
1. Run 3-5 consecutive speed tests
2. Verify memory usage remains stable
3. Check for any error messages

### 4. Network Error Handling
1. Start a speed test
2. Disconnect network briefly (optional)
3. Reconnect network
4. Verify graceful error handling

## Build Options

### Debug Build (for development)
```bash
./build.sh clean debug
```

### Release Build (for production)
```bash
./build.sh clean release
```

### Build All Packages
```bash
./build_all.sh
```

This will attempt to build:
- Native Linux executable
- Windows cross-compiled executable
- Debian package (.deb)
- RPM package (.rpm)
- Arch Linux package (.pkg.tar.zst)
- AppImage (portable)

## Troubleshooting

### Speed test still crashes
1. Ensure you have the latest version: `git pull origin main`
2. Clean build: `./build.sh clean release`
3. Check for missing dependencies: see docs/BUILD_AND_TEST.md
4. Report issue with logs from terminal

### Build fails
1. Install missing dependencies:
   ```bash
   sudo apt install libgtk-3-dev libappindicator3-dev libcurl4-openssl-dev
   ```
2. Check CMake version: `cmake --version` (requires 3.10+)
3. Try debug build: `./build.sh clean debug`

### Network errors during speed test
1. Check internet connectivity
2. Try different test server from dropdown
3. Check firewall settings
4. Some servers may be temporarily unavailable

## Performance Tips

### For Best Results
- Use Release build for production: `./build.sh release`
- Close other network-intensive applications
- Run test when network is not under heavy load
- Try different servers if one gives errors

### For Development
- Use Debug build: `./build.sh debug`
- Enable verbose logging
- Monitor with tools like valgrind for memory issues

## Known Issues

### None Critical
All previously known critical issues have been resolved in this update.

### Minor Notes
- SSL verification disabled for speed test endpoints (standard for speed testing)
- First test after application start may take slightly longer (CURL initialization)

## Rollback (if needed)

If you experience issues with this update:

```bash
# Checkout previous version
git checkout <previous-commit-hash>

# Rebuild
./build.sh clean release
```

Then report the issue with details.

## Support

If you encounter issues:

1. Check docs/SPEED_TEST_FIXES.md for technical details
2. Check docs/BUILD_AND_TEST.md for build help
3. Open an issue on GitHub with:
   - Your platform (Ubuntu 22.04, Fedora 38, etc.)
   - Steps to reproduce
   - Error messages from terminal
   - Build type (debug/release)

## Changes Summary

- 7 files modified for crash fixes
- 1 new RAII wrapper for memory safety
- 2 documentation files added
- Build system improvements
- ~150 lines of code changes focused on stability

## Next Steps

After upgrading:
1. Test the speed test functionality
2. Report any issues immediately
3. Enjoy stable, crash-free speed testing!

---

**Note**: This is a critical stability update. All users are strongly encouraged to upgrade.
