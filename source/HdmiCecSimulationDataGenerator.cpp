#include "HdmiCecSimulationDataGenerator.h"
#include "HdmiCecAnalyzerSettings.h"

#include "HdmiCecProtocol.h"

#include <AnalyzerHelpers.h>
#include <iostream> // TODO sacar
using namespace std; // TODO sacar

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

    // Initialize clock at the recomended sampling rate
    mClockGenerator.Init( HdmiCec::MinSampleRate, mSimulationSampleRateHz );

    mCecSimulationData.SetChannel( mSettings->mCecChannel );
    mCecSimulationData.SetSampleRate( simulation_sample_rate );
    mCecSimulationData.SetInitialBitState( BIT_HIGH );
    // Advance a bit in HIGH
    mCecSimulationData.Advance( mClockGenerator.AdvanceByHalfPeriod( 10 ) );
}

U32 HdmiCecSimulationDataGenerator::GenerateSimulationData( U64 largest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channel )
{
    const U64 lastSample = AnalyzerHelpers::AdjustSimulationTargetSample( largest_sample_requested, sample_rate, mSimulationSampleRateHz );

    while( mCecSimulationData.GetCurrentSampleNumber() < lastSample )
    {
        GenerateStartSeqFrame();

        GenerateHeaderFrame(0x0, 0xF); // a message from the TV to broadcast TODO use addr enum
        Advance(10.0f); // Add some delay
        GenerateDataFrame(0x9E, false, true); // ask for CEC version TODO use opcode
        Advance(10.0f); // Add some delay
        GenerateDataFrame(0x4, true, true); // answer with 4 (cec 1.3a)
        Advance(15.0f); // Add some delay
    }

    *simulation_channel = &mCecSimulationData;
    return 1; // We are simulating one channel
}

void HdmiCecSimulationDataGenerator::GenerateStartSeqFrame()
{
    // The bus must be in high
    mCecSimulationData.TransitionIfNeeded( BIT_HIGH );

    // Timing values from CEC 1.3a section 5.2.1 "Start Bit Timing"
    mCecSimulationData.Transition(); // HIGH to LOW
    Advance( 3.7f );

    mCecSimulationData.Transition(); // LOW to HIGH
    Advance( 0.8f );
}

void HdmiCecSimulationDataGenerator::GenerateHeaderFrame( U8 src, U8 dst )
{
    const U8 data = (src << 4) | (dst & 0xF);
    GenerateDataFrame( data, false, true ); // TODO ver ack y eom
}

void HdmiCecSimulationDataGenerator::GenerateDataFrame( U8 data, bool eom, bool ack )
{
    for(int i=7; i>=0; i--)
        GenerateBit( (data >> i) & 0x1 );

    GenerateBit( eom );
    GenerateBit( ack );
}

void HdmiCecSimulationDataGenerator::GenerateBit( bool value )
{
    // Timing values from CEC 1.3a section 5.2.2 "Data Bit Timing"
    const float risingTime= value ? 0.6f : 1.5f;
    const float totalTime= 2.4f;

    // We should be in low
    mCecSimulationData.TransitionIfNeeded( BIT_LOW );

    Advance( risingTime) ;
    mCecSimulationData.Transition(); // LOW to HIGH

    Advance( totalTime - risingTime );
}


void HdmiCecSimulationDataGenerator::Advance(float msecs)
{
    mCecSimulationData.Advance( mClockGenerator.AdvanceByTimeS( msecs / 1000.0 ) );
}
