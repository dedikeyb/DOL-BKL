#pragma once

#include <JuceHeader.h>
#include "SampleManager.h"

class DOLSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote(int midiNoteNumber) override
    {
        return midiNoteNumber == 36
            || midiNoteNumber == 38
            || midiNoteNumber == 40
            || midiNoteNumber == 41
            || midiNoteNumber == 42
            || midiNoteNumber == 44
            || midiNoteNumber == 46;
    }

    bool appliesToChannel(int midiChannel) override
    {
        juce::ignoreUnused(midiChannel);
        return true;
    }
};

class DOLVoice : public juce::SynthesiserVoice
{
public:
    explicit DOLVoice(SampleManager& manager);

    bool canPlaySound(juce::SynthesiserSound* sound) override;

    void startNote(
        int midiNoteNumber,
        float velocity,
        juce::SynthesiserSound* sound,
        int currentPitchWheelPosition) override;

    void stopNote(
        float velocity,
        bool allowTailOff) override;

    void pitchWheelMoved(int newPitchWheelValue) override;

    void controllerMoved(
        int controllerNumber,
        int newControllerValue) override;

    void renderNextBlock(
        juce::AudioBuffer<float>& outputBuffer,
        int startSample,
        int numSamples) override;

    // Envelope controls.
    // These are realtime voice parameters; no sample data is changed.
    void setAttackMilliseconds(float milliseconds);
    void setDecayMilliseconds(float milliseconds);
    void setSustainLevel(float level);
    void setReleaseMilliseconds(float milliseconds);

private:
    void updateEnvelope();

    SampleManager& sampleManager;

    const juce::AudioSampleBuffer* currentSample = nullptr;

    double currentSamplePosition = 0.0;
    double playbackRate = 1.0;

    float currentGain = 1.0f;

    juce::ADSR envelope;
    juce::ADSR::Parameters envelopeParameters;

    // Initial percussion-friendly defaults.
    float attackMilliseconds = 1.0f;
    float decayMilliseconds = 0.0f;
    float sustainLevel = 1.0f;
    float releaseMilliseconds = 80.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DOLVoice)
};
