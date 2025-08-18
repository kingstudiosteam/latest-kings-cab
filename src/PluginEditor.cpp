#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TheKingsCabAudioProcessorEditor::TheKingsCabAudioProcessorEditor(TheKingsCabAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setupComponents();
    setLookAndFeel(&kingsCabLookAndFeel);
    
    // Set fixed window size for luxury, professional appearance
    setSize(kWindowWidth, kWindowHeight);
    setResizable(false, false); // Disable resizing to prevent layout stretching
    
    // Initialize IR folder data
    initializeIRData();
    
    // Start timer for UI updates
    startTimerHz(30); // 30 FPS for smooth UI updates
}

TheKingsCabAudioProcessorEditor::~TheKingsCabAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void TheKingsCabAudioProcessorEditor::setupComponents()
{
        // Load custom background image if available
    loadCustomBackground();
    
    // Load header background image
    loadHeaderBackground();
    
    // Load main body background image
    loadMainBodyBackground();

    // Master controls setup
    masterGainSlider = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag,
                                                      juce::Slider::NoTextBox);
    masterGainSlider->setRange(0.0, 4.0, 0.001);
    masterGainSlider->setSkewFactorFromMidPoint(1.0); // finer control near 1.0
    masterGainSlider->setVelocityModeParameters(0.8, 1, 0.05, true);
    masterGainSlider->setValue(1.0);
    masterGainSlider->addListener(this);
    addAndMakeVisible(*masterGainSlider);

    masterMixSlider = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag,
                                                     juce::Slider::NoTextBox);
    masterMixSlider->setRange(0.0, 1.0, 0.01);
    masterMixSlider->setValue(1.0);
    masterMixSlider->addListener(this);
    addAndMakeVisible(*masterMixSlider);

    masterGainLabel = std::make_unique<juce::Label>("GainLabel", "MASTER VOLUME");
    masterGainLabel->setFont(juce::Font(11.0f).boldened());
    masterGainLabel->setJustificationType(juce::Justification::centred);
    masterGainLabel->setColour(juce::Label::textColourId, 
                              kingsCabLookAndFeel.findColour(KingsCabLookAndFeel::primaryTextColourId));
    addAndMakeVisible(*masterGainLabel);

    masterMixLabel = std::make_unique<juce::Label>("MixLabel", "DRY/IR");
    masterMixLabel->setFont(juce::Font(11.0f).boldened());
    masterMixLabel->setJustificationType(juce::Justification::centred);
    masterMixLabel->setColour(juce::Label::textColourId, 
                             kingsCabLookAndFeel.findColour(KingsCabLookAndFeel::primaryTextColourId));
    addAndMakeVisible(*masterMixLabel);

    // Create parameter attachments for master controls
    masterGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "master_gain", *masterGainSlider);
    
    masterMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "master_mix", *masterMixSlider);

    // Setup IR slots
    setupIRSlots();

    // Folder browser removed - each IR slot has its own dropdowns

    // Footer components

    statusLabel = std::make_unique<juce::Label>("Status", "Ready");
    statusLabel->setFont(juce::Font(10.0f));
    statusLabel->setJustificationType(juce::Justification::centredRight);
    statusLabel->setColour(juce::Label::textColourId, 
                          kingsCabLookAndFeel.findColour(KingsCabLookAndFeel::secondaryTextColourId));
    addAndMakeVisible(*statusLabel);

    // King Studios store link - larger and more prominent
    storeLink = std::make_unique<juce::HyperlinkButton>("Visit King Studios Store", 
                                                        juce::URL("https://www.kingstudiospa.com/store"));
    storeLink->setFont(juce::Font(14.0f).boldened(), false);
    storeLink->setJustificationType(juce::Justification::centred);
    storeLink->setColour(juce::HyperlinkButton::textColourId, 
                        kingsCabLookAndFeel.findColour(KingsCabLookAndFeel::goldHighlightColourId));
    addAndMakeVisible(*storeLink);
}

void TheKingsCabAudioProcessorEditor::setupIRSlots()
{
    for (int i = 0; i < TheKingsCabAudioProcessor::kNumIRSlots; ++i)
    {
        irSlots[i] = std::make_unique<IRSlot>(i, audioProcessor.getValueTreeState());
        
        // Setup callbacks
        irSlots[i]->onIRSelected = [this](int slotIndex, const juce::File& irFile) {
            onIRSelected(slotIndex, irFile);
        };
        
        irSlots[i]->onIRCleared = [this](int slotIndex) {
            onIRCleared(slotIndex);
        };
        
        addAndMakeVisible(*irSlots[i]);
    }
}

//==============================================================================
void TheKingsCabAudioProcessorEditor::paint(juce::Graphics& g)
{
    drawBackground(g);
    
    auto bounds = getLocalBounds();
    
    // Draw sections
    drawHeader(g, bounds.removeFromTop(kHeaderHeight));
    drawFooter(g, bounds.removeFromBottom(kFooterHeight));
}

void TheKingsCabAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    
    // Header section (no logo - clean space)
    bounds.removeFromTop(kHeaderHeight); // Header space (no logo currently)
    
    // Footer section
    auto footerBounds = bounds.removeFromBottom(kFooterHeight);
    footerBounds.reduce(8, 4);
    
    // Status in small corner (minimal space)
    auto statusArea = footerBounds.removeFromRight(60);
    statusLabel->setBounds(statusArea);
    
    // Store link centered in remaining space
    storeLink->setBounds(footerBounds);
    
    // Main content area
    bounds.reduce(8, 8);
    
    // No folder browser needed - using individual slot dropdowns
    
    // Show master volume centered at the top; hide mix knob
    masterGainSlider->setVisible(true);
    masterGainLabel->setVisible(true);
    masterMixSlider->setVisible(false);
    masterMixLabel->setVisible(false);

    // Reserve area for master gain in the top of the main content
    auto masterArea = bounds.removeFromTop(90);
    {
        const int knobSize = 64;
        const int labelHeight = 18;
        const int spacing = 6;
        auto centerX = masterArea.getCentreX();
        // Place label above the knob
        juce::Rectangle<int> labelBounds(centerX - 120/2, masterArea.getY(), 120, labelHeight);
        masterGainLabel->setBounds(labelBounds);
        juce::Rectangle<int> knobBounds(centerX - knobSize / 2, labelBounds.getBottom() + spacing, knobSize, knobSize);
        masterGainSlider->setBounds(knobBounds);
    }
    
    // Compact IR slots in 2x3 grid - positioned relative to remaining bounds  
    auto slotsArea = bounds;
    
    // Calculate slot width - make them wide enough with a reasonable gap
    auto slotWidth = (slotsArea.getWidth() * 0.45); // Each slot takes 45% of width, leaving 10% gap in center
    auto slotHeight = (slotsArea.getHeight() - kSlotSpacing * 2) / 3; // Dynamic height for 500px body
    
    for (int row = 0; row < 3; ++row)
    {
        for (int col = 0; col < 2; ++col)
        {
            int slotIndex = row * 2 + col;
            if (slotIndex < TheKingsCabAudioProcessor::kNumIRSlots)
            {
                // Configure right alignment for right column slots
                irSlots[slotIndex]->setRightAligned(col == 1);
                
                // Perfect symmetry - left edge and right edge alignment
                int slotX;
                if (col == 0)
                {
                    // Left column: align to left edge (small margin from edge)
                    slotX = slotsArea.getX() + 10;
                }
                else
                {
                    // Right column: align to right edge (slot ends at right edge with small margin)
                    slotX = slotsArea.getRight() - slotWidth - 10;
                }
                
                auto slotBounds = juce::Rectangle<int>(
                    slotX,
                    slotsArea.getY() + row * (slotHeight + kSlotSpacing),
                    slotWidth,
                    slotHeight
                );
                

                irSlots[slotIndex]->setBounds(slotBounds);
            }
        }
    }
}

//==============================================================================
void TheKingsCabAudioProcessorEditor::timerCallback()
{
    updateStatusDisplay();
    
    // IR slots will get folder data directly from IRManager
}

void TheKingsCabAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    juce::ignoreUnused(slider);
    // Parameter attachments handle the updates automatically
}

//==============================================================================


void TheKingsCabAudioProcessorEditor::updateStatusDisplay()
{
    // Count loaded IRs
    int loadedCount = 0;
    for (const auto& slot : irSlots)
    {
        if (slot->isActive())
            loadedCount++;
    }
    
    juce::String status = juce::String(loadedCount) + "/" + 
                         juce::String(TheKingsCabAudioProcessor::kNumIRSlots) + " IRs loaded";
    statusLabel->setText(status, juce::dontSendNotification);
}

void TheKingsCabAudioProcessorEditor::loadCustomBackground()
{
    // Try to load custom background image (840x620 pixels)
    juce::File backgroundFile = juce::File::getCurrentWorkingDirectory()
                              .getChildFile("assets")
                              .getChildFile("custom_background.png");

   #if JUCE_WINDOWS
    if (!backgroundFile.existsAsFile())
    {
        backgroundFile = juce::File::getSpecialLocation(juce::File::commonApplicationDataDirectory)
                              .getChildFile("King Studios")
                              .getChildFile("The Kings Cab")
                              .getChildFile("assets")
                              .getChildFile("custom_background.png");
    }
   #endif
   #if JUCE_MAC
    if (!backgroundFile.existsAsFile())
    {
        backgroundFile = juce::File("/Users/Shared/King Studios/The Kings Cab/assets/custom_background.png");
    }
   #endif
    
        if (backgroundFile.existsAsFile())
    {
        customBackgroundImage = juce::ImageFileFormat::loadFrom(backgroundFile);
        if (customBackgroundImage.isValid())
        {
            DBG("Custom background loaded: " << backgroundFile.getFullPathName());
            DBG("Background size: " << customBackgroundImage.getWidth() << "x" << customBackgroundImage.getHeight());
        }
        else
        {
            DBG("Failed to load custom background image");
        }
    }
    else
    {
        DBG("No custom background found (looking for: custom_background.png)");
    }
}

void TheKingsCabAudioProcessorEditor::loadHeaderBackground()
{
    // Try multiple paths for header background image
    juce::File headerFile;
    
    // Try relative to current working directory
    headerFile = juce::File::getCurrentWorkingDirectory()
                    .getChildFile("assets")
                    .getChildFile("kkheader.png");

   #if JUCE_WINDOWS
    if (!headerFile.existsAsFile())
    {
        headerFile = juce::File::getSpecialLocation(juce::File::commonApplicationDataDirectory)
                        .getChildFile("King Studios")
                        .getChildFile("The Kings Cab")
                        .getChildFile("assets")
                        .getChildFile("kkheader.png");
    }
   #endif
   #if JUCE_MAC
    if (!headerFile.existsAsFile())
        headerFile = juce::File("/Users/Shared/King Studios/The Kings Cab/assets/kkheader.png");
   #endif
    
    // Try relative to executable
    if (!headerFile.existsAsFile())
    {
        headerFile = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                        .getParentDirectory()
                        .getChildFile("assets")
                        .getChildFile("kkheader.png");
    }
    
    if (headerFile.existsAsFile())
    {
        headerBackgroundImage = juce::ImageFileFormat::loadFrom(headerFile);
        if (headerBackgroundImage.isValid())
        {
            DBG("Header background loaded: " << headerFile.getFullPathName());
            DBG("Header size: " << headerBackgroundImage.getWidth() << "x" << headerBackgroundImage.getHeight());
        }
        else
        {
            DBG("Failed to load header background: " << headerFile.getFullPathName());
        }
    }
    else
    {
        DBG("No header background found (looking for: kkheader.png)");
    }
}

void TheKingsCabAudioProcessorEditor::loadMainBodyBackground()
{
    // Try multiple paths for main body background image
    juce::File mainBodyFile;
    
    // Try relative to current working directory
    mainBodyFile = juce::File::getCurrentWorkingDirectory()
                    .getChildFile("assets")
                    .getChildFile("kkmain.png");

   #if JUCE_WINDOWS
    if (!mainBodyFile.existsAsFile())
    {
        mainBodyFile = juce::File::getSpecialLocation(juce::File::commonApplicationDataDirectory)
                          .getChildFile("King Studios")
                          .getChildFile("The Kings Cab")
                          .getChildFile("assets")
                          .getChildFile("kkmain.png");
    }
   #endif
   #if JUCE_MAC
    if (!mainBodyFile.existsAsFile())
        mainBodyFile = juce::File("/Users/Shared/King Studios/The Kings Cab/assets/kkmain.png");
   #endif
    
    // Try relative to executable
    if (!mainBodyFile.existsAsFile())
    {
        mainBodyFile = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                        .getParentDirectory()
                        .getChildFile("assets")
                        .getChildFile("kkmain.png");
    }
    
    if (mainBodyFile.existsAsFile())
    {
        mainBodyBackgroundImage = juce::ImageFileFormat::loadFrom(mainBodyFile);
        if (mainBodyBackgroundImage.isValid())
        {
            DBG("Main body background loaded: " << mainBodyFile.getFullPathName());
            DBG("Main body size: " << mainBodyBackgroundImage.getWidth() << "x" << mainBodyBackgroundImage.getHeight());
        }
        else
        {
            DBG("Failed to load main body background: " << mainBodyFile.getFullPathName());
        }
    }
    else
    {
        DBG("No main body background found (looking for: kkmain.png)");
    }
}

void TheKingsCabAudioProcessorEditor::loadFooterBackground()
{
    // Try multiple paths for footer background image
    juce::File footerFile;
    
    // Try relative to current working directory
    footerFile = juce::File::getCurrentWorkingDirectory()
                    .getChildFile("assets")
                    .getChildFile("kkfooter.png");

   #if JUCE_WINDOWS
    if (!footerFile.existsAsFile())
    {
        footerFile = juce::File::getSpecialLocation(juce::File::commonApplicationDataDirectory)
                        .getChildFile("King Studios")
                        .getChildFile("The Kings Cab")
                        .getChildFile("assets")
                        .getChildFile("kkfooter.png");
    }
   #endif
   #if JUCE_MAC
    if (!footerFile.existsAsFile())
        footerFile = juce::File("/Users/Shared/King Studios/The Kings Cab/assets/kkfooter.png");
   #endif
    
    // Try relative to executable
    if (!footerFile.existsAsFile())
    {
        footerFile = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                        .getParentDirectory()
                        .getChildFile("assets")
                        .getChildFile("kkfooter.png");
    }
    
    if (footerFile.existsAsFile())
    {
        footerBackgroundImage = juce::ImageFileFormat::loadFrom(footerFile);
        if (footerBackgroundImage.isValid())
        {
            DBG("Footer background loaded: " << footerFile.getFullPathName());
            DBG("Footer size: " << footerBackgroundImage.getWidth() << "x" << footerBackgroundImage.getHeight());
        }
        else
        {
            DBG("Failed to load footer background: " << footerFile.getFullPathName());
        }
    }
    else
    {
        DBG("No footer background found (looking for: kkfooter.png)");
    }
}

//==============================================================================
void TheKingsCabAudioProcessorEditor::drawBackground(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Check if custom background is available (full background override)
    if (customBackgroundImage.isValid())
    {
        // Draw custom background image
        g.drawImageAt(customBackgroundImage, 0, 0);
        
        // Optional: Add subtle overlay for text readability
        g.setColour(juce::Colours::black.withAlpha(0.1f));
        g.fillRect(bounds);
    }
    else
    {
        // Calculate areas for different background treatments
        auto headerArea = bounds.removeFromTop(static_cast<float>(kHeaderHeight));
        auto footerArea = bounds.removeFromBottom(static_cast<float>(kFooterHeight));
        auto mainBodyArea = bounds; // Remaining area in the middle
        
        // Draw main body background image if available (840x500px area)
        if (mainBodyBackgroundImage.isValid())
        {
            g.drawImage(mainBodyBackgroundImage, mainBodyArea, 
                       juce::RectanglePlacement::stretchToFit, false);
        }
        else
        {
            // Fall back to luxury gradient background for main body
            g.setGradientFill(juce::ColourGradient(
                kingsCabLookAndFeel.findColour(KingsCabLookAndFeel::backgroundColourId).brighter(0.15f),
                mainBodyArea.getTopLeft(),
                kingsCabLookAndFeel.findColour(KingsCabLookAndFeel::backgroundColourId).darker(0.25f),
                mainBodyArea.getBottomLeft(),
                false));
            g.fillRect(mainBodyArea);
        }
        
        // Fill header area with base gradient (if no header image)
        auto fullBounds = getLocalBounds().toFloat();
        g.setGradientFill(juce::ColourGradient(
            kingsCabLookAndFeel.findColour(KingsCabLookAndFeel::backgroundColourId).brighter(0.15f),
            fullBounds.getTopLeft(),
            kingsCabLookAndFeel.findColour(KingsCabLookAndFeel::backgroundColourId).darker(0.25f),
            fullBounds.getBottomLeft(),
            false));
        g.fillRect(headerArea);
        
        // Footer area will be handled by drawFooter method
        g.fillRect(footerArea);
        
        // Add radial highlight from center for depth
        auto centerX = bounds.getCentreX();
        auto centerY = bounds.getCentreY();
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.7f;
        
        g.setGradientFill(juce::ColourGradient(
            juce::Colours::white.withAlpha(0.12f), centerX, centerY,
            juce::Colours::transparentBlack, centerX, centerY + radius,
            true));
        g.fillRect(bounds);
        
        // Premium brushed metal texture with gold accents
        g.setColour(kingsCabLookAndFeel.findColour(KingsCabLookAndFeel::metallicBaseColourId).withAlpha(0.08f));
        for (int y = 0; y < getHeight(); y += 3)
        {
            g.drawHorizontalLine(y, 0.0f, static_cast<float>(getWidth()));
        }
        
        // Gold accent lines removed for cleaner look
        
        // Premium border frame
        g.setColour(kingsCabLookAndFeel.findColour(KingsCabLookAndFeel::goldHighlightColourId).withAlpha(0.7f));
        g.drawRect(bounds, 3.0f);
        
        g.setColour(kingsCabLookAndFeel.findColour(KingsCabLookAndFeel::metallicHighlightColourId).withAlpha(0.5f));
        g.drawRect(bounds.reduced(3.0f), 1.0f);
        
        // Subtle inner shadow for professional depth
        auto shadowArea = bounds.reduced(4.0f);
        g.setGradientFill(juce::ColourGradient(
            juce::Colours::black.withAlpha(0.3f), shadowArea.getTopLeft(),
            juce::Colours::transparentBlack, shadowArea.getTopLeft() + juce::Point<float>(0, 25),
            false));
        g.fillRect(shadowArea.removeFromTop(25));
    }
}

void TheKingsCabAudioProcessorEditor::drawHeader(juce::Graphics& g, const juce::Rectangle<int>& bounds)
{
    auto floatBounds = bounds.toFloat();
    
    // Draw header background image if available
    if (headerBackgroundImage.isValid())
    {
        // Draw the header image stretched to fit the header bounds
        g.drawImage(headerBackgroundImage, floatBounds, 
                   juce::RectanglePlacement::stretchToFit, false);
    }
    else
    {
        // Fallback: Luxury header background with enhanced metallic gradient
        g.setGradientFill(juce::ColourGradient(
            kingsCabLookAndFeel.findColour(KingsCabLookAndFeel::metallicHighlightColourId).brighter(0.2f),
            floatBounds.getTopLeft(),
            kingsCabLookAndFeel.findColour(KingsCabLookAndFeel::metallicBaseColourId).darker(0.1f),
            floatBounds.getBottomLeft(),
            false));
        g.fillRect(floatBounds);
        
        // Enhanced glossy overlay for luxury finish
        auto glossyArea = floatBounds.removeFromTop(floatBounds.getHeight() * 0.6f);
        g.setGradientFill(juce::ColourGradient(
            juce::Colours::white.withAlpha(0.25f), glossyArea.getTopLeft(),
            juce::Colours::white.withAlpha(0.0f), glossyArea.getBottomLeft(),
            false));
        g.fillRect(glossyArea);
    }
}

void TheKingsCabAudioProcessorEditor::drawFooter(juce::Graphics& g, const juce::Rectangle<int>& bounds)
{
    // Footer background - solid black
    g.setColour(juce::Colour(0xff000000));
    g.fillRect(bounds);
}

//==============================================================================
void TheKingsCabAudioProcessorEditor::onIRSelected(int slotIndex, const juce::File& irFile)
{
    // Load IR into the audio processor
    audioProcessor.loadImpulseResponse(slotIndex, irFile);
    
    // Update slot visual state
    if (slotIndex >= 0 && slotIndex < TheKingsCabAudioProcessor::kNumIRSlots)
    {
        auto irInfo = IRManager::getIRInfo(irFile);
        irSlots[slotIndex]->setLoadedIR(irInfo.folder, irInfo.name);
    }
}

void TheKingsCabAudioProcessorEditor::onIRCleared(int slotIndex)
{
    // Clear IR from the audio processor
    audioProcessor.clearImpulseResponse(slotIndex);
}

void TheKingsCabAudioProcessorEditor::initializeIRData()
{
    // Get folders from the IR Manager
    auto folders = audioProcessor.getIRManager().getFolders();
    
    // Update status to show folder count for debugging
    statusLabel->setText("Found " + juce::String(folders.size()) + " folders", juce::dontSendNotification);
    
    // Update all IR slots with folder data
    for (auto& irSlot : irSlots)
    {
        if (irSlot)
        {
            irSlot->updateFolderList(folders);
        }
    }
    
    // No folder browser - slots get data directly
}

void TheKingsCabAudioProcessorEditor::onIRPreview(const IRManager::IRInfo& irInfo)
{
    juce::ignoreUnused(irInfo);
    // Could implement IR preview/audition here in the future
}