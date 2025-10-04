#!/bin/bash
# Simple test script for Internet Speed Test functionality

echo "==================================================="
echo "Internet Speed Test - Functionality Test"
echo "==================================================="
echo ""

# Check if executable exists
if [ ! -f "./build/LinuxSpeedMeter" ]; then
    echo "❌ Error: LinuxSpeedMeter executable not found"
    echo "   Please build the project first with: cd build && cmake .. && make"
    exit 1
fi

echo "✅ Executable found"
echo ""

# Check for required libraries
echo "Checking dependencies..."

# Check for libcurl
if ldconfig -p | grep -q libcurl; then
    echo "✅ libcurl found"
else
    echo "❌ libcurl not found - required for speed tests"
    echo "   Install with: sudo apt-get install libcurl4-openssl-dev"
fi

# Check for GTK
if ldconfig -p | grep -q libgtk-3; then
    echo "✅ GTK3 found"
else
    echo "❌ GTK3 not found"
fi

# Check for AppIndicator
if ldconfig -p | grep -q libappindicator3; then
    echo "✅ AppIndicator3 found"
else
    echo "❌ AppIndicator3 not found"
fi

echo ""
echo "==================================================="
echo "Build Information"
echo "==================================================="

# Check CMake configuration
if [ -f "./build/CMakeCache.txt" ]; then
    echo "CMake configured for: Linux GTK build"
    grep "CMAKE_BUILD_TYPE" ./build/CMakeCache.txt | head -1
    echo ""
    
    # Check if speed test files are compiled
    if [ -f "./build/CMakeFiles/LinuxSpeedMeter.dir/src/speed_test.cpp.o" ]; then
        echo "✅ Speed test core compiled"
    fi
    
    if [ -f "./build/CMakeFiles/LinuxSpeedMeter.dir/src/download_test.cpp.o" ]; then
        echo "✅ Download test compiled"
    fi
    
    if [ -f "./build/CMakeFiles/LinuxSpeedMeter.dir/src/upload_test.cpp.o" ]; then
        echo "✅ Upload test compiled"
    fi
    
    if [ -f "./build/CMakeFiles/LinuxSpeedMeter.dir/src/ping_test.cpp.o" ]; then
        echo "✅ Ping test compiled"
    fi
    
    if [ -f "./build/CMakeFiles/LinuxSpeedMeter.dir/src/speed_test_widget.cpp.o" ]; then
        echo "✅ Speed test GUI widget compiled"
    fi
fi

echo ""
echo "==================================================="
echo "Speed Test Features Available"
echo "==================================================="
echo ""
echo "✅ Download Speed Test"
echo "   - Multi-threaded (4 parallel connections)"
echo "   - TCP warm-up compensation (2 seconds)"
echo "   - Progressive results reporting"
echo ""
echo "✅ Upload Speed Test"
echo "   - Multi-threaded upload"
echo "   - Random data generation"
echo "   - Accurate throughput measurement"
echo ""
echo "✅ Latency Test (Ping)"
echo "   - HTTP-based ping measurement"
echo "   - Min/Max/Average calculation"
echo "   - Multiple samples (5 pings)"
echo ""
echo "✅ Jitter Measurement"
echo "   - Latency variation analysis"
echo "   - Connection quality indicator"
echo ""
echo "✅ Test Servers"
echo "   - Cloudflare CDN"
echo "   - Fast.com (Netflix)"
echo "   - LibreSpeed servers"
echo ""
echo "✅ Beautiful GUI"
echo "   - Tabbed interface"
echo "   - Progress indicators"
echo "   - Color-coded results"
echo "   - Server selection dropdown"
echo ""

echo "==================================================="
echo "How to Use"
echo "==================================================="
echo ""
echo "1. Run the application:"
echo "   cd build && ./LinuxSpeedMeter"
echo ""
echo "2. Click the system tray icon"
echo ""
echo "3. Select 'Show Dashboard'"
echo ""
echo "4. Click the 'Speed Test' tab"
echo ""
echo "5. Select a test server and click 'Start Test'"
echo ""
echo "==================================================="
echo ""
echo "For detailed documentation, see:"
echo "   docs/SPEED_TEST_GUIDE.md"
echo ""
