# Features Documentation

## Complete Feature List - Linux Speed Meter v2.0

### Core Monitoring Features

#### Real-Time Speed Monitoring

- **Download Speed**: Continuous monitoring of incoming data rate
- **Upload Speed**: Continuous monitoring of outgoing data rate
- **Update Frequency**: Configurable from 1 to 60 seconds
- **Precision**: Displays speeds in B/s, KB/s, or MB/s automatically
- **Accuracy**: Direct interface polling for accurate measurements

#### Network Interface Detection

- **Automatic Detection**: Finds active network interface automatically
- **Multi-Interface Support**: Works with Ethernet, WiFi, VPN, and mobile hotspots
- **Interface Information Display**: Shows adapter name and IP address
- **Connection Status**: Real-time connected/disconnected status

### Dashboard Features

#### User Interface

- **Multi-Tab Layout**:
  - Overview: Current speeds and session info
  - Statistics: Detailed usage metrics
  - Charts: Visual data representation
  - Settings: Configuration options
- **Responsive Design**: Adapts to window resizing
- **Modern Styling**: Clean, professional appearance
- **Color-Coded Data**:
  - Green for download
  - Orange for upload
  - Status indicators with traffic light colors

#### Session Tracking

- **Session Duration**: Automatic time tracking from application start
- **Total Data Transfer**: Cumulative download and upload tracking
- **Average Speed Calculation**: Session-based average speeds
- **Peak Speed Recording**: Maximum speeds achieved in session
- **Session Reset**: Clear statistics and start fresh

#### Speed Test Enhancements

- **Advanced Controls**: Configure duration, warmup window, and parallel connections directly in the UI
- **Smart Server Selection**: Optional automatic latency scan picks the fastest available test server before each run
- **Live Progress Feedback**: Progress indicator now reflects accurate measurement windows even when timings are adjusted
- **Result History**: Recent test outcomes are stored in-session with timestamps, server names, and key metrics for quick comparison
- **Stability Guardrails**: Input validation prevents invalid timing combinations that previously caused crashes

### Data Management

#### Historical Data Tracking

- **Automatic Recording**: Records data at each refresh interval
- **Storage Capacity**: Keeps last 1000 data points in memory
- **Efficient Memory Usage**: Automatic cleanup of old data
- **Granular Data**: Timestamp, speeds, and totals for each record

#### Data Export

- **CSV Export**:
  - Standard comma-separated format
  - Compatible with Excel, Google Sheets
  - Headers included for easy import
  - Precision: 3 decimal places for speeds, 2 for totals
  
- **JSON Export**:
  - Structured data format
  - Includes export metadata
  - Suitable for programmatic access
  - Human-readable formatting

- **Export Metadata**:
  - Export date and time
  - Record count
  - Formatted timestamps
  - Standardized units (MB/s, MB)

### Customization Features

#### Visual Themes

- **Light Theme**:
  - Traditional bright interface
  - High contrast for daytime use
  - Default system colors
  
- **Dark Theme**:
  - Modern dark interface
  - Reduced eye strain
  - Custom color scheme
  - Suitable for low-light environments

#### Configurable Settings

- **Refresh Rate**:
  - Range: 1-60 seconds
  - Default: 1 second
  - Live update (no restart required)
  
- **Window Behavior**:
  - Stay on top option
  - Minimize to tray
  - Close to tray (doesn't exit)
  
- **Auto-Start**:
  - Start on system boot
  - Platform-specific implementation
  - Easy enable/disable toggle

#### Notifications

- **System Integration**: Native OS notifications
- **Notification Types**:
  - Connection status changes
  - Data limit warnings (when configured)
  - Export completion
  - Error messages
- **User Control**: Enable/disable in settings

### Platform-Specific Features

#### Windows Features

- **Qt-Based GUI**:
  - Native Windows look and feel
  - High-DPI support
  - Modern Windows integration
  
- **Taskbar Integration**:
  - Speed display in window title
  - Visible in taskbar preview
  - Real-time updates
  
- **Registry Auto-Start**:
  - Integrated with Windows startup
  - No additional tools required
  
- **System Tray**:
  - Windows notification area icon
  - Tooltip with current speeds
  - Context menu for quick actions

#### Linux Features

- **GTK3 Interface**:
  - Native GNOME/KDE integration
  - System theme compatibility
  - Lightweight and fast
  
- **AppIndicator Support**:
  - Works with modern desktop environments
  - Fallback for older systems
  - Status bar integration
  
- **Multiple Auto-Start Methods**:
  - XDG Autostart (all desktop environments)
  - Systemd user services
  - Cron job fallback
  
- **Distribution Packaging**:
  - DEB packages (Debian/Ubuntu)
  - RPM packages (Fedora/RHEL)
  - Arch packages (Arch Linux)
  - AppImage (universal)

### Charts and Visualization

#### Speed Chart

- **Real-Time Graphing**: Live speed visualization
- **Dual Line Series**: Separate lines for download and upload
- **Time Axis**: Shows historical progression
- **Speed Axis**: Automatically scales based on data
- **Color Coding**: Matches dashboard color scheme

#### Data Usage Display

- **Total Data Counter**: Session cumulative data
- **Visual Progress Bars**: Speed comparison indicators
- **Peak Speed Indicators**: Highlight maximum values

### Data Persistence

#### Monthly Statistics (GTK Version)

- **Monthly Tracking**: Tracks data usage per month
- **Peak Values**: Records monthly peak speeds
- **Active Days**: Counts days with network activity
- **Data Limits**: Configurable monthly caps
- **Usage Warnings**: Alerts when approaching limits

#### Persistent Storage

- **Automatic Saving**: Regular data persistence
- **Cross-Session**: Statistics survive application restarts
- **File-Based**: Simple text file storage
- **Platform-Specific Locations**:
  - Linux: `~/.config/linux-speed-meter/usage_data.txt`
  - Windows: `%APPDATA%\linux-speed-meter\usage_data.txt`

### Performance Features

#### Efficient Monitoring

- **Low CPU Usage**: Optimized polling algorithms
- **Minimal Memory**: Efficient data structures
- **Background Operation**: Runs without window open
- **Automatic Cleanup**: Removes old data automatically

#### Resource Management

- **Configurable Polling**: Adjust for performance vs accuracy
- **Smart Caching**: Reduces redundant calculations
- **Lazy Loading**: Loads charts only when viewed
- **Memory Limits**: Caps historical data storage

### User Experience Features

#### Ease of Use

- **One-Click Actions**: Export, reset, theme change
- **Clear Labels**: Intuitive interface text
- **Tooltips**: Helpful hints on controls
- **Visual Feedback**: Progress indicators and confirmations

#### Error Handling

- **Graceful Failures**: Informative error messages
- **Recovery Options**: Suggests solutions
- **Validation**: Prevents invalid configurations
- **Safe Defaults**: Sensible default values

#### Accessibility

- **High Contrast**: Readable text and indicators
- **Large Fonts**: Adjustable through system settings
- **Color-Blind Friendly**: Multiple visual indicators
- **Keyboard Navigation**: Qt version supports tab navigation

### Integration Features

#### System Integration

- **Startup Management**: Automatic startup configuration
- **Tray Icon**: System notification area integration
- **File Dialogs**: Native file choosers
- **Notifications**: Native OS notification system

#### Cross-Platform

- **Consistent Experience**: Similar features on all platforms
- **Platform Optimizations**: Uses best tools for each OS
- **Data Compatibility**: Cross-platform data file format
- **Build System**: CMake for easy compilation

### Future-Ready Features

#### Extensibility

- **Modular Design**: Easy to add new features
- **Plugin-Ready**: Architecture supports extensions
- **API-Friendly**: JSON export for external tools
- **Version Management**: Centralized version tracking

#### Maintainability

- **Clean Code**: Well-documented source
- **Version Control**: Git-based development
- **Issue Tracking**: GitHub integration
- **Changelog**: Detailed version history

## Feature Comparison Matrix

| Feature | Linux GTK | Windows Qt | Windows Console |
|---------|-----------|------------|-----------------|
| Real-time monitoring | ✅ | ✅ | ✅ |
| System tray | ✅ | ✅ | ❌ |
| Dashboard GUI | ✅ | ✅ | ❌ |
| Charts | ✅ | ✅ | ❌ |
| Export CSV | ✅ | ✅ | ❌ |
| Export JSON | ✅ | ✅ | ❌ |
| Dark theme | ❌ | ✅ | ❌ |
| Customizable refresh | ❌ | ✅ | ❌ |
| Notifications | ✅ | ✅ | ❌ |
| Auto-start | ✅ | ✅ | ❌ |
| Monthly stats | ✅ | ✅ | ❌ |
| Data persistence | ✅ | ✅ | ✅ |

## Configuration Options

### Settings File Locations

**Linux:**

- User data: `~/.config/linux-speed-meter/`
- Autostart: `~/.config/autostart/linux-speed-meter.desktop`
- Systemd: `~/.config/systemd/user/linux-speed-meter.service`

**Windows:**

- User data: `%APPDATA%\linux-speed-meter\`
- Registry: `HKCU\Software\Microsoft\Windows\CurrentVersion\Run`
- Settings: Application-managed (Qt settings)

### Configurable Parameters

1. **Refresh Rate**: 1-60 seconds
2. **Theme**: Light or Dark
3. **Auto-Start**: On/Off
4. **Stay on Top**: On/Off
5. **Notifications**: On/Off
6. **Data Limits**: Custom MB values (GTK version)

## Technical Specifications

### Supported Platforms

- **Linux**: Ubuntu 18.04+, Fedora 30+, Arch Linux, Debian 10+
- **Windows**: Windows 10/11 (64-bit)
- **Desktop Environments**: GNOME, KDE, XFCE, MATE, Cinnamon, Windows 10/11

### Requirements

- **Linux**: GTK 3.22+, libappindicator3
- **Windows**: Qt 5.15+ or Qt 6.x
- **RAM**: 50-100 MB
- **CPU**: Minimal (< 1% on modern systems)
- **Disk**: < 20 MB installed

### Build Dependencies

- **C++ Compiler**: GCC 7+ or MSVC 2019+
- **CMake**: 3.10+ (Linux), 3.16+ (Windows Qt)
- **Qt**: 5.15+ or 6.x (Windows version)
- **GTK**: 3.22+ (Linux version)

## Version Information

This documentation is for **Linux Speed Meter v2.0.0**

See [CHANGELOG.md](../CHANGELOG.md) for detailed version history.
See [USER_GUIDE.md](USER_GUIDE.md) for usage instructions.
