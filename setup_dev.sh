#!/bin/bash

# Linux Speed Meter Development Environment Setup Script
# This script sets up a complete development environment for the Linux Speed Meter project

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_header() {
    echo -e "${BLUE}================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}================================${NC}"
}

# Function to detect Linux distribution
detect_distro() {
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        DISTRO=$ID
        VERSION=$VERSION_ID
    elif [ -f /etc/debian_version ]; then
        DISTRO="debian"
    elif [ -f /etc/redhat-release ]; then
        DISTRO="rhel"
    else
        DISTRO="unknown"
    fi
    echo $DISTRO
}

# Function to install dependencies based on distribution
install_dependencies() {
    local distro=$1

    print_header "Installing Dependencies for $distro"

    case $distro in
        "ubuntu"|"debian"|"linuxmint"|"pop")
            print_status "Installing dependencies for Debian/Ubuntu-based systems..."
            sudo apt update
            sudo apt install -y \
                build-essential \
                cmake \
                pkg-config \
                libgtk-3-dev \
                libappindicator3-dev \
                git \
                vim \
                nano \
                htop \
                tree \
                wget \
                curl

            # Windows cross-compilation support
            print_status "Installing Windows cross-compilation tools..."
            sudo apt install -y \
                mingw-w64 \
                g++-mingw-w64-x86-64-win32 \
                binutils-mingw-w64
            ;;

        "fedora"|"rhel"|"centos"|"almalinux"|"rocky")
            print_status "Installing dependencies for Red Hat-based systems..."
            sudo dnf install -y \
                gcc-c++ \
                cmake \
                pkgconfig \
                gtk3-devel \
                libappindicator-gtk3-devel \
                git \
                vim \
                nano \
                htop \
                tree \
                wget \
                curl
            # For older RHEL/CentOS, might need EPEL
            if [ "$distro" = "rhel" ] || [ "$distro" = "centos" ]; then
                sudo dnf install -y epel-release
                sudo dnf install -y libappindicator-gtk3-devel
            fi
            ;;

        "arch"|"manjaro"|"endeavouros")
            print_status "Installing dependencies for Arch Linux-based systems..."
            sudo pacman -Syu --noconfirm
            sudo pacman -S --noconfirm \
                base-devel \
                cmake \
                pkgconf \
                gtk3 \
                libappindicator-gtk3 \
                git \
                vim \
                nano \
                htop \
                tree \
                wget \
                curl
            ;;

        "opensuse"|"sles")
            print_status "Installing dependencies for openSUSE-based systems..."
            sudo zypper refresh
            sudo zypper install -y \
                gcc-c++ \
                cmake \
                pkgconfig \
                gtk3-devel \
                libappindicator3-1 \
                git \
                vim \
                nano \
                htop \
                tree \
                wget \
                curl
            ;;

        *)
            print_error "Unsupported distribution: $distro"
            print_warning "Please install the following manually:"
            echo "  - C++ compiler (g++)"
            echo "  - CMake"
            echo "  - pkg-config"
            echo "  - GTK3 development libraries"
            echo "  - AppIndicator3 development libraries"
            echo "  - Git"
            return 1
            ;;
    esac
}

# Function to setup project
setup_project() {
    print_header "Setting up Linux Speed Meter Project"

    # Check if we're in the project directory
    if [ ! -f "CMakeLists.txt" ]; then
        print_error "CMakeLists.txt not found. Please run this script from the project root directory."
        return 1
    fi

    print_status "Creating build directory..."
    mkdir -p build

    print_status "Configuring project with CMake..."
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=Debug

    print_status "Building project..."
    make -j$(nproc)

    print_status "Build completed successfully!"
    cd ..
}

# Function to create development scripts
create_dev_scripts() {
    print_header "Creating Development Helper Scripts"

    # Create build script
    cat > build.sh << 'EOF'
#!/bin/bash
# Quick build script for development

set -e

echo "Building Linux Speed Meter..."

# Clean and rebuild
if [ "$1" = "clean" ]; then
    rm -rf build
fi

mkdir -p build
cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Build
make -j$(nproc)

echo "Build completed! Run ./build/LinuxSpeedMeter to test."
EOF

    chmod +x build.sh

    # Create run script
    cat > run.sh << 'EOF'
#!/bin/bash
# Quick run script for development

if [ -f "build/LinuxSpeedMeter" ]; then
    echo "Running Linux Speed Meter..."
    ./build/LinuxSpeedMeter
else
    echo "Binary not found. Run ./build.sh first."
    exit 1
fi
EOF

    chmod +x run.sh

    # Create clean script
    cat > clean.sh << 'EOF'
#!/bin/bash
# Clean build artifacts

echo "Cleaning build artifacts..."
rm -rf build
echo "Clean completed!"
EOF

    chmod +x clean.sh

    print_status "Development scripts created: build.sh, run.sh, clean.sh"
}

# Function to verify installation
verify_setup() {
    print_header "Verifying Development Environment"

    local errors=0

    # Check for required tools
    local tools=("g++" "cmake" "pkg-config" "git")
    for tool in "${tools[@]}"; do
        if command -v "$tool" >/dev/null 2>&1; then
            print_status "✓ $tool is installed"
        else
            print_error "✗ $tool is not installed"
            errors=$((errors + 1))
        fi
    done

    # Check for GTK3
    if pkg-config --exists gtk+-3.0; then
        print_status "✓ GTK3 development libraries found"
    else
        print_error "✗ GTK3 development libraries not found"
        errors=$((errors + 1))
    fi

    # Check for AppIndicator
    if pkg-config --exists appindicator3-0.1; then
        print_status "✓ AppIndicator3 development libraries found"
    else
        print_error "✗ AppIndicator3 development libraries not found"
        errors=$((errors + 1))
    fi

    # Check if project builds
    if [ -f "CMakeLists.txt" ]; then
        print_status "✓ Project files found"
        if [ -f "build/LinuxSpeedMeter" ]; then
            print_status "✓ Project is built and ready"
        else
            print_warning "! Project needs to be built (run ./build.sh)"
        fi
    else
        print_error "✗ Not in project directory"
        errors=$((errors + 1))
    fi

    if [ $errors -eq 0 ]; then
        print_status "🎉 Development environment is ready!"
        echo ""
        echo "Quick start commands:"
        echo "  ./build.sh     - Build the project"
        echo "  ./run.sh       - Run the application"
        echo "  ./clean.sh     - Clean build artifacts"
        echo "  ./build_all.sh - Build packages for all distributions"
    else
        print_error "Some issues were found. Please resolve them before proceeding."
        return 1
    fi
}

# Main script
main() {
    print_header "Linux Speed Meter Development Environment Setup"

    # Detect distribution
    DISTRO=$(detect_distro)
    print_status "Detected Linux distribution: $DISTRO"

    # Install dependencies
    if install_dependencies "$DISTRO"; then
        print_status "Dependencies installed successfully"
    else
        print_error "Failed to install dependencies"
        exit 1
    fi

    # Setup project
    if setup_project; then
        print_status "Project setup completed"
    else
        print_error "Failed to setup project"
        exit 1
    fi

    # Create development scripts
    create_dev_scripts

    # Verify everything
    verify_setup

    print_header "Setup Complete!"
    echo ""
    echo "Your Linux Speed Meter development environment is now ready!"
    echo "The project has been built and you can start developing immediately."
    echo ""
    echo "Useful commands:"
    echo "  ./build.sh     - Rebuild the project"
    echo "  ./run.sh       - Run the application"
    echo "  ./clean.sh     - Clean build files"
    echo "  ./build_all.sh - Build packages for distribution"
    echo ""
    echo "Happy coding! 🚀"
}

# Run main function
main "$@"