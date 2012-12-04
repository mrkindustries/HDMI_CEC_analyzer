#include "HdmiCecSimulationDataGenerator.h"
#include "HdmiCecAnalyzerSettings.h"

#include "HdmiCecProtocol.h"

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

    // Initialize clock at the recomended sampling rate
    mClockGenerator.Init(HdmiCec::MinSampleRate, sample_rate);


    while( mCecSimulationData.GetCurrentSampleNumber() < adjusted_largest_sample_requested )
    {
//        CreateSerialByte();
    }

    *simulation_channel = &mCecSimulationData;
    return 1;
}
