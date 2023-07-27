#pragma once

/*
 The DSP roadmap:
 1) Split the incoming audio into three bands
 2) Create parameters for crossover frequencies
 3) Process each band using Compressor Band struct
 4) Add Mute, Solo and Bypass Functionality for each band
 5) Recombine the three signals into one
 */

#include <JuceHeader.h>

//==============================================================================
/**
*/



//Define a compressor Band struct

struct CompressorBand
{
    
    juce::AudioParameterFloat* threshold { nullptr };
    juce::AudioParameterFloat* attack { nullptr };
    juce::AudioParameterFloat* release { nullptr };
    juce::AudioParameterChoice* ratio { nullptr };
    juce::AudioParameterBool* bypass { nullptr };
    
    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        compressor.prepare(spec);
    };
    
    void updateCompressorSettings()
    {
        compressor.setAttack(attack->get());
        compressor.setRelease(release->get());
        compressor.setThreshold(threshold->get());
        compressor.setRatio(ratio->getCurrentChoiceName().getFloatValue());
    };
    
    void process(juce::AudioBuffer<float>& buffer)
    {
        auto block = juce::dsp::AudioBlock<float>(buffer);
        auto context = juce::dsp::ProcessContextReplacing<float>(block);
        
        context.isBypassed = bypass->get();
        compressor.process(context);

    };
private:
    juce::dsp::Compressor<float> compressor;
    
    
};
class SimpleMBCompAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    inline static const juce::String
    
            LOW_MID_CROSSOVER_FREQ_NAME = "Low-Mid Crossover Freq",
            MID_HIGH_CROSSOVER_FREQ_NAME = "Mid-High Crossover Freq",
            
            THRESHOLD_LOW_BAND_NAME = "Threshold Low Band",
            THRESHOLD_MID_BAND_NAME = "Threshold Mid Band",
            THRESHOLD_HIGH_BAND_NAME = "Threshold High Band",
            
            ATTACK_LOW_BAND_NAME = "Attack Low Band",
            ATTACK_MID_BAND_NAME = "Attack Mid Band",
            ATTACK_HIGH_BAND_NAME = "Attack High Band",
            
            RELEASE_LOW_BAND_NAME = "Release Low Band",
            RELEASE_MID_BAND_NAME = "Release Mid Band",
            RELEASE_HIGH_BAND_NAME = "Release High Band",
            
            RATIO_LOW_BAND_NAME = "Ratio Low Band",
            RATIO_MID_BAND_NAME = "Ratio Mid Band",
            RATIO_HIGH_BAND_NAME = "Ratio High Band",
            
            BYPASS_LOW_BAND_NAME = "Bypass Low Band",
            BYPASS_MID_BAND_NAME = "Bypass Mid Band",
            BYPASS_HIGH_BAND_NAME = "Bypass High Band"
    ;


    inline static const juce::ParameterID
            LOW_MID_CROSSOVER_FREQ_ID = {"Low_Mid_Crossover_Freq",1},
            MID_HIGH_CROSSOVER_FREQ_ID = {"Mid-High Crossover Freq",1},
            
            THRESHOLD_LOW_BAND_ID ={"Threshold_Low_Band",1},
            THRESHOLD_MID_BAND_ID= {"Threshold_Mid_Band",1},
            THRESHOLD_HIGH_BAND_ID= {"Threshold_High_Band",1},
            
            ATTACK_LOW_BAND_ID= {"Attack_Low_Band", 1},
            ATTACK_MID_BAND_ID= {"Attack_Mid_Band",1},
            ATTACK_HIGH_BAND_ID= {"Attack_High_Band",1},
            
            RELEASE_LOW_BAND_ID= {"Release_Low_Band",1},
            RELEASE_MID_BAND_ID= {"Release_Mid_Band",1},
            RELEASE_HIGH_BAND_ID= {"Release_High_Band", 1},
            
            RATIO_LOW_BAND_ID= {"Ratio_Low_Band", 1},
            RATIO_MID_BAND_ID= {"Ratio_Mid_Band", 1},
            RATIO_HIGH_BAND_ID= {"Ratio_High_Band", 1},
            
            BYPASS_LOW_BAND_ID= {"Bypass_Low_Band", 1},
            BYPASS_MID_BAND_ID= {"Bypass_Mid_Band",1},
            BYPASS_HIGH_BAND_ID= {"Bypass_High_Band", 1}
       ;
    //==============================================================================
    SimpleMBCompAudioProcessor();
    ~SimpleMBCompAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    
    juce::AudioProcessorValueTreeState apvts {*this, nullptr, "Parameters", createParameterLayout()};
private:
    //inline static juce::StringArray parameterList;
    
    CompressorBand compressorband;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleMBCompAudioProcessor)
};
