#include "SampleManager.h"

SampleManager::SampleManager()
{
    formatManager.registerBasicFormats();
}

int SampleManager::getHeadLayerForVelocity(int velocity) const
{
    velocity = juce::jlimit(1, 127, velocity);

    // 8 real recorded velocity layers spread across MIDI 1-127.
    if (velocity <= 15)  return 0; // V01
    if (velocity <= 31)  return 1; // V02
    if (velocity <= 47)  return 2; // V03
    if (velocity <= 63)  return 3; // V04
    if (velocity <= 79)  return 4; // V05
    if (velocity <= 95)  return 5; // V06
    if (velocity <= 111) return 6; // V07

    return 7; // V08
}

int SampleManager::getEdgeLayerForVelocity(int velocity) const
{
    velocity = juce::jlimit(1, 127, velocity);

    if (velocity <= 42)
        return 0;

    if (velocity <= 84)
        return 1;

    return 2;
}

bool SampleManager::loadSamples(const juce::File& sampleFolder)
{
    for (auto& layer : headSamples)
        layer.clear();

    for (auto& layer : edgeSamples)
        layer.clear();

    for (int i = 0; i < 8; ++i)
    {
        headRoundRobinLeft[i] = 0;
        headRoundRobinRight[i] = 0;
    }

    for (int i = 0; i < 3; ++i)
    {
        edgeRoundRobinLeft[i] = 0;
        edgeRoundRobinRight[i] = 0;
    }

    if (!sampleFolder.isDirectory())
        return false;

    const auto headFolder = sampleFolder.getChildFile("Head");
    const auto edgeFolder = sampleFolder.getChildFile("Edge");

    if (!headFolder.isDirectory() || !edgeFolder.isDirectory())
        return false;

    // HEAD: V01-V08, each with RR01-RR05
    for (int velocityLayer = 1; velocityLayer <= 8; ++velocityLayer)
    {
        const int layer = velocityLayer - 1;

        for (int rr = 1; rr <= 5; ++rr)
        {
            const auto file = headFolder.getChildFile(
                "V" + juce::String(velocityLayer).paddedLeft('0', 2)
                + "_RR" + juce::String(rr).paddedLeft('0', 2)
                + ".wav"
            );

            if (!file.existsAsFile())
                return false;

            std::unique_ptr<juce::AudioFormatReader> reader(
                formatManager.createReaderFor(file)
            );

            if (reader == nullptr)
                return false;

            if (velocityLayer == 1 && rr == 1)
                sampleRate = reader->sampleRate;

            auto* buffer = new juce::AudioSampleBuffer(
                static_cast<int>(reader->numChannels),
                static_cast<int>(reader->lengthInSamples)
            );

            reader->read(
                buffer,
                0,
                static_cast<int>(reader->lengthInSamples),
                0,
                true,
                true
            );

            headSamples[layer].add(buffer);
        }
    }

    // EDGE: V01-V03, each with RR01-RR05
    for (int velocityLayer = 1; velocityLayer <= 3; ++velocityLayer)
    {
        const int layer = velocityLayer - 1;

        for (int rr = 1; rr <= 5; ++rr)
        {
            const auto file = edgeFolder.getChildFile(
                "V" + juce::String(velocityLayer).paddedLeft('0', 2)
                + "_RR" + juce::String(rr).paddedLeft('0', 2)
                + ".wav"
            );

            if (!file.existsAsFile())
                return false;

            std::unique_ptr<juce::AudioFormatReader> reader(
                formatManager.createReaderFor(file)
            );

            if (reader == nullptr)
                return false;

            auto* buffer = new juce::AudioSampleBuffer(
                static_cast<int>(reader->numChannels),
                static_cast<int>(reader->lengthInSamples)
            );

            reader->read(
                buffer,
                0,
                static_cast<int>(reader->lengthInSamples),
                0,
                true,
                true
            );

            edgeSamples[layer].add(buffer);
        }
    }

    for (int i = 0; i < 8; ++i)
        if (headSamples[i].size() != 5)
            return false;

    for (int i = 0; i < 3; ++i)
        if (edgeSamples[i].size() != 5)
            return false;

    return true;
}

const juce::AudioSampleBuffer*
SampleManager::getHeadSample(int velocity, bool rightHand)
{
    const int layer = getHeadLayerForVelocity(velocity);

    int& rr = rightHand
        ? headRoundRobinRight[layer]
        : headRoundRobinLeft[layer];

    auto* sample = headSamples[layer][rr];
    rr = (rr + 1) % headSamples[layer].size();

    return sample;
}

const juce::AudioSampleBuffer*
SampleManager::getEdgeSample(int velocity, bool rightHand)
{
    const int layer = getEdgeLayerForVelocity(velocity);

    int& rr = rightHand
        ? edgeRoundRobinRight[layer]
        : edgeRoundRobinLeft[layer];

    auto* sample = edgeSamples[layer][rr];
    rr = (rr + 1) % edgeSamples[layer].size();

    return sample;
}
