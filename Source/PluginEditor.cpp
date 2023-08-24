/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

template<typename T>
bool truncateKiloValue(T& value)
{
    if(value > static_cast<T>(999))
    {
        value /= static_cast<T>(1000);
        return true;
        
    }
    
    return false;
};

juce::String getValString(const juce::RangedAudioParameter& param, bool getLow, juce::String suffix)
{
    juce::String str;
    
    auto val = (getLow) ? param.getNormalisableRange().start : param.getNormalisableRange().end;
    
    bool useK = truncateKiloValue(val);
    
    str << val;
    
    if(useK)
        str << "k";
    
    str << suffix;
    
    return str;
};
//==============================================================================
void LookAndFeel::drawRotarySlider(juce::Graphics & g,
                                   int x,
                                   int y,
                                   int width,
                                   int height,
                                   float sliderPosProportional,
                                   float rotaryStartAngle,
                                   float rotaryEndAngle,
                                   juce::Slider & slider)
{
    using namespace juce;
    
    auto bounds = Rectangle<float>(x, y, width, height);
    
    auto enabled = slider.isEnabled();
    
    g.setColour(enabled ? Colour(97u, 18u, 167u) : Colours::darkgrey );
    g.fillEllipse(bounds);
    
    g.setColour(enabled ? Colour(255u, 154u, 1u) : Colours::grey);
    g.drawEllipse(bounds, 1.f);
    
    if( auto* rswl = dynamic_cast<RotarySliderWithLabels*>(&slider))
    {
        auto center = bounds.getCentre();
        Path p;
        
        Rectangle<float> r;
        r.setLeft(center.getX() - 2);
        r.setRight(center.getX() + 2);
        r.setTop(bounds.getY());
        r.setBottom(center.getY() - rswl->getTextHeight() * 1.5);
        
        p.addRoundedRectangle(r, 2.f);
        
        jassert(rotaryStartAngle < rotaryEndAngle);
        
        auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);
        
        p.applyTransform(AffineTransform().rotated(sliderAngRad, center.getX(), center.getY()));
        
        g.fillPath(p);
        
        g.setFont(rswl->getTextHeight());
        auto text = rswl->getDisplayString();
        auto strWidth = g.getCurrentFont().getStringWidth(text);
        
        r.setSize(strWidth + 4, rswl->getTextHeight() + 2);
        r.setCentre(bounds.getCentre());
        
        g.setColour(enabled ? Colours::black : Colours::darkgrey);
        g.fillRect(r);
        
        g.setColour(enabled ? Colours::white : Colours::lightgrey);
        g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);
    }
}

void LookAndFeel::drawToggleButton(juce::Graphics &g,
                                   juce::ToggleButton &toggleButton,
                                   bool shouldDrawButtonAsHighlighted,
                                   bool shouldDrawButtonAsDown)
{
    using namespace juce;
    
    if( auto* pb = dynamic_cast<PowerButton*>(&toggleButton) )
    {
        Path powerButton;
        
        auto bounds = toggleButton.getLocalBounds();
        
        auto size = jmin(bounds.getWidth(), bounds.getHeight()) - 6;
        auto r = bounds.withSizeKeepingCentre(size, size).toFloat();
        
        float ang = 30.f; //30.f;
        
        size -= 6;
        
        powerButton.addCentredArc(r.getCentreX(),
                                  r.getCentreY(),
                                  size * 0.5,
                                  size * 0.5,
                                  0.f,
                                  degreesToRadians(ang),
                                  degreesToRadians(360.f - ang),
                                  true);
        
        powerButton.startNewSubPath(r.getCentreX(), r.getY());
        powerButton.lineTo(r.getCentre());
        
        PathStrokeType pst(2.f, PathStrokeType::JointStyle::curved);
        
        auto color = toggleButton.getToggleState() ? Colours::dimgrey : Colour(0u, 172u, 1u);
        
        g.setColour(color);
        g.strokePath(powerButton, pst);
        g.drawEllipse(r, 2);
    }
    else if( auto* analyzerButton = dynamic_cast<AnalyzerButton*>(&toggleButton) )
    {
        auto color = ! toggleButton.getToggleState() ? Colours::dimgrey : Colour(0u, 172u, 1u);
        
        g.setColour(color);
        
        auto bounds = toggleButton.getLocalBounds();
        g.drawRect(bounds);
        
        g.strokePath(analyzerButton->randomPath, PathStrokeType(1.f));
    }
    
    else
    {
        auto bounds = toggleButton.getLocalBounds().reduced(2);
        
        auto buttonIsOn = toggleButton.getToggleState();
        
        const int cornerSize = 4;
        
        g.setColour(buttonIsOn ? juce::Colours::white : juce::Colours::black);
        g.fillRoundedRectangle(bounds.toFloat(), cornerSize);
        
        g.setColour(buttonIsOn ? juce::Colours::black : juce::Colours::white);
        g.drawRoundedRectangle(bounds.toFloat(), cornerSize, 1);
        
        g.drawFittedText(toggleButton.getName(), bounds, juce::Justification::centred, 1);
        
    }
}

//==============================================================================
void RotarySliderWithLabels::paint(juce::Graphics &g)
{
    using namespace juce;
    
    auto startAng = degreesToRadians(180.f + 45.f);
    auto endAng = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi;
    
    auto range = getRange();
    
    auto sliderBounds = getSliderBounds();
    
    auto bounds = getLocalBounds();
    
    g.setColour(Colours::blueviolet);
    g.drawFittedText(getName(),
                     bounds.removeFromTop(getTextHeight() + 2),
                     Justification::centred,
                     1);
    
//    g.setColour(Colours::red);
//    g.drawRect(getLocalBounds());
//    g.setColour(Colours::yellow);
//    g.drawRect(sliderBounds);
    
    getLookAndFeel().drawRotarySlider(g,
                                      sliderBounds.getX(),
                                      sliderBounds.getY(),
                                      sliderBounds.getWidth(),
                                      sliderBounds.getHeight(),
                                      jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0),
                                      startAng,
                                      endAng,
                                      *this);
    
    auto center = sliderBounds.toFloat().getCentre();
    auto radius = sliderBounds.getWidth() * 0.5f;
    
    g.setColour(Colour(0u, 172u, 1u));
    g.setFont(getTextHeight());
    
    auto numChoices = labels.size();
    for( int i = 0; i < numChoices; ++i )
    {
        auto pos = labels[i].pos;
        jassert(0.f <= pos);
        jassert(pos <= 1.f);
        
        auto ang = jmap(pos, 0.f, 1.f, startAng, endAng);
        
        auto c = center.getPointOnCircumference(radius + getTextHeight() * 0.5f + 1, ang);
        
        Rectangle<float> r;
        auto str = labels[i].label;
        r.setSize(g.getCurrentFont().getStringWidth(str), getTextHeight());
        r.setCentre(c);
        r.setY(r.getY() + getTextHeight());
        
        g.drawFittedText(str, r.toNearestInt(), juce::Justification::centred, 1);
    }
    
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const
{
    auto bounds = getLocalBounds();
    
    bounds.removeFromTop(getTextHeight()*1.5);
    
    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());
    
    size -= getTextHeight() * 1.5;
    juce::Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), 0);
    r.setY(bounds.getY());
    
    return r;
    
}

juce::String RotarySliderWithLabels::getDisplayString() const
{
    if( auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param) )
        return choiceParam->getCurrentChoiceName();
    
    juce::String str;
    bool addK = false;
    
    if( auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param) )
    {
        float val = getValue();
        
        addK = truncateKiloValue(val);
        
        str = juce::String(val, (addK ? 2 : 0));
    }
    else
    {
        jassertfalse; //this shouldn't happen!
    }
    
    if( suffix.isNotEmpty() )
    {
        str << " ";
        if( addK )
            str << "k";
        
        str << suffix;
    }
    
    return str;
}

void RotarySliderWithLabels::changeParameter(juce::RangedAudioParameter *p)
{
    param = p;
    repaint();
};
//==============================================================================

juce::String RatioSlider::getDisplayString() const
{
    auto choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param);
    jassert(choiceParam != nullptr);
    
    auto currentChoice = choiceParam->getCurrentChoiceName();
    if (currentChoice.contains(".0"))
        currentChoice = currentChoice.substring(0, currentChoice.indexOf("."));
    
    currentChoice << ":1";
    
    return currentChoice;
};

//==============================================================================
CompressorBandControls::CompressorBandControls(juce::AudioProcessorValueTreeState& apvts) :
apvts(apvts),
attackSlider(nullptr, "ms", "ATTACK"),
releaseSlider(nullptr, "ms", "RELEASE"),
thresholdSlider(nullptr, "dB", "THRESHOLD"),
ratioSlider(nullptr, "")
{
    addAndMakeVisible(attackSlider);
    addAndMakeVisible(releaseSlider);
    addAndMakeVisible(thresholdSlider);
    addAndMakeVisible(ratioSlider);
    
    bypassButton.setName("X");
    soloButton.setName("S");
    muteButton.setName("M");
    
    addAndMakeVisible(bypassButton);
    addAndMakeVisible(soloButton);
    addAndMakeVisible(muteButton);
    
    lowBandButton.setName(" LOW ");
    midBandButton.setName(" MID ");
    highBandButton.setName(" HIGH ");
    
    lowBandButton.setRadioGroupId(1);
    midBandButton.setRadioGroupId(1);
    highBandButton.setRadioGroupId(1);

    addAndMakeVisible(lowBandButton);
    addAndMakeVisible(midBandButton);
    addAndMakeVisible(highBandButton);
    
    auto getParameterHelper = [&apvts= this->apvts](const auto& param) -> auto&
    {
        return getParam(apvts, param);
    };
    
    attackSlider.changeParameter(&getParameterHelper(SimpleMBCompAudioProcessor::ATTACK_MID_BAND_ID));
    releaseSlider.changeParameter(&getParameterHelper(SimpleMBCompAudioProcessor::RELEASE_MID_BAND_ID));
    thresholdSlider.changeParameter(&getParameterHelper(SimpleMBCompAudioProcessor::THRESHOLD_MID_BAND_ID));
    ratioSlider.changeParameter(&getParameterHelper(SimpleMBCompAudioProcessor::RATIO_MID_BAND_ID));
    
    addLabelPairs(attackSlider.labels, getParameterHelper(SimpleMBCompAudioProcessor::ATTACK_MID_BAND_ID), "ms");
    addLabelPairs(releaseSlider.labels, getParameterHelper(SimpleMBCompAudioProcessor::RELEASE_MID_BAND_ID), "ms");
    addLabelPairs(thresholdSlider.labels, getParameterHelper(SimpleMBCompAudioProcessor::THRESHOLD_MID_BAND_ID), "dB");
    
    ratioSlider.labels.add({0.f,"1:1"});
    //ratioSlider.labels.add({1.f,"100:1"});
    
    auto ratioParam = dynamic_cast<juce::AudioParameterChoice*>(&getParameterHelper(SimpleMBCompAudioProcessor::RATIO_MID_BAND_ID));
    
    ratioSlider.labels.add({1.0f,
                    juce::String(ratioParam->choices.getReference(ratioParam->choices.size()-1).getIntValue()) + ":1"});
    
    auto makeAttachmentHelper = [&apvts = this->apvts](auto& attachment, const auto& param, auto& slider)
    {
        makeAttachment(attachment, apvts, param, slider);
    };
    
    makeAttachmentHelper(attackSliderAttachment, SimpleMBCompAudioProcessor::ATTACK_MID_BAND_ID, attackSlider);
    makeAttachmentHelper(releaseSliderAttachment, SimpleMBCompAudioProcessor::RELEASE_MID_BAND_ID, releaseSlider);
    makeAttachmentHelper(thresholdSliderAttachment,   SimpleMBCompAudioProcessor::THRESHOLD_MID_BAND_ID, thresholdSlider);
    makeAttachmentHelper(ratioSliderAttachment, SimpleMBCompAudioProcessor::RATIO_MID_BAND_ID, ratioSlider);
    
    makeAttachmentHelper(bypassButtonAttachment, SimpleMBCompAudioProcessor::BYPASS_MID_BAND_ID, bypassButton);
    makeAttachmentHelper(soloButtonAttachment, SimpleMBCompAudioProcessor::SOLO_MID_BAND_ID, soloButton);
    makeAttachmentHelper(muteButtonAttachment, SimpleMBCompAudioProcessor::MUTE_MID_BAND_ID, muteButton);
}

void CompressorBandControls::resized()
{
    auto bounds = getLocalBounds().reduced(5);
    using namespace juce;
    
    auto createBandButtonControlBox = [](std::vector<Component*> comps){
        FlexBox flexBox;
        flexBox.flexDirection = FlexBox::Direction::column;
        flexBox.flexWrap = FlexBox::Wrap::noWrap;
        
        auto spacer = FlexItem().withHeight(2);
        
        for( auto* comp : comps )
        {
            flexBox.items.add(spacer);
            flexBox.items.add(FlexItem(*comp).withFlex(1.f));
            flexBox.items.add(spacer);
        }
        
        return flexBox;
    };
    
    auto bandButtonControlBox = createBandButtonControlBox({&bypassButton, &soloButton, &muteButton});
    auto bandSelectControlBox = createBandButtonControlBox({&lowBandButton, &midBandButton, &highBandButton});
    
    FlexBox flexBox;
    flexBox.flexDirection = FlexBox::Direction::row;
    flexBox.flexWrap = FlexBox::Wrap::noWrap;
    
    auto spacer = FlexItem().withWidth(4);
    auto endCap = FlexItem().withWidth(6);
    
    flexBox.items.add(endCap);
    flexBox.items.add(FlexItem(bandSelectControlBox).withWidth(44.f));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(attackSlider).withFlex(1.f));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(releaseSlider).withFlex(1.f));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(thresholdSlider).withFlex(1.f));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(ratioSlider).withFlex(1.f));
    //flexBox.items.add(endCap);
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(bandButtonControlBox).withWidth(30.f));
    flexBox.items.add(endCap);
    
    flexBox.performLayout(bounds);
};

void drawModuleBackground(juce::Graphics& g,
                          juce::Rectangle<int> bounds)
{
    using namespace juce;
    
    g.setColour(Colours::blueviolet);
    g.fillAll();
    
    auto localBounds = bounds;
    
    bounds.reduce(3, 3);
    g.setColour(Colours::black);
    g.fillRoundedRectangle(bounds.toFloat(), 3.f);
    
    g.drawRect(localBounds);
};

void CompressorBandControls::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    drawModuleBackground(g, bounds);
};

//==============================================================================
GlobalControls::GlobalControls(juce::AudioProcessorValueTreeState& apvts)
{
    
    auto getParameterHelper = [&apvts](const auto& param) -> auto&
    {
        return getParam(apvts, param);
    };
    
    auto& gainInParam = getParameterHelper(SimpleMBCompAudioProcessor::GAIN_IN_ID);
    auto& gainOutParam = getParameterHelper(SimpleMBCompAudioProcessor::GAIN_OUT_ID);
    auto& lowMidParam = getParameterHelper(SimpleMBCompAudioProcessor::LOW_MID_CROSSOVER_FREQ_ID);
    auto& midHighParam = getParameterHelper(SimpleMBCompAudioProcessor::MID_HIGH_CROSSOVER_FREQ_ID);
    
    inGainSlider = std::make_unique<RotarySliderWithLabels>(&gainInParam, "dB", "INPUT TRIM");
    outGainSlider = std::make_unique<RotarySliderWithLabels>(&gainOutParam, "dB", "OUTPUT TRIM");
    lowMidXoverSlider = std::make_unique<RotarySliderWithLabels>(&lowMidParam, "Hz", "LOW-MID X-OVER");
    midHighXoverSlider = std::make_unique<RotarySliderWithLabels>(&midHighParam, "Hz", "MID_HIGH X-OVER");
    
    auto makeAttachmentHelper = [&apvts](auto& attachment, const auto& param, auto& slider)
    {
        makeAttachment(attachment, apvts, param, slider);
    };
    
    addLabelPairs(inGainSlider->labels, gainInParam, "dB");
    addLabelPairs(lowMidXoverSlider->labels, lowMidParam, "Hz");
    addLabelPairs(midHighXoverSlider->labels, midHighParam, "Hz");
    addLabelPairs(outGainSlider->labels, gainOutParam, "dB");
    
    addAndMakeVisible(*inGainSlider);
    addAndMakeVisible(*lowMidXoverSlider);
    addAndMakeVisible(*midHighXoverSlider);
    addAndMakeVisible(*outGainSlider);
    
    makeAttachmentHelper(inGainSliderAttachment, SimpleMBCompAudioProcessor::GAIN_IN_ID, *inGainSlider);
    makeAttachmentHelper(outGainSliderAttachment, SimpleMBCompAudioProcessor::GAIN_OUT_ID, *outGainSlider);
    makeAttachmentHelper(lowMidXoverSliderAttachment,   SimpleMBCompAudioProcessor::LOW_MID_CROSSOVER_FREQ_ID, *lowMidXoverSlider);
    makeAttachmentHelper(midHighXoverSliderAttachment, SimpleMBCompAudioProcessor::MID_HIGH_CROSSOVER_FREQ_ID, *midHighXoverSlider);
};
void GlobalControls::paint(juce::Graphics& g)
{
    using namespace juce;
    auto bounds = getLocalBounds();
    
    drawModuleBackground(g, bounds);
};

void GlobalControls::resized()
{
    auto bounds = getLocalBounds().reduced(5);
    using namespace juce;
    
    FlexBox flexBox;
    flexBox.flexDirection = FlexBox::Direction::row;
    flexBox.flexWrap = FlexBox::Wrap::noWrap;
    
    auto spacer = FlexItem().withWidth(4);
    auto endCap = FlexItem().withWidth(6);
    
    flexBox.items.add(endCap);
    flexBox.items.add(FlexItem(*inGainSlider).withFlex(1.f));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(*lowMidXoverSlider).withFlex(1.f));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(*midHighXoverSlider).withFlex(1.f));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(*outGainSlider).withFlex(1.f));
    flexBox.items.add(endCap);
    
    flexBox.performLayout(bounds);
};
//==============================================================================
SimpleMBCompAudioProcessorEditor::SimpleMBCompAudioProcessorEditor (SimpleMBCompAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setLookAndFeel(&lnf);
    
//    addAndMakeVisible(controlBar);
//    addAndMakeVisible(analyzer);
    addAndMakeVisible(globalControls);
    addAndMakeVisible(bandControls);
    
    setSize (600, 500);
}

SimpleMBCompAudioProcessorEditor::~SimpleMBCompAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void SimpleMBCompAudioProcessorEditor::paint (juce::Graphics& g)
{

    g.fillAll (juce::Colours::black);

    g.setColour (juce::Colours::white);

}

void SimpleMBCompAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    
    controlBar.setBounds(bounds.removeFromTop(32));
    bandControls.setBounds(bounds.removeFromBottom(135));
    
    analyzer.setBounds(bounds.removeFromTop(225));
    
    globalControls.setBounds(bounds);
    
}
