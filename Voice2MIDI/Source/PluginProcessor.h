#pragma once
#include <JuceHeader.h>
#include "YinPitchDetector.h"

class Voice2MIDIProcessor : public juce::AudioProcessor
{
public:
    Voice2MIDIProcessor();
    ~Voice2MIDIProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return true; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.1; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    juce::AudioProcessorValueTreeState parameters;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::String getDetectedNoteName() const;
    float getDetectedFrequency() const { return detectedFrequency.load(); }

private:
    YinPitchDetector detector;
    std::vector<float> analysisBuffer;
    int writePosition = 0;
    int samplesSinceAnalysis = 0;
    int currentMidiNote = -1;
    int candidateMidiNote = -1;
    int candidateSamples = 0;
    int silenceSamples = 0;
    double currentSampleRate = 44100.0;
    std::atomic<float> detectedFrequency { 0.0f };
    std::atomic<int> displayedMidiNote { -1 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Voice2MIDIProcessor)
};
