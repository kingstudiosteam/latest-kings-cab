#!/bin/bash

# The King's Cab - macOS Installer Creator (Intel-only)
# Produces a .pkg targeting Intel (x86_64) Macs (2015+), with preinstall cleanup,
# postinstall de-quarantine and ad-hoc signing. Includes VST3/AU/AAX (if built), IRs, and assets.

set -e

# Resolve absolute paths
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

echo "🎛️  Creating The King's Cab macOS Installer (Intel-only)..."
echo "=========================================================="

# Configuration
PLUGIN_NAME="The Kings Cab"
COMPANY_NAME="King Studios"
VERSION="1.2.0"
BUNDLE_ID="com.kingstudios.thekingscab"
BUILD_DIR="${ROOT_DIR}/build-intel"
INSTALLER_DIR="${SCRIPT_DIR}/mac-intel"
PKG_DIR="${INSTALLER_DIR}/pkg_contents"
RESOURCES_DIR="${INSTALLER_DIR}/resources"

# Clean and create directories
rm -rf "${INSTALLER_DIR}"
mkdir -p "${PKG_DIR}/Applications/Audio/Plug-Ins"
mkdir -p "${PKG_DIR}/Library/Audio/Plug-Ins"
mkdir -p "${RESOURCES_DIR}"

echo "📦 Preparing plugin files (x86_64)..."

# Create plugin directories
mkdir -p "${PKG_DIR}/Library/Audio/Plug-Ins/VST3"
mkdir -p "${PKG_DIR}/Library/Audio/Plug-Ins/Components"
mkdir -p "${PKG_DIR}/Library/Application Support/Avid/Audio/Plug-Ins"

# Create IR files directory
mkdir -p "${PKG_DIR}/Users/Shared/King Studios/The Kings Cab/IR Collections"

# Build Intel-only VST3/AU if missing
if [ ! -d "${BUILD_DIR}/TheKingsCab_artefacts/Release/VST3/${PLUGIN_NAME}.vst3" ] || \
   [ ! -d "${BUILD_DIR}/TheKingsCab_artefacts/Release/AU/${PLUGIN_NAME}.component" ]; then
  echo "🔨 Building macOS (Release, Intel x86_64 only)..."
  cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="x86_64" -DCMAKE_OSX_DEPLOYMENT_TARGET="10.13"
  cmake --build "${BUILD_DIR}" --config Release --target TheKingsCab_VST3 TheKingsCab_AU -j4
fi

# Copy VST3 plugin
if [ -d "${BUILD_DIR}/TheKingsCab_artefacts/Release/VST3/${PLUGIN_NAME}.vst3" ]; then
  echo "✅ Copying VST3 plugin..."
  cp -R "${BUILD_DIR}/TheKingsCab_artefacts/Release/VST3/${PLUGIN_NAME}.vst3" \
        "${PKG_DIR}/Library/Audio/Plug-Ins/VST3/"
else
  echo "❌ VST3 plugin not found after build. Aborting."; exit 1
fi

# Copy AU component
if [ -d "${BUILD_DIR}/TheKingsCab_artefacts/Release/AU/${PLUGIN_NAME}.component" ]; then
  echo "✅ Copying AU component..."
  cp -R "${BUILD_DIR}/TheKingsCab_artefacts/Release/AU/${PLUGIN_NAME}.component" \
        "${PKG_DIR}/Library/Audio/Plug-Ins/Components/"
else
  echo "❌ AU component not found after build. Aborting."; exit 1
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

# Create preinstall script (remove previous installs)
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

# Create postinstall script (permissions, de-quarantine, ad-hoc sign)
cat > "${RESOURCES_DIR}/postinstall" << 'EOF'
#!/bin/bash
echo "The King's Cab installation (Intel) completed successfully!"

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

# Remove quarantine so Logic sees the plugins without third-party prompts
xattr -dr com.apple.quarantine "/Library/Audio/Plug-Ins/VST3/The Kings Cab.vst3" || true
xattr -dr com.apple.quarantine "/Library/Audio/Plug-Ins/Components/The Kings Cab.component" || true
if [ -d "/Library/Application Support/Avid/Audio/Plug-Ins/The Kings Cab.aaxplugin" ]; then
  xattr -dr com.apple.quarantine "/Library/Application Support/Avid/Audio/Plug-Ins/The Kings Cab.aaxplugin" || true
fi

# Ad-hoc sign installed binaries
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

echo "✅ The King's Cab (Intel) is ready to use in your DAW!"
exit 0
EOF

chmod +x "${RESOURCES_DIR}/preinstall"
chmod +x "${RESOURCES_DIR}/postinstall"

# Build the package
echo "🔨 Building Intel-only installer package..."
pkgbuild \
    --root "${PKG_DIR}" \
    --identifier "${BUNDLE_ID}.intel" \
    --version "${VERSION}" \
    --scripts "${RESOURCES_DIR}" \
    --install-location "/" \
    "${INSTALLER_DIR}/TheKingsCab-Core-Intel.pkg"

cat > "${INSTALLER_DIR}/distribution.xml" << EOF
<?xml version="1.0" encoding="utf-8"?>
<installer-gui-script minSpecVersion="1">
    <title>The King's Cab ${VERSION} (Intel)</title>
    <organization>${COMPANY_NAME}</organization>
    <domains enable_anywhere="false" enable_currentUserHome="false" enable_localSystem="true"/>
    <options customize="never" require-scripts="true" rootVolumeOnly="true" hostArchitectures="x86_64"/>
    <pkg-ref id="${BUNDLE_ID}.intel"/>
    <choices-outline>
        <line choice="default">
            <line choice="${BUNDLE_ID}.intel"/>
        </line>
    </choices-outline>
    <choice id="default"/>
    <choice id="${BUNDLE_ID}.intel" visible="false">
        <pkg-ref id="${BUNDLE_ID}.intel"/>
    </choice>
    <pkg-ref id="${BUNDLE_ID}.intel" version="${VERSION}" onConclusion="none">TheKingsCab-Core-Intel.pkg</pkg-ref>
    <readme file="${RESOURCES_DIR}/readme.txt"/>
</installer-gui-script>
EOF

# Minimal readme
cat > "${RESOURCES_DIR}/readme.txt" << EOF
The King's Cab (Intel build) - v${VERSION}
• Installs VST3, AU, and AAX (if present)
• Includes IR Collections and assets
• Intel x86_64 Macs (2015+) - macOS 10.13 or later
EOF

productbuild \
    --distribution "${INSTALLER_DIR}/distribution.xml" \
    --resources "${RESOURCES_DIR}" \
    --package-path "${INSTALLER_DIR}" \
    "${INSTALLER_DIR}/TheKingsCab-${VERSION}-macOS-Intel.pkg"

echo ""
echo "✅ Intel-only macOS Installer created successfully!"
echo "📦 Location: ${INSTALLER_DIR}/TheKingsCab-${VERSION}-macOS-Intel.pkg"
echo ""
echo "🚀 Ready for distribution!"
