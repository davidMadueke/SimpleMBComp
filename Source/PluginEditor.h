/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
juce::String getValString(const juce::RangedAudioParameter& param, bool getLow, juce::String suffix);

//==============================================================================

struct Placeholder : juce::Component
{
    Placeholder()
    {
        juce::Random r;
        customColor = juce::Colour(r.nextInt(255), r.nextInt(255), r.nextInt(255));
    };
    
    void paint(juce::Graphics& g) override
    {
        g.fillAll(customColor);
    };
    
    juce::Colour customColor;
};

struct LookAndFeel : juce::LookAndFeel_V4
{
    void drawRotarySlider (juce::Graphics&,
                           int x, int y, int width, int height,
                           float sliderPosProportional,
                           float rotaryStartAngle,
                           float rotaryEndAngle,
                           juce::Slider&) override;
    
    void drawToggleButton (juce::Graphics &g,
                           juce::ToggleButton & toggleButton,
                           bool shouldDrawButtonAsHighlighted,
                           bool shouldDrawButtonAsDown) override;
};
//==============================================================================

struct RotarySliderWithLabels : juce::Slider
{
    RotarySliderWithLabels(juce::RangedAudioParameter* rap,
                           const juce::String& unitSuffix,
                           const juce::String& title = "NO TITLE") :
    juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                 juce::Slider::TextEntryBoxPosition::NoTextBox),
    param(rap),
    suffix(unitSuffix)
    {
        setName(title);
//        setLookAndFeel(&lnf);
    }
    
//    ~RotarySliderWithLabels()
//    {
//        setLookAndFeel(nullptr);
//    }
    
    struct LabelPos
    {
        float pos;
        juce::String label;
    };
    
    juce::Array<LabelPos> labels;
    
    void paint(juce::Graphics& g) override;
    juce::Rectangle<int> getSliderBounds() const;
    int getTextHeight() const { return 14; }
    virtual juce::String getDisplayString() const;
    
    void changeParameter(juce::RangedAudioParameter* p);
    
protected:
//    LookAndFeel lnf;
    
    juce::RangedAudioParameter* param;
    juce::String suffix;
};
//==============================================================================
struct RatioSlider : RotarySliderWithLabels
{
    RatioSlider(juce::RangedAudioParameter* rap,
                           const juce::String& unitSuffix) :
    RotarySliderWithLabels(rap, unitSuffix, "RATIO"){}
    
    juce::String getDisplayString() const override;
};
//==============================================================================

struct PowerButton : juce::ToggleButton { };

struct AnalyzerButton : juce::ToggleButton
{
    void resized() override
    {
        auto bounds = getLocalBounds();
        auto insetRect = bounds.reduced(4);
        
        randomPath.clear();
        
        juce::Random r;
        
        randomPath.startNewSubPath(insetRect.getX(),
                                   insetRect.getY() + insetRect.getHeight() * r.nextFloat());
        
        for( auto x = insetRect.getX() + 1; x < insetRect.getRight(); x += 2 )
        {
            randomPath.lineTo(x,
                              insetRect.getY() + insetRect.getHeight() * r.nextFloat());
        }
    }
    
    juce::Path randomPath;
};
//==============================================================================

struct RotarySlider : juce::Slider
{
    RotarySlider() : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox )
    {};
};
//==============================================================================
template<
typename Attachment,
typename APVTS,
typename ParamID,
typename SliderType
>
void makeAttachment(std::unique_ptr<Attachment>& attachment, APVTS& apvts, const ParamID& param, SliderType& slider )
{
    attachment = std::make_unique<Attachment>(apvts,param.getParamID(), slider);
};

template<
typename APVTS,
typename ParamID>
juce::RangedAudioParameter& getParam(APVTS& apvts, const ParamID& param)
{
    auto parameter = apvts.getParameter(param.getParamID());
    jassert( parameter != nullptr);
    
    return *parameter;
};

template<
typename Labels,
typename ParamType,
typename SuffixType
>
void addLabelPairs(Labels& labels, const ParamType& param, const SuffixType& suffix)
{
    labels.clear();
    labels.add({0.f,getValString(param, true, suffix)});
    labels.add({1.f,getValString(param, false, suffix)});
};
//==============================================================================

struct CompressorBandControls : juce::Component
{
    CompressorBandControls(juce::AudioProcessorValueTreeState& apvts);
    
    void paint(juce::Graphics& g) override;
    
    void resized() override;
private:
    juce::AudioProcessorValueTreeState& apvts;
    
    RotarySliderWithLabels attackSlider, releaseSlider, thresholdSlider;
    RatioSlider ratioSlider;
    
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> attackSliderAttachment,
                                releaseSliderAttachment,
                                thresholdSliderAttachment,
                                ratioSliderAttachment;
};

//==============================================================================
struct GlobalControls : juce::Component
{
    GlobalControls(juce::AudioProcessorValueTreeState& apvts);
    
    void paint(juce::Graphics& g) override;
    
    void resized() override;
    
private:
    using RSWL = RotarySliderWithLabels;
    std::unique_ptr<RSWL> inGainSlider, outGainSlider, lowMidXoverSlider, midHighXoverSlider;
    
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> lowMidXoverSliderAttachment,
                                midHighXoverSliderAttachment,
                                inGainSliderAttachment,
                                outGainSliderAttachment;
};
//==============================================================================

class SimpleMBCompAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    SimpleMBCompAudioProcessorEditor (SimpleMBCompAudioProcessor&);
    ~SimpleMBCompAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    LookAndFeel lnf;
    SimpleMBCompAudioProcessor& audioProcessor;
    
    Placeholder controlBar, analyzer /*globalControls,*/ /*bandControls*/;
    
    GlobalControls globalControls { audioProcessor.apvts };
    
    CompressorBandControls bandControls { audioProcessor.apvts };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleMBCompAudioProcessorEditor)
};
