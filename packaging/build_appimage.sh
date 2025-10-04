#!/bin/bash

# Build AppImage for Linux Speed Meter

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

cd "$PROJECT_ROOT"

echo "Building Linux Speed Meter AppImage..."

# Create build directory
rm -rf build
mkdir -p build
cd build

# Configure and build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Create AppImage directory structure
mkdir -p ../packaging/appimage/AppDir/usr/bin
mkdir -p ../packaging/appimage/AppDir/usr/share/applications
mkdir -p ../packaging/appimage/AppDir/usr/share/icons/hicolor/256x256/apps

# Copy binary
cp LinuxSpeedMeter ../packaging/appimage/AppDir/usr/bin/linux-speed-meter

# Copy desktop file
cp ../packaging/debian/usr/share/applications/linux-speed-meter.desktop ../packaging/appimage/AppDir/usr/share/applications/

# Create icon (placeholder for now)
if command -v convert; then
    convert -size 256x256 xc:blue ../packaging/appimage/AppDir/usr/share/icons/hicolor/256x256/apps/linux-speed-meter.png
else
    echo "Warning: ImageMagick not available, creating placeholder icon"
    echo "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNkYPhfDwAChwGA60e6kgAAAABJRU5ErkJggg==" | base64 -d > ../packaging/appimage/AppDir/usr/share/icons/hicolor/256x256/apps/linux-speed-meter.png
fi

cd ../packaging/appimage

# Download linuxdeploy if not present
if [ ! -f linuxdeploy-x86_64.AppImage ]; then
    echo "Downloading linuxdeploy..."
    wget -q https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
    chmod +x linuxdeploy-x86_64.AppImage
fi

# Try to build AppImage with linuxdeploy
VERSION=$(cat ../../VERSION)
if ./linuxdeploy-x86_64.AppImage --appimage-extract-and-run --appdir AppDir --output appimage; then
    # Rename and move to dist
    mv Linux_Speed_Meter-x86_64.AppImage ../../dist/linux-speed-meter-${VERSION}-x86_64.AppImage
    echo "AppImage created: dist/linux-speed-meter-${VERSION}-x86_64.AppImage"
else
    echo "linuxdeploy failed, trying manual AppImage creation..."
    
    # Create portable tar.gz bundle as fallback
    cd AppDir
    tar -czf ../../../dist/linux-speed-meter-${VERSION}-x86_64.tar.gz .
    cd ..
    
    # Create simple .run script
    cat > ../../dist/linux-speed-meter-${VERSION}-x86_64.run << 'INNER_EOF'
#!/bin/bash
# Simple self-extracting archive for Linux Speed Meter

echo "Extracting Linux Speed Meter..."

# Find the line number where the tar archive starts
ARCHIVE_START_LINE=$(grep -n "^__ARCHIVE_START__$" "$0" | cut -d: -f1)
ARCHIVE_START_LINE=$((ARCHIVE_START_LINE + 1))

# Extract the archive
tail -n +$ARCHIVE_START_LINE "$0" | tar -xz

echo "Installation complete. Run ./linux-speed-meter to start the application."
exit 0

__ARCHIVE_START__
INNER_EOF
    
    # Append the tar archive to the run script
    cat ../../dist/linux-speed-meter-${VERSION}-x86_64.tar.gz >> ../../dist/linux-speed-meter-${VERSION}-x86_64.run
    chmod +x ../../dist/linux-speed-meter-${VERSION}-x86_64.run
    
    echo "Portable bundle created: dist/linux-speed-meter-${VERSION}-x86_64.run"
fi
