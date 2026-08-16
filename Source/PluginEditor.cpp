#include "PluginEditor.h"
#include "BinaryData.h"
#include <cmath>

namespace
{
    constexpr int W = 800;
    constexpr int H = 500;

    const juce::Colour gold       (0xffd7a94b);
    const juce::Colour goldBright (0xffffd978);
    const juce::Colour cream      (0xfff1dfb0);
    const juce::Colour darkPanel  (0xe61a120b);

    // ===== KREDIT / CREDITS =====
    // Tambahkan nama & peran setiap orang yang terlibat di sini.
    struct CreditEntry
    {
        const char* name;
        const char* role;
    };

    const CreditEntry kCredits[] = {
        { "DEDI SANTOSO", "CREATOR, DEVELOPER & PROGRAMMER" },
        { "REZA KURNIAWAN", "OPERATOR SAMPLING DATA AUDIO" },
        { "LISTER RECORD BENGKULU", "STUDIO" },
        { "SANGGAR PRP (PESONA RUMPUN PESISIR)", "TALENT" },
    };

    constexpr int kNumCredits =
        static_cast<int>(sizeof(kCredits) / sizeof(kCredits[0]));

    juce::String formatParameterValue(const juce::Slider& s)
    {
        const auto v = s.getValue();
        const auto name = s.getName();

        if (name == "ATTACK" || name == "DECAY" || name == "RELEASE")
            return juce::String(v, 1) + " ms";
        if (name == "SUSTAIN" || name == "ROOM" || name == "REVERB")
            return juce::String(v, 1) + " %";
        if (name == "MASTER OUT")
            return juce::String(v, 1) + " dB";

        return juce::String(v, 1);
    }

    float valueToAngle(const juce::Slider& s)
    {
        const auto range = s.getRange();
        const double length = range.getLength();
        const double n = length > 0.0
            ? juce::jlimit(0.0, 1.0,
                (s.getValue() - range.getStart()) / length)
            : 0.0;

        constexpr float start = juce::degreesToRadians(-135.0f);
        constexpr float end   = juce::degreesToRadians(135.0f);
        return start + (end - start) * static_cast<float>(n);
    }

    // Update atomic key-state, kembalikan true kalau nilainya berubah.
    // Dipanggil HANYA dari message thread (lewat Timer di Editor).
    bool updateKeyState(std::atomic<float>& state, bool isOn)
    {
        const float target = isOn ? 1.0f : 0.0f;
        if (state.load() != target)
        {
            state.store(target);
            return true;
        }
        return false;
    }
}

DOLInfoButton::DOLInfoButton()
    : juce::Button("INFO")
{
    setTooltip("Info & Kredit");
}

void DOLInfoButton::paintButton(
    juce::Graphics& g,
    bool isOver,
    bool isDown)
{
    const auto area = getLocalBounds().toFloat();

    g.setColour(juce::Colour(0xd91a120b));
    g.fillEllipse(area);

    g.setColour(isOver ? goldBright : gold);
    g.drawEllipse(area.reduced(1.0f), isDown ? 2.5f : 1.8f);

    g.setColour(cream);
    g.setFont(juce::Font(juce::FontOptions()
        .withHeight(area.getHeight() * 0.55f)
        .withStyleFlags(juce::Font::FontStyleFlags::italic)));
    g.drawText("i", area, juce::Justification::centred);
}

juce::Rectangle<int> DOLCreditsOverlay::getPanelBounds() const
{
    return { 140, 88, 520, 324 };
}

DOLCreditsOverlay::DOLCreditsOverlay()
{
    setInterceptsMouseClicks(true, true);
}

void DOLCreditsOverlay::paint(juce::Graphics& g)
{
    // Dim the whole editor behind the panel.
    g.fillAll(juce::Colour(0x8c000000));

    const auto panel = getPanelBounds().toFloat();

    g.setColour(juce::Colour(0x2d000000));
    g.fillRoundedRectangle(panel.translated(0.0f, 6.0f), 12.0f);

    g.setColour(juce::Colour(0xf20f0a06));
    g.fillRoundedRectangle(panel, 12.0f);

    g.setColour(gold);
    g.drawRoundedRectangle(panel, 12.0f, 1.5f);
    g.drawRoundedRectangle(panel.reduced(4.0f), 10.0f, 0.8f);

    // Close button (X) at the top-right of the panel.
    const auto closeArea = juce::Rectangle<float>(
        panel.getRight() - 34.0f, panel.getY() + 12.0f,
        22.0f, 22.0f);

    g.setColour(juce::Colour(0xff1a120b));
    g.fillEllipse(closeArea);
    g.setColour(gold);
    g.drawEllipse(closeArea, 1.5f);
    g.setColour(cream);
    g.setFont(juce::Font(juce::FontOptions().withHeight(12.0f)));
    g.drawText("x", closeArea, juce::Justification::centred);

    // Title block.
    g.setColour(goldBright);
    g.setFont(juce::Font(juce::FontOptions()
        .withName("Times New Roman")
        .withHeight(24.0f)
        .withStyleFlags(juce::Font::FontStyleFlags::bold)));
    g.drawText("DOL BENGKULU",
        juce::Rectangle<float>(panel.getX(), 118.0f, panel.getWidth(), 30.0f),
        juce::Justification::centred);

    g.setColour(gold);
    g.setFont(juce::Font(juce::FontOptions().withHeight(11.0f)));
    g.drawText("TRADITIONAL PERCUSSION OF BENGKULU",
        juce::Rectangle<float>(panel.getX(), 148.0f, panel.getWidth(), 16.0f),
        juce::Justification::centred);

    g.setColour(gold);
    g.fillRect(300.0f, 172.0f, 200.0f, 1.0f);

    g.setColour(gold);
    g.setFont(juce::Font(juce::FontOptions()
        .withHeight(13.0f)
        .withStyleFlags(juce::Font::FontStyleFlags::bold)));
    g.drawText("KREDIT  \u00b7  CREDITS",
        juce::Rectangle<float>(panel.getX(), 186.0f, panel.getWidth(), 18.0f),
        juce::Justification::centred);

    // Credits entries.
    float y = 216.0f;
    for (int i = 0; i < kNumCredits; ++i)
    {
        g.setColour(cream);
        g.setFont(juce::Font(juce::FontOptions().withHeight(14.0f)));
        g.drawText(kCredits[i].name,
            juce::Rectangle<float>(panel.getX(), y, panel.getWidth(), 18.0f),
            juce::Justification::centred);

        g.setColour(gold);
        g.setFont(juce::Font(juce::FontOptions().withHeight(11.0f)));
        g.drawText(kCredits[i].role,
            juce::Rectangle<float>(panel.getX(), y + 18.0f, panel.getWidth(), 14.0f),
            juce::Justification::centred);

        y += 42.0f;
    }

    g.setColour(juce::Colour(0xff8a6d3b));
    g.setFont(juce::Font(juce::FontOptions().withHeight(10.0f)));
    g.drawText("VERSION 0.1.0  \u00b7  DOL BKL",
        juce::Rectangle<float>(panel.getX(), 384.0f, panel.getWidth(), 14.0f),
        juce::Justification::centred);
}

void DOLCreditsOverlay::mouseDown(const juce::MouseEvent& e)
{
    const auto panel = getPanelBounds();
    const auto closeArea = juce::Rectangle<int>(
        panel.getRight() - 34, panel.getY() + 12, 22, 22);

    // Close on the X button or on a click outside the panel.
    if (closeArea.contains(e.getPosition())
        || !panel.contains(e.getPosition()))
        setVisible(false);
}

DOLBKLDrumKnob::DOLBKLDrumKnob()
{
    setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::rotarySliderFillColourId,
              juce::Colours::transparentBlack);
    setColour(juce::Slider::rotarySliderOutlineColourId,
              juce::Colours::transparentBlack);

    face = juce::ImageFileFormat::loadFrom(
        BinaryData::knobFace_png,
        BinaryData::knobFace_pngSize);
}

void DOLBKLDrumKnob::paint(juce::Graphics& g)
{
    const auto area = getLocalBounds().toFloat();

    // Component is 88x88: knob face on top, value box at the bottom.
    constexpr float diameter = 68.0f;

    const auto knob = juce::Rectangle<float>(
        (area.getWidth() - diameter) * 0.5f,
        0.0f, diameter, diameter);

    if (face.isValid())
        g.drawImageWithin(
            face, (int) knob.getX(), (int) knob.getY(),
            (int) knob.getWidth(), (int) knob.getHeight(),
            juce::RectanglePlacement::stretchToFit, false);
    else
    {
        g.setColour(juce::Colour(0xff8d6a42));
        g.fillEllipse(knob);
    }

    const auto centre = knob.getCentre();
    // Tick ring hugs the face (bezel) so it never overlaps the value box.
    const float radius = knob.getWidth() * 0.5f + 2.0f;

    for (int i = 0; i <= 30; ++i)
    {
        const float a = juce::degreesToRadians(-135.0f + i * 9.0f);
        const float inner = radius - (i % 5 == 0 ? 5.0f : 2.5f);

        g.setColour(i % 5 == 0 ? goldBright : gold);
        g.drawLine(
            centre.x + std::cos(a) * inner,
            centre.y + std::sin(a) * inner,
            centre.x + std::cos(a) * radius,
            centre.y + std::sin(a) * radius,
            i % 5 == 0 ? 1.2f : 0.7f);
    }

    const float angle = valueToAngle(*this);
    const float len = radius - 13.0f;
    const auto tip = centre + juce::Point<float>(
        std::cos(angle) * len,
        std::sin(angle) * len);

    g.setColour(juce::Colour(0xff2b1b0d));
    g.drawLine(centre.x, centre.y, tip.x, tip.y, 6.0f);

    g.setColour(juce::Colour(0xffc48b4a));
    g.drawLine(centre.x, centre.y, tip.x, tip.y, 3.5f);

    const auto bandCentre = centre + juce::Point<float>(
        std::cos(angle) * len * 0.62f,
        std::sin(angle) * len * 0.62f);

    g.setColour(juce::Colour(0xff9f2520));
    g.fillEllipse(
        bandCentre.x - 2.5f, bandCentre.y - 2.5f, 5.0f, 5.0f);

    g.setColour(juce::Colour(0xff111111));
    g.fillEllipse(centre.x - 5.0f, centre.y - 5.0f, 10.0f, 10.0f);

    g.setColour(goldBright);
    g.drawEllipse(
        centre.x - 5.0f, centre.y - 5.0f, 10.0f, 10.0f, 1.0f);

    // Live-value box below the knob face, clear of the tick ring.
    auto valueBox = area.withTop(72.0f)
                        .withHeight(14.0f)
                        .reduced(12.0f, 0.0f);

    g.setColour(darkPanel);
    g.fillRoundedRectangle(valueBox, 4.0f);

    g.setColour(gold);
    g.drawRoundedRectangle(valueBox, 4.0f, 1.0f);

    g.setColour(cream);
    g.setFont(10.5f);
    g.drawFittedText(
        formatParameterValue(*this),
        valueBox.toNearestInt(),
        juce::Justification::centred, 1);
}

DOL_BKLAudioProcessorEditor::DOL_BKLAudioProcessorEditor(
    DOL_BKLAudioProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p)
{
    setSize(W, H);
    setResizable(false, false);

    background = juce::ImageFileFormat::loadFrom(
        BinaryData::background_png,
        BinaryData::background_pngSize);

    setupKnob(attackKnob, "ATTACK");
    setupKnob(decayKnob, "DECAY");
    setupKnob(sustainKnob, "SUSTAIN");
    setupKnob(releaseKnob, "RELEASE");
    setupKnob(roomKnob, "ROOM");
    setupKnob(reverbKnob, "REVERB");
    setupKnob(masterKnob, "MASTER OUT");
    setupLabel(attackLabel, "ATTACK");
    setupLabel(decayLabel, "DECAY");
    setupLabel(sustainLabel, "SUSTAIN");
    setupLabel(releaseLabel, "RELEASE");
    setupLabel(roomLabel, "ROOM");
    setupLabel(reverbLabel, "REVERB");
    setupLabel(masterLabel, "MASTER OUT");

    attachIfPresent("ATTACK", attackKnob, attackAttachment);
    attachIfPresent("DECAY", decayKnob, decayAttachment);
    attachIfPresent("SUSTAIN", sustainKnob, sustainAttachment);
    attachIfPresent("RELEASE_5S", releaseKnob, releaseAttachment);
    attachIfPresent("ROOM", roomKnob, roomAttachment);
    attachIfPresent("REVERB", reverbKnob, reverbAttachment);
    attachIfPresent("MASTER_OUT", masterKnob, masterAttachment);

    // Info button + credits overlay.
    addAndMakeVisible(infoButton);

    creditsOverlay = std::make_unique<DOLCreditsOverlay>();
    addChildComponent(creditsOverlay.get());
    creditsOverlay->setBounds(0, 0, W, H);

    infoButton.onClick = [this]
    {
        creditsOverlay->setVisible(true);
        creditsOverlay->toFront(false);
    };

    // Polling key-light dari message thread (30x/detik cukup untuk mata,
    // dan aman -- TIDAK pernah dipanggil dari audio thread).
    startTimerHz(30);
}

DOL_BKLAudioProcessorEditor::~DOL_BKLAudioProcessorEditor()
{
    stopTimer();
}

void DOL_BKLAudioProcessorEditor::setupKnob(
    DOLBKLDrumKnob& knob,
    const juce::String& name)
{
    knob.setName(name);
    knob.setDoubleClickReturnValue(false, 0.0);
    addAndMakeVisible(knob);
}

void DOL_BKLAudioProcessorEditor::setupLabel(
    juce::Label& label,
    const juce::String& text)
{
    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, juce::Colour(0xffe8d8b0));
    label.setFont(juce::Font(juce::FontOptions().withHeight(11.0f)));
    label.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(label);
}

void DOL_BKLAudioProcessorEditor::attachIfPresent(
    const juce::String& parameterID,
    DOLBKLDrumKnob& slider,
    std::unique_ptr<SliderAttachment>& attachment)
{
    if (audioProcessor.getAPVTS().getParameter(parameterID) != nullptr)
        attachment = std::make_unique<SliderAttachment>(
            audioProcessor.getAPVTS(), parameterID, slider);
}

void DOL_BKLAudioProcessorEditor::paint(juce::Graphics& g)
{
    if (background.isValid())
        g.drawImageWithin(
            background,
            0, 0, getWidth(), getHeight(),
            juce::RectanglePlacement::stretchToFit,
            false);
    else
        g.fillAll(juce::Colour(0xff090806));

    // PIXEL-CORRECTED INTERACTION LAYER.
    // Based on the supplied VST screenshot at the 800x500 UI coordinate space.
    // HEAD = actual skin surface / center.
    drawRoundKeyLight(g, { 345.0f, 205.0f }, 27.0f,
                      keyC1.load() > 0.5f, keyC1.load(),
                      juce::Colour(0xffffd45c));
    drawRoundKeyLight(g, { 400.0f, 200.0f }, 27.0f,
                      keyD1.load() > 0.5f, keyD1.load(),
                      juce::Colour(0xffffd45c));
    drawRoundKeyLight(g, { 455.0f, 200.0f }, 27.0f,
                      keyE1.load() > 0.5f, keyE1.load(),
                      juce::Colour(0xffffd45c));
    drawRoundKeyLight(g, { 510.0f, 205.0f }, 27.0f,
                      keyF1.load() > 0.5f, keyF1.load(),
                      juce::Colour(0xffffd45c));

    // EDGE = actual lower rim / pinggir kulit.
    drawRoundKeyLight(g, { 350.0f, 278.0f }, 14.0f,
                      keyFs1.load() > 0.5f, keyFs1.load(),
                      juce::Colour(0xff67a9ff));
    drawRoundKeyLight(g, { 425.0f, 292.0f }, 14.0f,
                      keyGs1.load() > 0.5f, keyGs1.load(),
                      juce::Colour(0xff67a9ff));
    drawRoundKeyLight(g, { 500.0f, 278.0f }, 14.0f,
                      keyAs1.load() > 0.5f, keyAs1.load(),
                      juce::Colour(0xff67a9ff));
}

void DOL_BKLAudioProcessorEditor::drawKeyGlow(
    juce::Graphics& g,
    juce::Rectangle<float> area,
    bool active)
{
    if (!active)
        return;

    g.setColour(juce::Colour(0x55ffd45c));
    g.fillRoundedRectangle(area, 7.0f);

    g.setColour(goldBright);
    g.drawRoundedRectangle(area, 7.0f, 2.0f);
}

void DOL_BKLAudioProcessorEditor::drawRoundKeyLight(
    juce::Graphics& g,
    juce::Point<float> centre,
    float radius,
    bool active,
    float intensity,
    juce::Colour colour)
{
    if (!active || intensity <= 0.001f)
        return;

    const float a = juce::jlimit(0.0f, 1.0f, intensity);

    // Circular-only glow. No rectangular mask.
    for (int ring = 5; ring >= 1; --ring)
    {
        const float r = radius * (1.0f + 0.20f * static_cast<float>(ring));
        const float alpha = 0.010f * a * static_cast<float>(6 - ring);
        g.setColour(colour.withAlpha(alpha));
        g.fillEllipse(centre.x - r, centre.y - r, r * 2.0f, r * 2.0f);
    }

    g.setColour(colour.withAlpha(0.11f * a));
    g.fillEllipse(centre.x - radius, centre.y - radius,
                  radius * 2.0f, radius * 2.0f);

    g.setColour(colour.withAlpha(0.75f * a));
    g.drawEllipse(centre.x - radius, centre.y - radius,
                  radius * 2.0f, radius * 2.0f, 1.5f);

    g.setColour(colour.withAlpha(0.95f * a));
    g.fillEllipse(centre.x - 2.5f, centre.y - 2.5f, 5.0f, 5.0f);
}

void DOL_BKLAudioProcessorEditor::resized()
{
    constexpr int knobSize = 88;
    constexpr int knobHeight = 88;
    constexpr int y = 374;

    // Knob row is centred inside the bottom panel (x 29..792), with
    // symmetric margins and even 110px spacing.
    attackKnob.setBounds(36,  y, knobSize, knobHeight);
    decayKnob.setBounds(146, y, knobSize, knobHeight);
    sustainKnob.setBounds(256, y, knobSize, knobHeight);
    releaseKnob.setBounds(366, y, knobSize, knobHeight);
    roomKnob.setBounds(476, y, knobSize, knobHeight);
    reverbKnob.setBounds(586, y, knobSize, knobHeight);
    masterKnob.setBounds(696, y, knobSize, knobHeight);

    // Label row above the knob faces, below the panel's top border.
    constexpr int labelY = 360;
    constexpr int labelH = 12;
    constexpr int labelW = 100;

    attackLabel.setBounds(30,  labelY, labelW, labelH);
    decayLabel.setBounds(140, labelY, labelW, labelH);
    sustainLabel.setBounds(250, labelY, labelW, labelH);
    releaseLabel.setBounds(360, labelY, labelW, labelH);
    roomLabel.setBounds(470, labelY, labelW, labelH);
    reverbLabel.setBounds(580, labelY, labelW, labelH);
    masterLabel.setBounds(690, labelY, labelW, labelH);

    // Info button: pinggir kiri, di area asap (tidak menimpa hero).
    infoButton.setBounds(73, 241, 34, 34);

    if (creditsOverlay != nullptr)
        creditsOverlay->setBounds(getLocalBounds());
}
void DOL_BKLAudioProcessorEditor::timerCallback()
{
    auto& kb = audioProcessor.getMidiKeyboardState();
    constexpr int allChannels = 0xffff;

    // FINAL UI MIDI MAP
    // HEAD: C1 D1 E1 F1 = 36 38 40 41
    // EDGE: F#1 G#1 A#1 = 42 44 46
    const bool c1  = kb.isNoteOnForChannels(allChannels, 36);
    const bool d1  = kb.isNoteOnForChannels(allChannels, 38);
    const bool e1  = kb.isNoteOnForChannels(allChannels, 40);
    const bool f1  = kb.isNoteOnForChannels(allChannels, 41);
    const bool fs1 = kb.isNoteOnForChannels(allChannels, 42);
    const bool gs1 = kb.isNoteOnForChannels(allChannels, 44);
    const bool as1 = kb.isNoteOnForChannels(allChannels, 46);

    bool changed = false;
    changed |= updateKeyState(keyC1,  c1);
    changed |= updateKeyState(keyD1,  d1);
    changed |= updateKeyState(keyE1,  e1);
    changed |= updateKeyState(keyF1,  f1);
    changed |= updateKeyState(keyFs1, fs1);
    changed |= updateKeyState(keyGs1, gs1);
    changed |= updateKeyState(keyAs1, as1);

    if (changed)
        repaint();
}
