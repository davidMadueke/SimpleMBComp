/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleMBCompAudioProcessor::SimpleMBCompAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), apvts(*this, nullptr, "Parameters", createParameterLayout())
#endif
{
    
    //floatHelper lambda function
    auto floatHelper = [&apvts = this->apvts](auto& parameter, const auto& parameterName)
    {
        parameter = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(parameterName));
        jassert(parameter != nullptr);
    };

    //choiceHelper Lambda
    auto choiceHelper = [&apvts = this->apvts](auto& parameter, const auto& parameterName)
    {
       parameter = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(parameterName));
        jassert(parameter != nullptr);
    };

    //boolHelper Lambda
    auto boolHelper = [&apvts = this->apvts](auto& parameter, const auto& parameterName)
    {
        parameter = dynamic_cast<juce::AudioParameterBool*>(apvts.getParameter(parameterName));
        jassert(parameter != nullptr);
    };

    // Using the parameterList StringArray, apply the float helper functions
    
    // Input and output Gain
    floatHelper(inputGainParam, SimpleMBCompAudioProcessor::GAIN_IN_ID.getParamID());
    floatHelper(outputGainParam, SimpleMBCompAudioProcessor::GAIN_OUT_ID.getParamID());
    
    //Crossover Frequencies
    floatHelper(lowMidCrossover, SimpleMBCompAudioProcessor::LOW_MID_CROSSOVER_FREQ_ID.getParamID() );
    floatHelper(midHighCrossover, SimpleMBCompAudioProcessor::MID_HIGH_CROSSOVER_FREQ_ID.getParamID() );
    
    LP1.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    AP2.setType(juce::dsp::LinkwitzRileyFilterType::allpass);
    
    HP1.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
    LP2.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    HP2.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
    
    //Compressor Low Band
    floatHelper(lowBandComp.attack, SimpleMBCompAudioProcessor::ATTACK_LOW_BAND_ID.getParamID() );
    floatHelper(lowBandComp.release, SimpleMBCompAudioProcessor::RELEASE_LOW_BAND_ID.getParamID() );
    floatHelper(lowBandComp.threshold, SimpleMBCompAudioProcessor::THRESHOLD_LOW_BAND_ID.getParamID() );
    
    choiceHelper(lowBandComp.ratio, SimpleMBCompAudioProcessor::RATIO_LOW_BAND_ID.getParamID() );
    
    boolHelper(lowBandComp.bypass, SimpleMBCompAudioProcessor::BYPASS_LOW_BAND_ID.getParamID() );
    boolHelper(lowBandComp.solo, SimpleMBCompAudioProcessor::SOLO_LOW_BAND_ID.getParamID() );
    boolHelper(lowBandComp.mute, SimpleMBCompAudioProcessor::MUTE_LOW_BAND_ID.getParamID() );
    
    //Compressor mid Band
    floatHelper(midBandComp.attack, SimpleMBCompAudioProcessor::ATTACK_MID_BAND_ID.getParamID() );
    floatHelper(midBandComp.release, SimpleMBCompAudioProcessor::RELEASE_MID_BAND_ID.getParamID() );
    floatHelper(midBandComp.threshold, SimpleMBCompAudioProcessor::THRESHOLD_MID_BAND_ID.getParamID() );
    
    choiceHelper(midBandComp.ratio, SimpleMBCompAudioProcessor::RATIO_MID_BAND_ID.getParamID() );
    
    boolHelper(midBandComp.bypass, SimpleMBCompAudioProcessor::BYPASS_MID_BAND_ID.getParamID() );
    boolHelper(midBandComp.solo, SimpleMBCompAudioProcessor::SOLO_MID_BAND_ID.getParamID() );
    boolHelper(midBandComp.mute, SimpleMBCompAudioProcessor::MUTE_MID_BAND_ID.getParamID() );
    
    //Compressor high Band
    floatHelper(highBandComp.attack, SimpleMBCompAudioProcessor::ATTACK_HIGH_BAND_ID.getParamID() );
    floatHelper(highBandComp.release, SimpleMBCompAudioProcessor::RELEASE_HIGH_BAND_ID.getParamID() );
    floatHelper(highBandComp.threshold, SimpleMBCompAudioProcessor::THRESHOLD_HIGH_BAND_ID.getParamID() );
    
    choiceHelper(highBandComp.ratio, SimpleMBCompAudioProcessor::RATIO_HIGH_BAND_ID.getParamID() );
    
    boolHelper(highBandComp.bypass, SimpleMBCompAudioProcessor::BYPASS_HIGH_BAND_ID.getParamID() );
    boolHelper(highBandComp.solo, SimpleMBCompAudioProcessor::SOLO_HIGH_BAND_ID.getParamID() );
    boolHelper(highBandComp.mute, SimpleMBCompAudioProcessor::MUTE_HIGH_BAND_ID.getParamID() );
    


}

SimpleMBCompAudioProcessor::~SimpleMBCompAudioProcessor()
{
}

//==============================================================================
const juce::String SimpleMBCompAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SimpleMBCompAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SimpleMBCompAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SimpleMBCompAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SimpleMBCompAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SimpleMBCompAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SimpleMBCompAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SimpleMBCompAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SimpleMBCompAudioProcessor::getProgramName (int index)
{
    return {};
}

void SimpleMBCompAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SimpleMBCompAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    spec.sampleRate = sampleRate;
    
    inputGain.prepare(spec);
    outputGain.prepare(spec);
    
    inputGain.setRampDurationSeconds(0.05); //50 ms
    outputGain.setRampDurationSeconds(0.05); //50 ms
    
    for (auto& comp : compressorbands)
        comp.prepare(spec);
    
    LP1.prepare(spec);
    HP1.prepare(spec);
    LP2.prepare(spec);
    HP2.prepare(spec);
    AP2.prepare(spec);
    
    for( auto& buffer : filterBuffers)
    {
        buffer.setSize(spec.numChannels, samplesPerBlock);
    };
}

void SimpleMBCompAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SimpleMBCompAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void SimpleMBCompAudioProcessor::updateState()
{
    inputGain.setGainDecibels(inputGainParam->get() );
    outputGain.setGainDecibels(outputGainParam->get());
    
    for(auto& comp : compressorbands)
    {
        comp.updateCompressorSettings();
        
    };
    
    auto lowMidCutoff = lowMidCrossover->get();
    auto midHighCutoff = midHighCrossover->get();
    
    LP1.setCutoffFrequency(lowMidCutoff);
    AP2.setCutoffFrequency(midHighCutoff);
    
    HP1.setCutoffFrequency(lowMidCutoff);
    LP2.setCutoffFrequency(midHighCutoff);
    HP2.setCutoffFrequency(midHighCutoff);
};

void SimpleMBCompAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    updateState();
    
    processGain(buffer, inputGain);
    
    
    for(auto& fb : filterBuffers )
    {
        fb = buffer;
        
    }
    
    auto fb0Block = juce::dsp::AudioBlock<float>(filterBuffers[0]);
    auto fb1Block = juce::dsp::AudioBlock<float>(filterBuffers[1]);
    auto fb2Block = juce::dsp::AudioBlock<float>(filterBuffers[2]);
    
    auto fb0Ctx = juce::dsp::ProcessContextReplacing<float>(fb0Block);
    auto fb1Ctx = juce::dsp::ProcessContextReplacing<float>(fb1Block);
    auto fb2Ctx = juce::dsp::ProcessContextReplacing<float>(fb2Block);
    
    LP1.process(fb0Ctx);
    AP2.process(fb0Ctx);
    
    HP1.process(fb1Ctx);
    filterBuffers[2] = filterBuffers[1];
    LP2.process(fb1Ctx);
    
    HP2.process(fb2Ctx);
    
    
    for( size_t i = 0; i < filterBuffers.size(); ++i)
    {
        if(compressorbands[i].bypass->get() == false )
            compressorbands[i].process(filterBuffers[i]);
    };
    
    
    auto numSamples = buffer.getNumSamples();
    auto numChannels = buffer.getNumChannels();
    

    
    buffer.clear();
    
    auto CompBandsAreSoloed = false;
    for(auto& comp: compressorbands)
    {
      if( comp.solo->get() )
      {
          CompBandsAreSoloed = true;
          break;
      };
        
    };
    
    
    auto addFilterBand = [nc = numChannels, ns = numSamples](auto& inputBuffer, const auto& source)
    {
        for(auto i = 0; i < nc ; ++i)
        {
            // buffer.addFrom(int destChannel, int destStartSample, source buffer, int sourceChannel, int sourceStartSample, int NumSamples)
            inputBuffer.addFrom(i, 0, source, i, 0, ns);
        };
    };
    
    if(CompBandsAreSoloed)
    {
        for(size_t i = 0; i < compressorbands.size() ; ++i )
        {
            auto& comp = compressorbands[i];
          if(comp.solo->get() )
              addFilterBand(buffer, filterBuffers[i]);
        };
    }
    else{
        for(size_t i = 0; i < compressorbands.size() ; ++i )
        {
            auto& comp = compressorbands[i];
          if(comp.mute->get() == false )
              addFilterBand(buffer, filterBuffers[i]);
        };
    }
    processGain(buffer, outputGain);
    
//    addFilterBand(buffer, filterBuffers[0]);
//    addFilterBand(buffer, filterBuffers[1]);
//    addFilterBand(buffer, filterBuffers[2]);
}

//==============================================================================
bool SimpleMBCompAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SimpleMBCompAudioProcessor::createEditor()
{
    //return new SimpleMBCompAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void SimpleMBCompAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    
    juce::MemoryOutputStream MemoryOutputStream(destData, true);
    apvts.state.writeToStream(MemoryOutputStream);
}

void SimpleMBCompAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if( tree.isValid() )
    {
        apvts.replaceState(tree);
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout SimpleMBCompAudioProcessor::createParameterLayout(){
//    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    

    
    // Create a std::vector with a ranged audio parameter template and add all of the unique pointers to it
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> vecParams;
    
    auto gainRange = juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f);
    
    juce::NormalisableRange<float> thresholdRange = juce::NormalisableRange<float>(-60, +12, 1, 1);
    
    auto attackReleaseRange =  juce::NormalisableRange<float>(5, 500, 1, 1); // Set the attack Release range to a minimal time of 5ms and maximal range of 500ms with a linear step with a skew factor 1

    auto ratioChoices = std::vector<double>{1, 1.5, 2, 3, 4, 5, 6, 7, 8, 10, 15, 20, 50, 100};
    juce::StringArray sa;
    for(auto choice: ratioChoices)
    {
        sa.add( juce::String(choice, 1) );
    };
    
    auto LOW_MID_crossoverFreqRange = juce::NormalisableRange<float>(20, 999, 1, 1);
    auto MID_HIGH_crossoverFreqRange = juce::NormalisableRange<float>(1000, 20000, 1, 1);
    
    // Input and Output Gain
    vecParams.push_back(std::make_unique<juce::AudioParameterFloat>(GAIN_IN_ID, GAIN_IN_NAME, gainRange, 0));
    vecParams.push_back(std::make_unique<juce::AudioParameterFloat>(GAIN_OUT_ID, GAIN_OUT_NAME, gainRange, 0));
    
    
    //Low Band Compressor
    vecParams.push_back(std::make_unique<juce::AudioParameterFloat>(THRESHOLD_LOW_BAND_ID, THRESHOLD_LOW_BAND_NAME, thresholdRange, 0));
    vecParams.push_back(std::make_unique<juce::AudioParameterFloat>(ATTACK_LOW_BAND_ID, ATTACK_LOW_BAND_NAME, attackReleaseRange, 0));
    vecParams.push_back(std::make_unique<juce::AudioParameterFloat>(RELEASE_LOW_BAND_ID, RELEASE_LOW_BAND_NAME, attackReleaseRange, 0));
    vecParams.push_back(std::make_unique<juce::AudioParameterChoice>(RATIO_LOW_BAND_ID, RATIO_LOW_BAND_NAME, sa, 3));
    vecParams.push_back(std::make_unique<juce::AudioParameterBool>( BYPASS_LOW_BAND_ID, BYPASS_LOW_BAND_NAME, false));
    
    vecParams.push_back(std::make_unique<juce::AudioParameterBool>( SOLO_LOW_BAND_ID, SOLO_LOW_BAND_NAME, false));
    vecParams.push_back(std::make_unique<juce::AudioParameterBool>( MUTE_LOW_BAND_ID, MUTE_LOW_BAND_NAME, false));
    
    // Mid Band Compressor
    vecParams.push_back(std::make_unique<juce::AudioParameterFloat>(THRESHOLD_MID_BAND_ID, THRESHOLD_MID_BAND_NAME, thresholdRange, 0));
    vecParams.push_back(std::make_unique<juce::AudioParameterFloat>(ATTACK_MID_BAND_ID, ATTACK_MID_BAND_NAME, attackReleaseRange, 0));
    vecParams.push_back(std::make_unique<juce::AudioParameterFloat>(RELEASE_MID_BAND_ID, RELEASE_MID_BAND_NAME, attackReleaseRange, 0));
    vecParams.push_back(std::make_unique<juce::AudioParameterChoice>(RATIO_MID_BAND_ID, RATIO_MID_BAND_NAME, sa, 3));
    vecParams.push_back(std::make_unique<juce::AudioParameterBool>( BYPASS_MID_BAND_ID, BYPASS_MID_BAND_NAME, false));
    
    vecParams.push_back(std::make_unique<juce::AudioParameterBool>( SOLO_MID_BAND_ID, SOLO_MID_BAND_NAME, false));
    vecParams.push_back(std::make_unique<juce::AudioParameterBool>( MUTE_MID_BAND_ID, MUTE_MID_BAND_NAME, false));
    
    // High Band Compressor
    vecParams.push_back(std::make_unique<juce::AudioParameterFloat>(THRESHOLD_HIGH_BAND_ID, THRESHOLD_HIGH_BAND_NAME, thresholdRange, 0));
    vecParams.push_back(std::make_unique<juce::AudioParameterFloat>(ATTACK_HIGH_BAND_ID, ATTACK_HIGH_BAND_NAME, attackReleaseRange, 0));
    vecParams.push_back(std::make_unique<juce::AudioParameterFloat>(RELEASE_HIGH_BAND_ID, RELEASE_HIGH_BAND_NAME, attackReleaseRange, 0));
    vecParams.push_back(std::make_unique<juce::AudioParameterChoice>(RATIO_HIGH_BAND_ID, RATIO_HIGH_BAND_NAME, sa, 3));
    
    vecParams.push_back(std::make_unique<juce::AudioParameterBool>( BYPASS_HIGH_BAND_ID, BYPASS_HIGH_BAND_NAME, false));
    vecParams.push_back(std::make_unique<juce::AudioParameterBool>( SOLO_HIGH_BAND_ID, SOLO_HIGH_BAND_NAME, false));
    vecParams.push_back(std::make_unique<juce::AudioParameterBool>( MUTE_HIGH_BAND_ID, MUTE_HIGH_BAND_NAME, false));
    
    //Crossover Freq Parameters
    vecParams.push_back(std::make_unique<juce::AudioParameterFloat>(LOW_MID_CROSSOVER_FREQ_ID, LOW_MID_CROSSOVER_FREQ_NAME, LOW_MID_crossoverFreqRange , 400 ));
    vecParams.push_back(std::make_unique<juce::AudioParameterFloat>(MID_HIGH_CROSSOVER_FREQ_ID, MID_HIGH_CROSSOVER_FREQ_NAME, MID_HIGH_crossoverFreqRange , 2000 ));
//
//    // Loop over this vector and add the resp. parameterIDs to the parameterlist stringArray
//    for (const auto& param : vecParams){
//        parameterList.add(param->getParameterID());
//    }


    return {vecParams.begin(), vecParams.end()};

}
//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleMBCompAudioProcessor();
}
