# User Guide - Linux Speed Meter v2.0

## Table of Contents
1. [Getting Started](#getting-started)
2. [Main Features](#main-features)
3. [Settings Configuration](#settings-configuration)
4. [Data Export](#data-export)
5. [Keyboard Shortcuts](#keyboard-shortcuts)
6. [Troubleshooting](#troubleshooting)

## Getting Started

### First Launch

When you first launch Linux Speed Meter:
1. The application will start minimized to system tray
2. Look for the network icon in your system tray (Windows taskbar or Linux notification area)
3. Click the tray icon to open the dashboard
4. Right-click for quick menu options

### System Tray Icon

The system tray icon shows:
- **Tooltip**: Current download and upload speeds (hover over icon)
- **Click**: Opens the main dashboard window
- **Right-click**: Shows context menu with options

## Main Features

### Dashboard Tabs

#### 1. Overview Tab
- **Current Speed Section**: Real-time download/upload speeds with color coding
  - Green for download
  - Orange for upload
- **Session Information**: 
  - Session duration
  - Total data transferred
  - Average speeds
- **Network Interface**: 
  - Active network adapter
  - IP address
  - Connection status

#### 2. Statistics Tab
- **Peak Speeds**: Maximum download/upload speeds recorded
- **Total Data**: Cumulative data transferred
- **Progress Bars**: Visual representation of current vs peak speeds

#### 3. Charts Tab
- **Real-time Speed Graph**: Live visualization of network speeds
- **Historical Data**: View speed trends over time
- **Data Usage Chart**: Total data consumption tracking

#### 4. Settings Tab
- **Auto-start**: Start application on system boot
- **Stay on Top**: Keep window above other applications
- **Notifications**: Enable/disable system notifications
- **Refresh Rate**: Adjust update frequency (1-60 seconds)
- **Theme**: Switch between Light and Dark modes
- **Reset Statistics**: Clear all session data
- **Export Data**: Save usage history to CSV or JSON

## Settings Configuration

### Refresh Rate

Adjust how often the application updates:
1. Go to Settings tab
2. Change "Refresh Rate" value (1-60 seconds)
3. Lower values = more frequent updates (higher CPU usage)
4. Higher values = less frequent updates (lower CPU usage)
5. Default: 1 second (recommended for most users)

### Theme Selection

Change the appearance:
1. Go to Settings tab
2. Select Theme dropdown
3. Choose:
   - **Light**: Traditional bright theme
   - **Dark**: Modern dark theme (easier on eyes)

### Notifications

Control system notifications:
1. Go to Settings tab
2. Check/uncheck "Enable notifications"
3. Notifications will show:
   - Connection status changes
   - Data limit warnings (if configured)
   - Important system messages

## Data Export

### Export to CSV

1. Go to Settings tab
2. Click "Export to CSV" button
3. Choose save location and filename
4. CSV file will contain:
   - Timestamp
   - Download speed (MB/s)
   - Upload speed (MB/s)
   - Total download (MB)
   - Total upload (MB)

**Use cases for CSV:**
- Import into Excel/Google Sheets
- Create custom charts and graphs
- Analyze usage patterns
- Share data with others

### Export to JSON

1. Go to Settings tab
2. Click "Export to JSON" button
3. Choose save location and filename
4. JSON file will contain:
   - Export metadata (date, record count)
   - Structured data array
   - All speed and usage information

**Use cases for JSON:**
- Integration with other applications
- Web-based data visualization
- Programmatic data analysis
- API consumption

### Data Retention

- The application keeps the **last 1000 data points** in memory
- Data is recorded at the refresh rate interval
- At 1-second intervals: ~16 minutes of history
- At 60-second intervals: ~16 hours of history
- Export data before closing if you need long-term history

## Windows-Specific Features

### Taskbar Integration (Windows Only)

On Windows, the application shows current speeds in the window title:
- Format: `Speed Meter - ↓ [download] ↑ [upload]`
- Visible when hovering over taskbar button
- Updates in real-time

### Auto-Start (Windows)

The application automatically adds itself to Windows startup registry:
- Key: `HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Run`
- Entry: `LinuxSpeedMeter`
- To disable: Uncheck "Start automatically on system boot" in Settings

## Linux-Specific Features

### Auto-Start (Linux)

The application uses multiple methods for auto-start:
1. **XDG Autostart**: Desktop environment integration
2. **Systemd User Service**: Modern Linux systems
3. **Cron Job**: Fallback for older systems

### GTK Dashboard

Linux version uses GTK3 for native look and feel:
- Integrates with system theme
- Follows desktop environment styling
- Native file dialogs for export

## Keyboard Shortcuts

Currently, the application supports mouse/touch interaction only. Keyboard shortcuts may be added in future versions.

## Troubleshooting

### Application Won't Start

1. **Linux**: Ensure GTK3 and libappindicator3 are installed
   ```bash
   sudo apt install libgtk-3-0 libappindicator3-1
   ```

2. **Windows**: Ensure all Qt DLLs are in the same directory as the executable

### System Tray Icon Not Showing

1. **Linux**: Check if system tray/notification area is enabled in your desktop environment
   - GNOME: Install "AppIndicator Support" extension
   - KDE: Enable system tray in panel settings

2. **Windows**: Check if notification area icons are enabled in Windows settings

### Incorrect Speed Readings

1. Restart the application
2. Check that you're monitoring the correct network interface
3. Verify with another tool (task manager, iftop, nethogs)

### Export Not Working

1. **Linux**: Ensure you have write permissions to the target directory
2. **Windows**: Try exporting to Documents or Desktop folder
3. Check available disk space

### High CPU Usage

1. Increase refresh rate to 5-10 seconds in Settings
2. Close unnecessary browser tabs/applications
3. Restart the application

### Auto-Start Not Working

**Linux:**
```bash
# Check if autostart entry exists
ls ~/.config/autostart/linux-speed-meter.desktop

# Check systemd service
systemctl --user status linux-speed-meter.service
```

**Windows:**
```cmd
# Check registry entry
reg query "HKCU\Software\Microsoft\Windows\CurrentVersion\Run" /v LinuxSpeedMeter
```

## Tips and Best Practices

1. **Performance**: Set refresh rate to 2-5 seconds for optimal balance
2. **Data Analysis**: Export data weekly/monthly for long-term trend analysis
3. **Theme**: Use dark mode in low-light environments
4. **Notifications**: Disable if you find them distracting
5. **Export Format**: Use CSV for spreadsheets, JSON for programming

## Support

For issues, feature requests, or contributions:
- GitHub Issues: https://github.com/mojahid2021/linux-speed-meter/issues
- Pull Requests: https://github.com/mojahid2021/linux-speed-meter/pulls

## Version Information

This guide is for **Linux Speed Meter v2.0.0**

See [CHANGELOG.md](../CHANGELOG.md) for version history and updates.
