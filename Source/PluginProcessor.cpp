#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    // Samples ship inside the VST3 bundle at:
    //   <bundle>.vst3/Contents/Resources/Samples
    // Resolve the bundle folder relative to the plugin binary so the
    // plugin works on any machine (macOS & Windows) after installation.
    juce::File getBundledSamplesFolder()
    {
        auto dir = juce::File::getSpecialLocation(
            juce::File::hostApplicationPath
        ).getParentDirectory();

        // Walk up to the .vst3 bundle, then look for the bundled samples.
        for (int i = 0; i < 8 && dir != dir.getParentDirectory(); ++i)
        {
            if (dir.getFileName().endsWithIgnoreCase(".vst3"))
            {
                const auto samples = dir.getChildFile(
                    "Contents/Resources/Samples");

                if (samples.isDirectory())
                    return samples;
            }

            dir = dir.getParentDirectory();
        }

        // Development fallback (DOL_SAMPLE_PATH is set by CMake).
        return juce::File(DOL_SAMPLE_PATH);
    }
}

DOL_BKLAudioProcessor::DOL_BKLAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "DOL_BKL_PARAMETERS", createParameterLayout())
{
    sampleManager.loadSamples(getBundledSamplesFolder());
    for (int i = 0; i < 32; ++i) synthesiser.addVoice(new DOLVoice(sampleManager));
    synthesiser.addSound(new DOLSound());
    updateVoiceEnvelopeParameters();
}
DOL_BKLAudioProcessor::~DOL_BKLAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout DOL_BKLAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterFloat>("ATTACK","Attack",juce::NormalisableRange<float>(0.0f,50.0f,0.01f),1.0f,juce::AudioParameterFloatAttributes().withLabel("ms")));
    layout.add(std::make_unique<juce::AudioParameterFloat>("DECAY","Decay",juce::NormalisableRange<float>(0.0f,2000.0f,0.1f),250.0f,juce::AudioParameterFloatAttributes().withLabel("ms")));
    layout.add(std::make_unique<juce::AudioParameterFloat>("SUSTAIN","Sustain",juce::NormalisableRange<float>(0.0f,100.0f,0.1f),100.0f,juce::AudioParameterFloatAttributes().withLabel("%")));
    layout.add(std::make_unique<juce::AudioParameterFloat>("RELEASE_5S","Release",juce::NormalisableRange<float>(1.0f,5000.0f,0.1f),80.0f,juce::AudioParameterFloatAttributes().withLabel("ms")));

    // Effects: single-knob wet controls. Defaults match the approved UI design.
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "ROOM", "Room",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        30.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "REVERB", "Reverb",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        25.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterFloat>("MASTER_OUT","Master Out",juce::NormalisableRange<float>(-24.0f,6.0f,0.01f),0.0f,juce::AudioParameterFloatAttributes().withLabel("dB")));
    return layout;
}
void DOL_BKLAudioProcessor::updateVoiceEnvelopeParameters()
{
    const float a=apvts.getRawParameterValue("ATTACK")->load();
    const float d=apvts.getRawParameterValue("DECAY")->load();
    const float s=apvts.getRawParameterValue("SUSTAIN")->load()/100.0f;
    const float r=apvts.getRawParameterValue("RELEASE_5S")->load();
    for (int i=0;i<synthesiser.getNumVoices();++i)
        if (auto* v=dynamic_cast<DOLVoice*>(synthesiser.getVoice(i))) {
            v->setAttackMilliseconds(a); v->setDecayMilliseconds(d);
            v->setSustainLevel(s); v->setReleaseMilliseconds(r);
        }
}
const juce::String DOL_BKLAudioProcessor::getName() const { return "DOL BENGKULU"; }
bool DOL_BKLAudioProcessor::acceptsMidi() const { return true; }
bool DOL_BKLAudioProcessor::producesMidi() const { return false; }
bool DOL_BKLAudioProcessor::isMidiEffect() const { return false; }
double DOL_BKLAudioProcessor::getTailLengthSeconds() const { return 3.0; }
int DOL_BKLAudioProcessor::getNumPrograms() { return 1; }
int DOL_BKLAudioProcessor::getCurrentProgram() { return 0; }
void DOL_BKLAudioProcessor::setCurrentProgram(int i){juce::ignoreUnused(i);}
const juce::String DOL_BKLAudioProcessor::getProgramName(int i){juce::ignoreUnused(i);return "Default";}
void DOL_BKLAudioProcessor::changeProgramName(int i,const juce::String& n){juce::ignoreUnused(i,n);}
void DOL_BKLAudioProcessor::prepareToPlay(double sr, int spb)
{
    juce::ignoreUnused(spb);

    currentSampleRate = sr;
    synthesiser.setCurrentPlaybackSampleRate(sr);

    roomReverb.reset();
    reverb.reset();

    juce::Reverb::Parameters roomParams;
    roomParams.roomSize = 0.18f;
    roomParams.damping = 0.78f;
    roomParams.wetLevel = 0.0f;
    roomParams.dryLevel = 1.0f;
    roomParams.width = 1.0f;
    roomParams.freezeMode = 0.0f;
    roomReverb.setParameters(roomParams);

    juce::Reverb::Parameters reverbParams;
    reverbParams.roomSize = 0.62f;
    reverbParams.damping = 0.42f;
    reverbParams.wetLevel = 0.0f;
    reverbParams.dryLevel = 1.0f;
    reverbParams.width = 1.0f;
    reverbParams.freezeMode = 0.0f;
    reverb.setParameters(reverbParams);

    updateVoiceEnvelopeParameters();
}
void DOL_BKLAudioProcessor::releaseResources()
{
    roomReverb.reset();
    reverb.reset();
}
bool DOL_BKLAudioProcessor::isBusesLayoutSupported(const BusesLayout& l) const {auto s=l.getMainOutputChannelSet();return s==juce::AudioChannelSet::mono()||s==juce::AudioChannelSet::stereo();}
void DOL_BKLAudioProcessor::processBlock(
    juce::AudioBuffer<float>& b,
    juce::MidiBuffer& m)
{
    // DOL_UI_MIDI_BRIDGE_BEGIN
    // Thread-safe: hanya mencatat state note di dalam MidiKeyboardState.
    // TIDAK menyentuh Editor/UI di sini -- itu tugas Editor lewat Timer
    // yang berjalan di message thread (lihat PluginEditor::timerCallback).
    midiKeyboardState.processNextMidiBuffer(
        m, 0, b.getNumSamples(), false);
    // DOL_UI_MIDI_BRIDGE_END

    b.clear();

    updateVoiceEnvelopeParameters();
    synthesiser.renderNextBlock(
        b, m, 0, b.getNumSamples());

    const float roomMix =
        juce::jlimit(0.0f, 1.0f,
            apvts.getRawParameterValue("ROOM")->load() / 100.0f);

    const float reverbMix =
        juce::jlimit(0.0f, 1.0f,
            apvts.getRawParameterValue("REVERB")->load() / 100.0f);

    auto roomParams = roomReverb.getParameters();
    roomParams.wetLevel = roomMix * 0.35f;
    roomParams.dryLevel = 1.0f;
    roomReverb.setParameters(roomParams);

    auto reverbParams = reverb.getParameters();
    reverbParams.wetLevel = reverbMix * 0.45f;
    reverbParams.dryLevel = 1.0f;
    reverb.setParameters(reverbParams);

    if (b.getNumChannels() >= 2)
    {
        roomReverb.processStereo(
            b.getWritePointer(0),
            b.getWritePointer(1),
            b.getNumSamples());

        reverb.processStereo(
            b.getWritePointer(0),
            b.getWritePointer(1),
            b.getNumSamples());
    }
    else if (b.getNumChannels() == 1)
    {
        reverb.processMono(
            b.getWritePointer(0),
            b.getNumSamples());
    }

    const float db =
        apvts.getRawParameterValue("MASTER_OUT")->load();

    b.applyGain(
        juce::Decibels::decibelsToGain(db));
}
bool DOL_BKLAudioProcessor::hasEditor() const{return true;}
juce::AudioProcessorEditor* DOL_BKLAudioProcessor::createEditor(){return new DOL_BKLAudioProcessorEditor(*this);}
void DOL_BKLAudioProcessor::getStateInformation(juce::MemoryBlock& d){if(auto x=apvts.copyState().createXml())copyXmlToBinary(*x,d);}
void DOL_BKLAudioProcessor::setStateInformation(const void* data,int size){
    if(auto x=getXmlFromBinary(data,size)) if(x->hasTagName(apvts.state.getType())) apvts.replaceState(juce::ValueTree::fromXml(*x));
    updateVoiceEnvelopeParameters();
}
void DOL_BKLAudioProcessor::setUiActiveNote(int midiNote, bool isDown)
{
    // Sengaja dikosongkan & sebaiknya tidak dipanggil lagi.
    // Diganti oleh midiKeyboardState + polling Timer di Editor,
    // karena fungsi ini sebelumnya dipanggil dari audio thread
    // dan langsung memicu repaint() UI -- itu melanggar aturan
    // threading JUCE dan menyebabkan key-light "lari dari mapping".
    juce::ignoreUnused(midiNote, isDown);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new DOL_BKLAudioProcessor();}
