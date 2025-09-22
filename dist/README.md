# Linux Speed Meter - Distribution Packages

This directory contains pre-built packages for various Linux distributions.

## Current Package Status

✅ **Available**: Debian/Ubuntu (.deb), RPM (.rpm), Portable Bundle (.tar.gz + .run)
⚠️ **Build Instructions**: Arch Linux (README-Arch.md) - requires native Arch system
❌ **Requires FUSE**: AppImage - needs FUSE support in build environment

## Package Files

All packages are stored directly in this directory:

- `linux-speed-meter_*.deb` - Debian/Ubuntu packages ✅
- `linux-speed-meter-*.rpm` - Fedora/RHEL packages ✅
- `linux-speed-meter-*.pkg.tar.zst` - Arch Linux packages (build on Arch system)
- `linux-speed-meter-*.AppImage` - Universal AppImage packages (requires FUSE)
- `linux-speed-meter-*.tar.gz` + `linux-speed-meter-*.run` - Portable bundle ✅
- `README-Arch.md` - Instructions for building Arch packages

## Installation Instructions

### Debian/Ubuntu

```bash
sudo dpkg -i linux-speed-meter_*.deb
```

### Fedora/RHEL

```bash
sudo rpm -i linux-speed-meter-*.rpm
```

### Arch Linux

```bash
sudo pacman -U linux-speed-meter-*.pkg.tar.zst
```

### AppImage (Universal)

```bash
chmod +x linux-speed-meter-*.AppImage
./linux-speed-meter-*.AppImage
```

### Portable Bundle (FUSE Alternative)

```bash
chmod +x linux-speed-meter-*.run
./linux-speed-meter-*.run
```

Or extract and run manually:

```bash
tar -xzf linux-speed-meter-*.tar.gz
cd AppDir
./AppRun
```

## Building Packages

To build all packages from source, run:

```bash
./build_all.sh
```

### Build Requirements by Distribution

- **Debian/Ubuntu**: `dpkg-deb` (usually pre-installed) ✅
- **RPM**: `rpmbuild` (installed) + sudo access ✅
- **Arch Linux**: `makepkg` (installed) but requires functional pacman DB
- **AppImage**: FUSE support - requires FUSE kernel module and `fusermount`
- **Portable Bundle**: No special requirements (fallback when FUSE unavailable) ✅

### Build Environment Notes

- AppImage cannot be built in containerized environments without FUSE support
- RPM and Arch packages require their respective package management tools
- Debian packages can be built in most Linux environments
