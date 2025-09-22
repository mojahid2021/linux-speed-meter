# Building Arch Linux Package

Since makepkg is not available in the current environment, you can build the Arch Linux package on an Arch Linux system:

## Prerequisites
- pacman package manager (comes with Arch Linux)
- Development tools: `sudo pacman -S base-devel`
- GTK3 development: `sudo pacman -S gtk3 libappindicator-gtk3`

## Build Instructions
1. Copy the PKGBUILD file and source tarball to a directory
2. Run: `makepkg -si` (build and install) or `makepkg` (build only)
3. Find the built package in the current directory

## Manual Installation
Once built, install with: `sudo pacman -U linux-speed-meter-*.pkg.tar.zst`
