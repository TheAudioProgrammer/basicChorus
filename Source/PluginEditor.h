/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Assets.h"

class TapImage : public juce::ImageComponent
{
public:
    TapImage()
    {
        tapImage = juce::ImageCache::getFromMemory (Assets::Lockup_3_Curves_png, Assets::Lockup_3_Curves_pngSize);
        jassert (tapImage.isValid());
        setImage (tapImage, juce::RectanglePlacement::stretchToFit);
        addAndMakeVisible (websiteButton);
    }
    
    void resized() override
    {
        websiteButton.setBounds (getLocalBounds());
    }
    
private:
    juce::Image tapImage;
    juce::HyperlinkButton websiteButton { "", juce::URL ("https://theaudioprogrammer.com") };
};

//==============================================================================
/**
*/
class BasicChorusAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    BasicChorusAudioProcessorEditor (BasicChorusAudioProcessor&);
    ~BasicChorusAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    
    juce::Slider rateSlider;
    juce::Slider depthSlider;
    juce::Slider centreDelaySlider;
    juce:: Slider feedbackSlider;
    juce::Slider mixSlider;
    
    juce::Label rateLabel     { "Rate", "Rate" };
    juce::Label depthLabel    { "Depth", "Depth" };
    juce::Label delayLabel    { "Delay" , "Delay" };
    juce::Label feedbackLabel { "Feedback", "Feedback" };
    juce::Label mixLabel      { "Mix", "Mix" };
    juce::Label pluginTitle   { "Plug-in Title", "Chorus" };
    
    using Attachment = std::unique_ptr<juce::SliderParameterAttachment>;
    
    Attachment rateSliderAttachment;
    Attachment depthSliderAttachment;
    Attachment centreDelaySliderAttachment;
    Attachment feedbackSliderAttachment;
    Attachment mixSliderAttachment;
    
    TapImage tapImage;
    juce::Font currentFont;
        
    BasicChorusAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BasicChorusAudioProcessorEditor)
};
