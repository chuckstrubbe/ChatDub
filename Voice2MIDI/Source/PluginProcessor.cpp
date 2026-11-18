#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

namespace
{
    constexpr int analysisSize = 2048;
    constexpr int analysisInterval = 1024;
    constexpr int midiChannel = 1;
}

Voice2MIDIProcessor::Voice2MIDIProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                      .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS", createParameterLayout()),
      analysisBuffer(static_cast<size_t>(analysisSize), 0.0f),
      detector(analysisSize)
{
}

juce::AudioProcessorValueTreeState::ParameterLayout Voice2MIDIProcessor::createParameterLayout()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> p;
    p.push_back(std::make_unique<AudioParameterFloat>("threshold", "Sensitivity", NormalisableRange<float>(0.05f, 0.35f, 0.005f), 0.15f));
    p.push_back(std::make_unique<AudioParameterFloat>("minDuration", "Note Stability", NormalisableRange<float>(20.0f, 250.0f, 1.0f, 0.5f), 65.0f, "ms"));
    p.push_back(std::make_unique<AudioParameterInt>("transpose", "Transpose", -24, 24, 0));
    p.push_back(std::make_unique<AudioParameterFloat>("gate", "Gate", NormalisableRange<float>(0.0005f, 0.05f, 0.0005f, 0.5f), 0.004f));
    return { p.begin(), p.end() };
}

void Voice2MIDIProcessor::prepareToPlay(double sampleRate, int)
{
    currentSampleRate = sampleRate;
    detector.prepare(sampleRate);
    std::fill(analysisBuffer.begin(), analysisBuffer.end(), 0.0f);
    writePosition = samplesSinceAnalysis = candidateSamples = silenceSamples = 0;
    currentMidiNote = candidateMidiNote = -1;
    detectedFrequency.store(0.0f);
    displayedMidiNote.store(-1);
}

void Voice2MIDIProcessor::releaseResources() {}

bool Voice2MIDIProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto in = layouts.getMainInputChannelSet();
    const auto out = layouts.getMainOutputChannelSet();
    return (in == juce::AudioChannelSet::mono() || in == juce::AudioChannelSet::stereo()) && out == in;
}

void Voice2MIDIProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    midi.clear();
    const int channels = getTotalNumInputChannels();
    const int samples = buffer.getNumSamples();
    for (int ch = channels; ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear(ch, 0, samples);

    const float threshold = parameters.getRawParameterValue("threshold")->load();
    const float stabilityMs = parameters.getRawParameterValue("minDuration")->load();
    const int transpose = static_cast<int>(parameters.getRawParameterValue("transpose")->load());
    const float gate = parameters.getRawParameterValue("gate")->load();
    const int requiredSamples = juce::jmax(1, static_cast<int>(stabilityMs * 0.001 * currentSampleRate));
    const int releaseSamples = juce::jmax(1, static_cast<int>(0.10 * currentSampleRate));

    for (int i = 0; i < samples; ++i)
    {
        float mono = 0.0f;
        if (channels > 0)
        {
            for (int ch = 0; ch < channels; ++ch)
                mono += buffer.getReadPointer(ch)[i];
            mono /= static_cast<float>(channels);
        }
        analysisBuffer[static_cast<size_t>(writePosition)] = mono;
        writePosition = (writePosition + 1) % analysisSize;
        ++samplesSinceAnalysis;

        if (samplesSinceAnalysis >= analysisInterval)
        {
            samplesSinceAnalysis = 0;
            std::vector<float> ordered(static_cast<size_t>(analysisSize));
            for (int n = 0; n < analysisSize; ++n)
                ordered[static_cast<size_t>(n)] = analysisBuffer[static_cast<size_t>((writePosition + n) % analysisSize)];

            float peak = 0.0f;
            for (float v : ordered)
                peak = juce::jmax(peak, std::abs(v));

            float frequency = peak >= gate ? detector.detect(ordered.data(), analysisSize, threshold) : 0.0f;
            detectedFrequency.store(frequency);
            int detectedNote = -1;
            if (frequency > 0.0f && std::isfinite(frequency))
            {
                const int note = static_cast<int>(std::lround(69.0 + 12.0 * std::log2(frequency / 440.0))) + transpose;
                if (note >= 0 && note <= 127)
                    detectedNote = note;
            }

            if (detectedNote >= 0)
            {
                silenceSamples = 0;
                if (detectedNote == candidateMidiNote)
                    candidateSamples += analysisInterval;
                else
                {
                    candidateMidiNote = detectedNote;
                    candidateSamples = analysisInterval;
                }

                if (candidateSamples >= requiredSamples && detectedNote != currentMidiNote)
                {
                    if (currentMidiNote >= 0)
                        midi.addEvent(juce::MidiMessage::noteOff(midiChannel, currentMidiNote), i);
                    currentMidiNote = detectedNote;
                    midi.addEvent(juce::MidiMessage::noteOn(midiChannel, currentMidiNote, static_cast<juce::uint8>(100)), i);
                }
            }
            else
            {
                candidateMidiNote = -1;
                candidateSamples = 0;
                silenceSamples += analysisInterval;
                if (currentMidiNote >= 0 && silenceSamples >= releaseSamples)
                {
                    midi.addEvent(juce::MidiMessage::noteOff(midiChannel, currentMidiNote), i);
                    currentMidiNote = -1;
                }
            }
            displayedMidiNote.store(currentMidiNote);
        }
    }
}

juce::AudioProcessorEditor* Voice2MIDIProcessor::createEditor()
{
    return new Voice2MIDIEditor(*this);
}

void Voice2MIDIProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, dest);
}

void Voice2MIDIProcessor::setStateInformation(const void* data, int size)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, size));
    if (xml != nullptr && xml->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::String Voice2MIDIProcessor::getDetectedNoteName() const
{
    const int note = displayedMidiNote.load();
    if (note < 0)
        return "Listening...";
    return juce::MidiMessage::getMidiNoteName(note, true, true, 4);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Voice2MIDIProcessor();
}
