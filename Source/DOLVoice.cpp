#include "DOLVoice.h"

namespace
{
    // DOL velocity response:
    // - The recorded WAV layer is still selected from the MIDI velocity.
    // - The same MIDI velocity also controls output amplitude continuously.
    //
    // This is intentionally a gentle first production curve. It preserves
    // the humanized level differences already present in the WAV recordings
    // instead of applying a heavy second attenuation.
    //
    // Gain curve:
    //   minimum velocity -> 0.18 (-14.9 dB)
    //   middle velocity  -> approximately 0.54 (-5.4 dB)
    //   maximum velocity -> 1.00 (0 dB)
    //
    // No gain above 1.0 is allowed here. Master Out will remain a separate
    // future control for the overall instrument level.
    float getDOLVelocityGain(int midiVelocity)
    {
        const float normalized =
            juce::jlimit(
                0.0f,
                1.0f,
                (static_cast<float>(midiVelocity) - 1.0f) / 126.0f
            );

        constexpr float minimumGain = 0.18f;
        constexpr float gainRange = 0.82f;
        constexpr float responseCurve = 1.20f;

        const float shaped =
            std::pow(normalized, responseCurve);

        return juce::jlimit(
            minimumGain,
            1.0f,
            minimumGain + gainRange * shaped
        );
    }
}

DOLVoice::DOLVoice(SampleManager& manager)
    : sampleManager(manager)
{
    updateEnvelope();
}

void DOLVoice::updateEnvelope()
{
    envelopeParameters.attack =
        juce::jmax(
            0.0f,
            attackMilliseconds / 1000.0f
        );

    envelopeParameters.decay =
        juce::jmax(
            0.0f,
            decayMilliseconds / 1000.0f
        );

    envelopeParameters.sustain =
        juce::jlimit(
            0.0f,
            1.0f,
            sustainLevel
        );

    envelopeParameters.release =
        juce::jmax(
            0.001f,
            releaseMilliseconds / 1000.0f
        );

    envelope.setParameters(
        envelopeParameters
    );
}

void DOLVoice::setAttackMilliseconds(
    float milliseconds)
{
    attackMilliseconds =
        juce::jlimit(
            0.0f,
            100.0f,
            milliseconds
        );

    updateEnvelope();
}

void DOLVoice::setDecayMilliseconds(
    float milliseconds)
{
    decayMilliseconds =
        juce::jlimit(
            0.0f,
            2000.0f,
            milliseconds
        );

    updateEnvelope();
}

void DOLVoice::setSustainLevel(
    float level)
{
    sustainLevel =
        juce::jlimit(
            0.0f,
            1.0f,
            level
        );

    updateEnvelope();
}

void DOLVoice::setReleaseMilliseconds(
    float milliseconds)
{
    releaseMilliseconds =
        juce::jlimit(
            1.0f,
            5000.0f,
            milliseconds
        );

    updateEnvelope();
}

bool DOLVoice::canPlaySound(
    juce::SynthesiserSound* sound)
{
    return dynamic_cast<DOLSound*>(sound) != nullptr;
}

void DOLVoice::startNote(
    int midiNoteNumber,
    float velocity,
    juce::SynthesiserSound* sound,
    int currentPitchWheelPosition)
{
    juce::ignoreUnused(
        sound,
        currentPitchWheelPosition
    );

    currentSample = nullptr;
    currentSamplePosition = 0.0;
    currentGain = 1.0f;

    const int midiVelocity =
        juce::jlimit(
            1,
            127,
            static_cast<int>(
                velocity * 127.0f
            )
        );

    // IMPORTANT:
    // Velocity does two independent jobs:
    // 1. SampleManager chooses the recorded velocity layer.
    // 2. This continuous gain follows the actual MIDI velocity inside
    //    that layer. Therefore V01 can still respond differently at
    //    velocity 5, 10, 20, etc.; it is not locked to one volume.
    currentGain =
        getDOLVelocityGain(midiVelocity);

    const bool rightHand =
        midiNoteNumber == 38
        || midiNoteNumber == 50;

    if (midiNoteNumber == 36
        || midiNoteNumber == 38
        || midiNoteNumber == 40
        || midiNoteNumber == 41)
    {
        currentSample =
            sampleManager.getHeadSample(
                midiVelocity,
                rightHand
            );
    }
    else if (midiNoteNumber == 42
             || midiNoteNumber == 44
             || midiNoteNumber == 46)
    {
        currentSample =
            sampleManager.getEdgeSample(
                midiVelocity,
                rightHand
            );
    }

    if (currentSample == nullptr)
    {
        clearCurrentNote();
        return;
    }

    playbackRate =
        sampleManager.getSampleRate()
        / getSampleRate();

    envelope.setSampleRate(
        getSampleRate()
    );

    envelope.setParameters(
        envelopeParameters
    );

    envelope.reset();
    envelope.noteOn();
}

void DOLVoice::stopNote(
    float velocity,
    bool allowTailOff)
{
    juce::ignoreUnused(velocity);

    if (!allowTailOff)
    {
        envelope.reset();
        clearCurrentNote();
        return;
    }

    envelope.noteOff();
}

void DOLVoice::pitchWheelMoved(
    int newPitchWheelValue)
{
    juce::ignoreUnused(
        newPitchWheelValue
    );
}

void DOLVoice::controllerMoved(
    int controllerNumber,
    int newControllerValue)
{
    juce::ignoreUnused(
        controllerNumber,
        newControllerValue
    );
}

void DOLVoice::renderNextBlock(
    juce::AudioBuffer<float>& outputBuffer,
    int startSample,
    int numSamples)
{
    if (currentSample == nullptr)
        return;

    const int sampleLength =
        currentSample->getNumSamples();

    const int sampleChannels =
        currentSample->getNumChannels();

    if (sampleLength <= 0
        || sampleChannels <= 0)
    {
        envelope.reset();
        clearCurrentNote();
        return;
    }

    for (int sample = 0;
         sample < numSamples;
         ++sample)
    {
        if (currentSamplePosition >=
            sampleLength - 1)
        {
            envelope.reset();
            clearCurrentNote();
            break;
        }

        const int indexA =
            static_cast<int>(
                currentSamplePosition
            );

        const int indexB =
            juce::jmin(
                indexA + 1,
                sampleLength - 1
            );

        const float fraction =
            static_cast<float>(
                currentSamplePosition
                - static_cast<double>(
                    indexA
                )
            );

        const float envelopeGain =
            envelope.getNextSample();

        for (int channel = 0;
             channel < outputBuffer.getNumChannels();
             ++channel)
        {
            const int sourceChannel =
                juce::jmin(
                    channel,
                    sampleChannels - 1
                );

            const float sampleA =
                currentSample->getSample(
                    sourceChannel,
                    indexA
                );

            const float sampleB =
                currentSample->getSample(
                    sourceChannel,
                    indexB
                );

            const float outputSample =
                sampleA
                + (sampleB - sampleA)
                  * fraction;

            outputBuffer.addSample(
                channel,
                startSample + sample,
                outputSample
                    * currentGain
                    * envelopeGain
            );
        }

        currentSamplePosition +=
            playbackRate;

        if (!envelope.isActive())
        {
            clearCurrentNote();
            break;
        }
    }
}
