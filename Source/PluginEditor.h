#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class Voice2MIDIEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit Voice2MIDIEditor(Voice2MIDIProcessor&);
    ~Voice2MIDIEditor() override = default;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    Voice2MIDIProcessor& processor;
    juce::Label title, subtitle, noteLabel, frequencyLabel;
    juce::Slider sensitivity, stability, transpose, gate;
    juce::Label sensitivityLabel, stabilityLabel, transposeLabel, gateLabel;
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> sensitivityAttachment, stabilityAttachment, transposeAttachment, gateAttachment;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Voice2MIDIEditor)
};
