#!/bin/bash

# Build script for RPM package

set -e

echo "Building Linux Speed Meter RPM package..."

# Create source tarball
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
cd "$PROJECT_DIR"
PROJECT_NAME="linux-speed-meter"
VERSION="1.0.0"

# Create properly structured tarball
rm -f "${PROJECT_NAME}-${VERSION}.tar.gz"
TEMP_DIR=$(mktemp -d)
cp -r . "$TEMP_DIR/${PROJECT_NAME}-${VERSION}"
cd "$TEMP_DIR"
rm -rf "${PROJECT_NAME}-${VERSION}/build" "${PROJECT_NAME}-${VERSION}/packaging" "${PROJECT_NAME}-${VERSION}/.git"
tar -czf "${PROJECT_NAME}-${VERSION}.tar.gz" "${PROJECT_NAME}-${VERSION}"
mv "${PROJECT_NAME}-${VERSION}.tar.gz" "$PROJECT_DIR/"
cd "$PROJECT_DIR"
rm -rf "$TEMP_DIR"

# Try to build RPM if rpmbuild is available and we have permissions
if command -v rpmbuild &> /dev/null; then
    echo "Attempting RPM build..."
    # Create RPM directories
    mkdir -p packaging/fedora/{BUILD,RPMS,SOURCES,SPECS,SRPMS}
    cp "${PROJECT_NAME}-${VERSION}.tar.gz" packaging/fedora/SOURCES/

    # Try building with sudo
    if sudo rpmbuild --define "_topdir $(pwd)/packaging/fedora" --nodeps -ba packaging/fedora/linux-speed-meter.spec 2>/dev/null; then
        mkdir -p dist
        find packaging/fedora/RPMS -name "*.rpm" -exec cp {} dist/ \;
        echo "RPM package created in dist/"
        exit 0
    else
        echo "⚠ RPM build failed - system requirements not met"
    fi
else
    echo "⚠ rpmbuild not available"
fi

# Fallback: Create instructions for manual RPM building
echo "Creating RPM build instructions..."
mkdir -p dist
cat > dist/README-RPM.md << 'EOF'
# Building RPM Package

Since rpmbuild is not available in the current environment, you can build the RPM package on a Fedora/RHEL/CentOS system:

## Prerequisites
- rpm-build package: `sudo dnf install rpm-build` (Fedora) or `sudo yum install rpm-build` (RHEL/CentOS)
- Development tools: `sudo dnf groupinstall "Development Tools"`
- GTK3 development: `sudo dnf install gtk3-devel libappindicator-gtk3-devel`

## Build Instructions
1. Copy the source tarball and spec file to your system
2. Run: `rpmbuild -ba linux-speed-meter.spec`
3. Find the built RPM in ~/rpmbuild/RPMS/x86_64/

## Manual Installation
Once built, install with: `sudo rpm -i linux-speed-meter-*.rpm`
EOF

echo "RPM build instructions created in dist/README-RPM.md"