#pragma once

#include <JuceHeader.h>

class SampleManager
{
public:
    SampleManager();

    bool loadSamples(const juce::File& sampleFolder);

    const juce::AudioSampleBuffer* getHeadSample(
        int velocity,
        bool rightHand);

    const juce::AudioSampleBuffer* getEdgeSample(
        int velocity,
        bool rightHand);

    int getHeadLayerForVelocity(int velocity) const;
    int getEdgeLayerForVelocity(int velocity) const;

    double getSampleRate() const noexcept
    {
        return sampleRate;
    }

private:
    juce::AudioFormatManager formatManager;

    // HEAD: 8 recorded velocity layers x 5 round-robin
    juce::OwnedArray<juce::AudioSampleBuffer> headSamples[8];

    // EDGE: 3 velocity layers x 5 round-robin
    juce::OwnedArray<juce::AudioSampleBuffer> edgeSamples[3];

    int headRoundRobinLeft[8]  {};
    int headRoundRobinRight[8] {};

    int edgeRoundRobinLeft[3]  {};
    int edgeRoundRobinRight[3] {};

    double sampleRate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleManager)
};
