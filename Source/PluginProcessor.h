#pragma once

#include <JuceHeader.h>
#include "SampleManager.h"
#include "DOLVoice.h"

class DOL_BKLAudioProcessor : public juce::AudioProcessor
{
public:
    DOL_BKLAudioProcessor();
    ~DOL_BKLAudioProcessor() override;

    void prepareToPlay(
        double sampleRate,
        int samplesPerBlock) override;

    void releaseResources() override;

    bool isBusesLayoutSupported(
        const BusesLayout& layouts) const override;

    void processBlock(
        juce::AudioBuffer<float>&,
        juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;

    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;

    void setCurrentProgram(int index) override;

    const juce::String getProgramName(int index) override;

    void changeProgramName(
        int index,
        const juce::String& newName) override;

    void getStateInformation(
        juce::MemoryBlock& destData) override;

    void setStateInformation(
        const void* data,
        int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() noexcept
    {
        return apvts;
    }

    // UI-only MIDI activity bridge. Does not alter the audio path.
    juce::MidiKeyboardState& getMidiKeyboardState() noexcept
    {
        return midiKeyboardState;
    }

public:
    void setUiActiveNote(int midiNote, bool isDown);

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout
    createParameterLayout();

    void updateVoiceEnvelopeParameters();

    SampleManager sampleManager;
    juce::Synthesiser synthesiser;

    juce::AudioProcessorValueTreeState apvts;

    // Short natural room ambience and longer reverb tail.
    juce::Reverb roomReverb;
    juce::Reverb reverb;

    // Stored for effect preparation and future parameter smoothing.
    double currentSampleRate = 44100.0;

    // Observes incoming MIDI for UI key animation.
    juce::MidiKeyboardState midiKeyboardState;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        DOL_BKLAudioProcessor
    )
};
