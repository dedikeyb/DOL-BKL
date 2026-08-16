#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class DOLBKLDrumKnob : public juce::Slider
{
public:
    DOLBKLDrumKnob();
    void paint(juce::Graphics& g) override;

private:
    juce::Image face;
};

// Circular "i" button in the top-right corner. Opens the credits overlay.
class DOLInfoButton : public juce::Button
{
public:
    DOLInfoButton();

private:
    void paintButton(juce::Graphics& g, bool isOver, bool isDown) override;
};

// Full-editor overlay showing the credits. Click outside the panel or on
// the close button to dismiss.
class DOLCreditsOverlay : public juce::Component
{
public:
    DOLCreditsOverlay();

    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;

private:
    juce::Rectangle<int> getPanelBounds() const;
};

class DOL_BKLAudioProcessorEditor
    : public juce::AudioProcessorEditor,
      private juce::Timer
{
public:
    explicit DOL_BKLAudioProcessorEditor(DOL_BKLAudioProcessor&);
    ~DOL_BKLAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    using SliderAttachment =
        juce::AudioProcessorValueTreeState::SliderAttachment;

    void setupKnob(DOLBKLDrumKnob&, const juce::String&);
    void setupLabel(juce::Label&, const juce::String&);
    void attachIfPresent(
        const juce::String& parameterID,
        DOLBKLDrumKnob& slider,
        std::unique_ptr<SliderAttachment>& attachment);
    void drawKeyGlow(
        juce::Graphics& g,
        juce::Rectangle<float> area,
        bool active);
    void drawRoundKeyLight(
        juce::Graphics& g,
        juce::Point<float> centre,
        float radius,
        bool active,
        float intensity,
        juce::Colour colour);

    DOL_BKLAudioProcessor& audioProcessor;
    juce::Image background;

    DOLBKLDrumKnob attackKnob;
    DOLBKLDrumKnob decayKnob;
    DOLBKLDrumKnob sustainKnob;
    DOLBKLDrumKnob releaseKnob;
    DOLBKLDrumKnob roomKnob;
    DOLBKLDrumKnob reverbKnob;
    DOLBKLDrumKnob masterKnob;

    juce::Label attackLabel;
    juce::Label decayLabel;
    juce::Label sustainLabel;
    juce::Label releaseLabel;
    juce::Label roomLabel;
    juce::Label reverbLabel;
    juce::Label masterLabel;

    std::unique_ptr<SliderAttachment> attackAttachment;
    std::unique_ptr<SliderAttachment> decayAttachment;
    std::unique_ptr<SliderAttachment> sustainAttachment;
    std::unique_ptr<SliderAttachment> releaseAttachment;
    std::unique_ptr<SliderAttachment> roomAttachment;
    std::unique_ptr<SliderAttachment> reverbAttachment;
    std::unique_ptr<SliderAttachment> masterAttachment;

    std::atomic<float> keyC1  { 0.0f };
    std::atomic<float> keyD1  { 0.0f };
    std::atomic<float> keyE1  { 0.0f };
    std::atomic<float> keyF1  { 0.0f };
    std::atomic<float> keyFs1 { 0.0f };
    std::atomic<float> keyGs1 { 0.0f };
    std::atomic<float> keyAs1 { 0.0f };

    DOLInfoButton infoButton;
    std::unique_ptr<DOLCreditsOverlay> creditsOverlay;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        DOL_BKLAudioProcessorEditor)
};
