#include "HdmiCecSimulationDataGenerator.h"
#include "HdmiCecAnalyzerSettings.h"

#include <AnalyzerHelpers.h>

HdmiCecSimulationDataGenerator::HdmiCecSimulationDataGenerator()
{
}

HdmiCecSimulationDataGenerator::~HdmiCecSimulationDataGenerator()
{
}

void HdmiCecSimulationDataGenerator::Initialize( U32 simulation_sample_rate, HdmiCecAnalyzerSettings* settings )
{
    mSimulationSampleRateHz = simulation_sample_rate;
    mSettings = settings;

    mCecSimulationData.SetChannel( mSettings->mCecChannel );
    mCecSimulationData.SetSampleRate( simulation_sample_rate );
    mCecSimulationData.SetInitialBitState( BIT_HIGH );
}

U32 HdmiCecSimulationDataGenerator::GenerateSimulationData( U64 largest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channel )
{
    U64 adjusted_largest_sample_requested = AnalyzerHelpers::AdjustSimulationTargetSample( largest_sample_requested, sample_rate, mSimulationSampleRateHz );

    while( mCecSimulationData.GetCurrentSampleNumber() < adjusted_largest_sample_requested )
    {
        CreateSerialByte();
    }

    *simulation_channel = &mCecSimulationData;
    return 1;
}

void HdmiCecSimulationDataGenerator::CreateSerialByte()
{
    U32 samples_per_bit = mSimulationSampleRateHz / mSettings->mBitRate;

    U8 byte = mSerialText[ mStringIndex ];
    mStringIndex++;
    if( mStringIndex == mSerialText.size() )
        mStringIndex = 0;

    //we're currenty high
    //let's move forward a little
    mCecSimulationData.Advance( samples_per_bit * 10 );

    mCecSimulationData.Transition();  //low-going edge for start bit
    mCecSimulationData.Advance( samples_per_bit );  //add start bit time

    U8 mask = 0x1 << 7;
    for( U32 i=0; i<8; i++ )
    {
        if( ( byte & mask ) != 0 )
            mCecSimulationData.TransitionIfNeeded( BIT_HIGH );
        else
            mCecSimulationData.TransitionIfNeeded( BIT_LOW );

        mCecSimulationData.Advance( samples_per_bit );
        mask = mask >> 1;
    }

    mCecSimulationData.TransitionIfNeeded( BIT_HIGH ); //we need to end high

    //lets pad the end a bit for the stop bit:
    mCecSimulationData.Advance( samples_per_bit );
}
