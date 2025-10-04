# Changelog

All notable changes to Linux Speed Meter will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [2.0.0] - 2024-01-XX (In Development)

### Added
- **Windows GUI Application**: Full Qt-based GUI for Windows with system tray
- **Windows Taskbar Integration**: Real-time speed display in Windows taskbar
- **Advanced Charts**: Real-time bandwidth usage charts with historical data
- **Speed Test Feature**: Built-in network speed testing functionality
- **Data Export**: Export usage statistics to CSV/JSON formats
- **Notification System**: Alerts for data limits and connection issues
- **Dark Mode**: Theme support for better user experience
- **Interface Selection**: Choose specific network interface to monitor
- **Customizable Settings**: Adjustable refresh rates and display options
- **Historical Data View**: Browse and analyze past usage patterns
- **Bandwidth Warnings**: Configurable alerts for speed thresholds

### Enhanced
- **Linux GUI**: Improved GTK dashboard with modern design
- **System Tray**: Enhanced tooltip with more detailed information
- **Data Persistence**: Better storage and retrieval of statistics
- **Performance**: Optimized monitoring for lower resource usage

### Changed
- Version numbering to 2.0.0 for major feature release
- Centralized version management through VERSION file
- Updated documentation with comprehensive feature guides

### Fixed
- Missing systemtray.cpp implementation for Qt version
- Memory leaks in monitoring loops
- UI responsiveness issues

## [1.0.0] - 2024-01-XX

### Added
- Cross-compilation support for Windows builds from Linux
- Enhanced dashboard with monthly statistics and data limits
- Data persistence across sessions
- Improved GTK dashboard with monthly stats
- Windows console version with native speed monitoring
- Comprehensive build system for multiple package formats
- Development automation scripts

### Features
- Real-time network speed monitoring
- System tray integration
- GTK dashboard with basic statistics
- Auto-startup configuration
- Multi-distribution packaging support (DEB, RPM, Arch, AppImage)

### Initial Release
- Basic speed monitoring for Linux
- System tray application
- GTK-based dashboard
- Session statistics tracking
