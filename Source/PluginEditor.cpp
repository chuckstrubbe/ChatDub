#include "PluginEditor.h"

Voice2MIDIEditor::Voice2MIDIEditor(Voice2MIDIProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(520, 360);
    title.setText("VOICE2MIDI", juce::dontSendNotification);
    title.setFont(juce::Font(juce::FontOptions(27.0f, juce::Font::bold)));
    title.setColour(juce::Label::textColourId, juce::Colour(0xfff3f5fa));
    addAndMakeVisible(title);
    subtitle.setText("REAL-TIME VOICE TO MIDI", juce::dontSendNotification);
    subtitle.setFont(juce::Font(juce::FontOptions(11.0f)));
    subtitle.setColour(juce::Label::textColourId, juce::Colour(0xff9da8ba));
    addAndMakeVisible(subtitle);
    noteLabel.setText("Listening...", juce::dontSendNotification);
    noteLabel.setJustificationType(juce::Justification::centred);
    noteLabel.setFont(juce::Font(juce::FontOptions(34.0f, juce::Font::bold)));
    noteLabel.setColour(juce::Label::textColourId, juce::Colour(0xff72e0bd));
    addAndMakeVisible(noteLabel);
    frequencyLabel.setText("Fundamental frequency", juce::dontSendNotification);
    frequencyLabel.setJustificationType(juce::Justification::centred);
    frequencyLabel.setColour(juce::Label::textColourId, juce::Colour(0xffaab4c4));
    addAndMakeVisible(frequencyLabel);

    auto setupSlider = [this](juce::Slider& s, juce::Label& l, const juce::String& text)
    {
        l.setText(text, juce::dontSendNotification);
        l.setColour(juce::Label::textColourId, juce::Colour(0xffdce2ec));
        addAndMakeVisible(l);
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 76, 20);
        s.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff72e0bd));
        s.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xfff3f5fa));
        s.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff202938));
        addAndMakeVisible(s);
    };
    setupSlider(sensitivity, sensitivityLabel, "Sensitivity");
    setupSlider(stability, stabilityLabel, "Note stability");
    setupSlider(transpose, transposeLabel, "Transpose");
    setupSlider(gate, gateLabel, "Input gate");

    sensitivityAttachment = std::make_unique<SliderAttachment>(processor.parameters, "threshold", sensitivity);
    stabilityAttachment = std::make_unique<SliderAttachment>(processor.parameters, "minDuration", stability);
    transposeAttachment = std::make_unique<SliderAttachment>(processor.parameters, "transpose", transpose);
    gateAttachment = std::make_unique<SliderAttachment>(processor.parameters, "gate", gate);
    startTimerHz(12);
}

void Voice2MIDIEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff111722));
    g.setColour(juce::Colour(0xff202b3a));
    g.fillRoundedRectangle(getLocalBounds().toFloat().reduced(12.0f), 14.0f);
    g.setColour(juce::Colour(0xff72e0bd));
    g.fillRoundedRectangle(24.0f, 88.0f, 472.0f, 112.0f, 10.0f);
    g.setColour(juce::Colour(0xff17232e));
    g.fillRoundedRectangle(27.0f, 91.0f, 466.0f, 106.0f, 8.0f);
}

void Voice2MIDIEditor::resized()
{
    title.setBounds(28, 18, 250, 36);
    subtitle.setBounds(30, 52, 250, 20);
    noteLabel.setBounds(34, 102, 452, 54);
    frequencyLabel.setBounds(34, 157, 452, 25);
    const int x[] = { 48, 162, 276, 390 };
    juce::Slider* sliders[] = { &sensitivity, &stability, &transpose, &gate };
    juce::Label* labels[] = { &sensitivityLabel, &stabilityLabel, &transposeLabel, &gateLabel };
    for (int i = 0; i < 4; ++i)
    {
        labels[i]->setJustificationType(juce::Justification::centred);
        labels[i]->setBounds(x[i] - 8, 218, 100, 22);
        sliders[i]->setBounds(x[i], 238, 84, 104);
    }
}

void Voice2MIDIEditor::timerCallback()
{
    noteLabel.setText(processor.getDetectedNoteName(), juce::dontSendNotification);
    const float hz = processor.getDetectedFrequency();
    frequencyLabel.setText(hz > 0.0f ? juce::String(hz, 1) + " Hz" : "Waiting for a clear vocal pitch", juce::dontSendNotification);
}
