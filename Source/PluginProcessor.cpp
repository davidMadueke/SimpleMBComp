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

    //Crossover Frequencies
    floatHelper(lowCrossover, SimpleMBCompAudioProcessor::LOW_MID_CROSSOVER_FREQ_ID.getParamID() );
    
    LP.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    HP.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
    
    //Compressor Low Band
    floatHelper(compressorband.attack, SimpleMBCompAudioProcessor::ATTACK_LOW_BAND_ID.getParamID() );
    floatHelper(compressorband.release, SimpleMBCompAudioProcessor::RELEASE_LOW_BAND_ID.getParamID() );
    floatHelper(compressorband.threshold, SimpleMBCompAudioProcessor::THRESHOLD_LOW_BAND_ID.getParamID() );
    
    choiceHelper(compressorband.ratio, SimpleMBCompAudioProcessor::RATIO_LOW_BAND_ID.getParamID() );
    
    boolHelper(compressorband.bypass, SimpleMBCompAudioProcessor::BYPASS_LOW_BAND_ID.getParamID() );
    


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
    
    compressorband.prepare(spec);
    LP.prepare(spec);
    HP.prepare(spec);
    
    for( auto& buffer : filterBuffers)
    {
        buffer.setSize(spec.numChannels, samplesPerBlock);
    }
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
    
//    compressorband.updateCompressorSettings();
//    compressorband.process(buffer);
    
    for(auto& fb : filterBuffers )
    {
        fb = buffer;
        
    }
    
    auto cutoff = lowCrossover->get();
    LP.setCutoffFrequency(cutoff);
    HP.setCutoffFrequency(cutoff);
    
    auto fb0Block = juce::dsp::AudioBlock<float>(filterBuffers[0]);
    auto fb1Block = juce::dsp::AudioBlock<float>(filterBuffers[1]);
    
    auto fb0Ctx = juce::dsp::ProcessContextReplacing<float>(fb0Block);
    auto fb1Ctx = juce::dsp::ProcessContextReplacing<float>(fb1Block);
    
    LP.process(fb0Ctx);
    HP.process(fb1Ctx);
    
    auto numSamples = buffer.getNumSamples();
    auto numChannels = buffer.getNumChannels();
    
    buffer.clear();


    
    auto addFilterBand = [nc = numChannels, ns = numSamples](auto& inputBuffer, const auto& source)
    {
        for(auto i = 0; i < nc ; ++i)
        {
            // buffer.addFrom(int destChannel, int destStartSample, source buffer, int sourceChannel, int sourceStartSample, int NumSamples)
            inputBuffer.addFrom(i, 0, source, i, 0, ns);
        };
    };
    
    addFilterBand(buffer, filterBuffers[0]);
    addFilterBand(buffer, filterBuffers[1]);
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

    auto attackReleaseRange =  juce::NormalisableRange<float>(5, 500, 1, 1); // Set the attack Release range to a minimal time of 5ms and maximal range of 500ms with a linear step with a skew factor 1

    auto ratioChoices = std::vector<double>{1, 1.5, 2, 3, 4, 5, 6, 7, 8, 10, 15, 20, 50, 100};
    juce::StringArray sa;
    for(auto choice: ratioChoices)
    {
        sa.add( juce::String(choice, 1) );
    };
    
    auto crossoverFreqRange = juce::NormalisableRange<float>(20, 20000, 1, 1);

    vecParams.push_back(std::make_unique<juce::AudioParameterFloat>(THRESHOLD_LOW_BAND_ID, THRESHOLD_LOW_BAND_NAME, juce::NormalisableRange<float>(-60, +12, 1, 1), 0));
    vecParams.push_back(std::make_unique<juce::AudioParameterFloat>(ATTACK_LOW_BAND_ID, ATTACK_LOW_BAND_NAME, attackReleaseRange, 0));
    vecParams.push_back(std::make_unique<juce::AudioParameterFloat>(RELEASE_LOW_BAND_ID, RELEASE_LOW_BAND_NAME, attackReleaseRange, 0));
    vecParams.push_back(std::make_unique<juce::AudioParameterChoice>(RATIO_LOW_BAND_ID, RATIO_LOW_BAND_NAME, sa, 3));
    vecParams.push_back(std::make_unique<juce::AudioParameterBool>( BYPASS_LOW_BAND_ID, BYPASS_LOW_BAND_NAME, false));
    
    vecParams.push_back(std::make_unique<juce::AudioParameterFloat>(LOW_MID_CROSSOVER_FREQ_ID, LOW_MID_CROSSOVER_FREQ_NAME, crossoverFreqRange , 500 ));
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
