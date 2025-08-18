#!/bin/bash

# The King's Cab - macOS Installer Creator
# Creates a professional .pkg installer for VST3, AU, and AAX formats
# Compatible with Logic Pro, Pro Tools, and all major DAWs

set -e

# Resolve absolute paths
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

echo "🎛️  Creating The King's Cab macOS Installer..."
echo "=============================================="

# Configuration
PLUGIN_NAME="The Kings Cab"
COMPANY_NAME="King Studios"
VERSION="1.0.0"
BUNDLE_ID="com.kingstudios.thekingscab"
BUILD_DIR="${ROOT_DIR}/build"
INSTALLER_DIR="${SCRIPT_DIR}/mac"
PKG_DIR="${INSTALLER_DIR}/pkg_contents"
RESOURCES_DIR="${INSTALLER_DIR}/resources"

# Clean and create directories
rm -rf "${INSTALLER_DIR}"
mkdir -p "${PKG_DIR}/Applications/Audio/Plug-Ins"
mkdir -p "${PKG_DIR}/Library/Audio/Plug-Ins"
mkdir -p "${RESOURCES_DIR}"

echo "📦 Preparing plugin files..."

# Create plugin directories
mkdir -p "${PKG_DIR}/Library/Audio/Plug-Ins/VST3"
mkdir -p "${PKG_DIR}/Library/Audio/Plug-Ins/Components"
mkdir -p "${PKG_DIR}/Library/Application Support/Avid/Audio/Plug-Ins"

# Create IR files directory
mkdir -p "${PKG_DIR}/Users/Shared/King Studios/The Kings Cab/IR Collections"

# Ensure build exists for VST3/AU
if [ ! -d "${BUILD_DIR}/TheKingsCab_artefacts/Release/VST3/${PLUGIN_NAME}.vst3" ] || \
   [ ! -d "${BUILD_DIR}/TheKingsCab_artefacts/Release/AU/${PLUGIN_NAME}.component" ]; then
  echo "🔨 Building macOS VST3/AU (Release)..."
  cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
  cmake --build "${BUILD_DIR}" --config Release --target TheKingsCab_VST3 TheKingsCab_AU -j4
fi

# Copy VST3 plugin
if [ -d "${BUILD_DIR}/TheKingsCab_artefacts/Release/VST3/${PLUGIN_NAME}.vst3" ]; then
  echo "✅ Copying VST3 plugin..."
  cp -R "${BUILD_DIR}/TheKingsCab_artefacts/Release/VST3/${PLUGIN_NAME}.vst3" \
        "${PKG_DIR}/Library/Audio/Plug-Ins/VST3/"
else
  echo "❌ VST3 plugin still not found after build. Aborting."
  exit 1
fi

# Copy AU component
if [ -d "${BUILD_DIR}/TheKingsCab_artefacts/Release/AU/${PLUGIN_NAME}.component" ]; then
  echo "✅ Copying AU component..."
  cp -R "${BUILD_DIR}/TheKingsCab_artefacts/Release/AU/${PLUGIN_NAME}.component" \
        "${PKG_DIR}/Library/Audio/Plug-Ins/Components/"
else
  echo "❌ AU component still not found after build. Aborting."
  exit 1
fi

# Copy AAX plugin (if available)
if [ -d "${BUILD_DIR}/TheKingsCab_artefacts/Release/AAX/${PLUGIN_NAME}.aaxplugin" ]; then
    echo "✅ Copying AAX plugin..."
    cp -R "${BUILD_DIR}/TheKingsCab_artefacts/Release/AAX/${PLUGIN_NAME}.aaxplugin" \
          "${PKG_DIR}/Library/Application Support/Avid/Audio/Plug-Ins/"
else
    echo "⚠️  AAX plugin not found. Pro Tools support will be limited."
fi

# Copy IR Collections (prefer repo-staged IRs; fallback to Desktop source)
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
IR_SOURCE_REPO="${SCRIPT_DIR}/windows/ir_collections"
IR_SOURCE_DESKTOP="/Users/justinmitchell/Desktop/KINGS CAB"
if [ -d "${IR_SOURCE_REPO}" ]; then
    echo "🎸 Copying IR Collections from repo staging..."
    cp -R "${IR_SOURCE_REPO}/"* "${PKG_DIR}/Users/Shared/King Studios/The Kings Cab/IR Collections/"
    echo "✅ IR Collections copied from repo!"
elif [ -d "${IR_SOURCE_DESKTOP}" ]; then
    echo "🎸 Copying IR Collections from Desktop..."
    cp -R "${IR_SOURCE_DESKTOP}/"* "${PKG_DIR}/Users/Shared/King Studios/The Kings Cab/IR Collections/"
    echo "✅ IR Collections copied from Desktop!"
else
    echo "❌ IR Collections not found in repo or Desktop."
    echo "   Plugin will work but won't have IR files loaded."
fi

# Copy PNG assets to shared assets folder for DAWs/Standalone
mkdir -p "${PKG_DIR}/Users/Shared/King Studios/The Kings Cab/assets"
ASSETS_ROOT="${SCRIPT_DIR}/../assets"
for f in kkheader.png kkmain.png kkfooter.png custom_background.png; do
  if [ -f "${ASSETS_ROOT}/${f}" ]; then
    cp "${ASSETS_ROOT}/${f}" "${PKG_DIR}/Users/Shared/King Studios/The Kings Cab/assets/"
  fi
done

# Create preinstall script
cat > "${RESOURCES_DIR}/preinstall" << 'EOF'
#!/bin/bash
# Remove any existing installations
rm -rf "/Library/Audio/Plug-Ins/VST3/The Kings Cab.vst3"
rm -rf "/Library/Audio/Plug-Ins/Components/The Kings Cab.component"
rm -rf "/Library/Application Support/Avid/Audio/Plug-Ins/The Kings Cab.aaxplugin"
rm -rf "/Users/Shared/King Studios/The Kings Cab"
rm -rf "/Library/Application Support/King Studios/The Kings Cab"
exit 0
EOF

# Create postinstall script
cat > "${RESOURCES_DIR}/postinstall" << 'EOF'
#!/bin/bash
echo "The King's Cab installation completed successfully!"

# Set proper permissions for plugins
chmod -R 755 "/Library/Audio/Plug-Ins/VST3/The Kings Cab.vst3"
chmod -R 755 "/Library/Audio/Plug-Ins/Components/The Kings Cab.component"

if [ -d "/Library/Application Support/Avid/Audio/Plug-Ins/The Kings Cab.aaxplugin" ]; then
    chmod -R 755 "/Library/Application Support/Avid/Audio/Plug-Ins/The Kings Cab.aaxplugin"
fi

# Set proper permissions for IR Collections
if [ -d "/Users/Shared/King Studios/The Kings Cab/IR Collections" ]; then
    chmod -R 755 "/Users/Shared/King Studios/The Kings Cab"
    echo "✅ IR Collections installed and configured!"
fi

# Remove quarantine attributes so Logic sees the plugins without third-party prompts
xattr -dr com.apple.quarantine "/Library/Audio/Plug-Ins/VST3/The Kings Cab.vst3" || true
xattr -dr com.apple.quarantine "/Library/Audio/Plug-Ins/Components/The Kings Cab.component" || true
if [ -d "/Library/Application Support/Avid/Audio/Plug-Ins/The Kings Cab.aaxplugin" ]; then
  xattr -dr com.apple.quarantine "/Library/Application Support/Avid/Audio/Plug-Ins/The Kings Cab.aaxplugin" || true
fi

# Ad-hoc sign installed binaries to satisfy Gatekeeper on Intel and Apple Silicon
codesign -s - --deep -f "/Library/Audio/Plug-Ins/VST3/The Kings Cab.vst3" || true
codesign -s - --deep -f "/Library/Audio/Plug-Ins/Components/The Kings Cab.component" || true
if [ -d "/Library/Application Support/Avid/Audio/Plug-Ins/The Kings Cab.aaxplugin" ]; then
  codesign -s - --deep -f "/Library/Application Support/Avid/Audio/Plug-Ins/The Kings Cab.aaxplugin" || true
fi

# Kill any running DAWs to ensure plugin registration
killall "Logic Pro" 2>/dev/null || true
killall "Pro Tools" 2>/dev/null || true
killall "Reaper" 2>/dev/null || true
killall "Ableton Live" 2>/dev/null || true

echo "✅ The King's Cab is ready to use in your DAW!"
echo "🎸 Visit https://www.kingstudiospa.com/store for premium IR collections"

exit 0
EOF

chmod +x "${RESOURCES_DIR}/preinstall"
chmod +x "${RESOURCES_DIR}/postinstall"

# Create installer welcome text
cat > "${RESOURCES_DIR}/welcome.txt" << EOF
Welcome to The King's Cab Installer

This installer will install The King's Cab professional impulse response loader plugin.

Compatible with:
• Logic Pro (AU Component)
• Pro Tools (AAX Plugin)
• All major DAWs (VST3 Plugin)

Version: ${VERSION}
Company: ${COMPANY_NAME}

The King's Cab provides premium cabinet simulation with 6 IR slots for professional guitar and bass recording.
EOF

# Create README
cat > "${RESOURCES_DIR}/readme.txt" << EOF
The King's Cab - Professional Impulse Response Loader

INSTALLATION LOCATIONS:
• VST3: /Library/Audio/Plug-Ins/VST3/
• AU: /Library/Audio/Plug-Ins/Components/
• AAX: /Library/Application Support/Avid/Audio/Plug-Ins/

COMPATIBILITY:
• macOS 10.13 or later
• Intel and Apple Silicon Macs
• 64-bit DAWs only

AFTER INSTALLATION:
1. Restart your DAW
2. Look for "The King's Cab" in your plugin list
3. Load your premium IR collection from the designated folder

For support and premium IR collections:
https://www.kingstudiospa.com/store

© ${COMPANY_NAME} - All rights reserved
EOF

echo "🔨 Building installer package..."

# Build the package
pkgbuild \
    --root "${PKG_DIR}" \
    --identifier "${BUNDLE_ID}" \
    --version "${VERSION}" \
    --scripts "${RESOURCES_DIR}" \
    --install-location "/" \
    "${INSTALLER_DIR}/TheKingsCab-Core.pkg"

# Create distribution XML
cat > "${INSTALLER_DIR}/distribution.xml" << EOF
<?xml version="1.0" encoding="utf-8"?>
<installer-gui-script minSpecVersion="1">
    <title>The King's Cab ${VERSION}</title>
    <organization>${COMPANY_NAME}</organization>
    <domains enable_anywhere="false" enable_currentUserHome="false" enable_localSystem="true"/>
    <options customize="never" require-scripts="true" rootVolumeOnly="true" hostArchitectures="x86_64,arm64"/>
    
    <welcome file="welcome.txt"/>
    <readme file="readme.txt"/>
    
    <pkg-ref id="${BUNDLE_ID}"/>
    
    <choices-outline>
        <line choice="default">
            <line choice="${BUNDLE_ID}"/>
        </line>
    </choices-outline>
    
    <choice id="default"/>
    <choice id="${BUNDLE_ID}" visible="false">
        <pkg-ref id="${BUNDLE_ID}"/>
    </choice>
    
    <pkg-ref id="${BUNDLE_ID}" version="${VERSION}" onConclusion="none">TheKingsCab-Core.pkg</pkg-ref>
</installer-gui-script>
EOF

# Build the final installer
productbuild \
    --distribution "${INSTALLER_DIR}/distribution.xml" \
    --resources "${RESOURCES_DIR}" \
    --package-path "${INSTALLER_DIR}" \
    "${INSTALLER_DIR}/TheKingsCab-${VERSION}-macOS.pkg"

echo ""
echo "✅ macOS Installer created successfully!"
echo "📦 Location: ${INSTALLER_DIR}/TheKingsCab-${VERSION}-macOS.pkg"
echo ""
echo "🎛️  Installation includes:"
echo "   • VST3 plugin for all DAWs"
echo "   • AU component for Logic Pro"
echo "   • AAX plugin for Pro Tools (if built)"
echo ""
echo "🚀 Ready for distribution!"
