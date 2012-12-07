#include "HdmiCecSimulationDataGenerator.h"
#include "HdmiCecAnalyzerSettings.h"

#include "HdmiCecProtocol.h"

#include <AnalyzerHelpers.h>
#include <iostream> // TODO sacar
using namespace std; // TODO sacar

#include <stdlib.h> // Used for rand()

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

    // Initialize the random number generator with a literal seed to obtain repeatability
    // Change this for srand(time(NULL)) for "truly" random sequences
    // NOTICE rand() an srand() are *not* thread safe
    srand(42);

    mCecSimulationData.SetChannel( mSettings->mCecChannel );
    mCecSimulationData.SetSampleRate( simulation_sample_rate );
    mCecSimulationData.SetInitialBitState( BIT_HIGH );
    // Advance a few ms in HIGH
    AdvanceRand(2.0f, 5.0f);
}

U32 HdmiCecSimulationDataGenerator::GenerateSimulationData( U64 largest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channel )
{
    const U64 lastSample = AnalyzerHelpers::AdjustSimulationTargetSample( largest_sample_requested, sample_rate, mSimulationSampleRateHz );

    while( mCecSimulationData.GetCurrentSampleNumber() < lastSample )
    {
        GenVersionTransaction();
        AdvanceRand(5.0f, 15.0f);

        GetStandbyTransaction();
        AdvanceRand(5.0f, 15.0f);

        GetInitTransaction();
        AdvanceRand(5.0f, 15.0f);
    }

    *simulation_channel = &mCecSimulationData;
    return 1; // We are simulating one channel
}

//
// Transaction generation
//

void HdmiCecSimulationDataGenerator::GenVersionTransaction()
{
    // The TV sends asks Tuner1 for it's CEC version
    GenStartSeqFrame();
    GenHeaderFrame(HdmiCec::DevAddress_TV, HdmiCec::DevAddress_Tuner1);
    AdvanceRand(0.2f, 0.8f);
    GenDataFrame(HdmiCec::OpCode_GetCecVersion, true, true);

    // Tuner1 asks with a CecVersion opcode and 0x4 (CEC 1.3a) as a single operand
    AdvanceRand(5.0f, 10.0f);
    GenStartSeqFrame();
    GenHeaderFrame(HdmiCec::DevAddress_Tuner1, HdmiCec::DevAddress_TV);
    AdvanceRand(0.2f, 0.8f);
    GenDataFrame(HdmiCec::OpCode_CecVersion, false, true);
    AdvanceRand(0.2f, 0.8f);
    GenDataFrame(0x4, true, true);
}

void HdmiCecSimulationDataGenerator::GetStandbyTransaction()
{
    // Audio System sends the TV an standby command
    GenStartSeqFrame();
    GenHeaderFrame(HdmiCec::DevAddress_AudioSystem, HdmiCec::DevAddress_TV);
    AdvanceRand(0.2f, 0.8f);
    GenDataFrame(HdmiCec::OpCode_Standby, true, true);

    // The TV decides to forward this message to all the devices
    AdvanceRand(5.0f, 10.0f);
    GenStartSeqFrame();
    GenHeaderFrame(HdmiCec::DevAddress_TV, HdmiCec::DevAddress_UnregBcast);
    AdvanceRand(0.2f, 0.8f);
    GenDataFrame(HdmiCec::OpCode_Standby, true, true);
}

void HdmiCecSimulationDataGenerator::GetInitTransaction()
{
    // Tuner1 sends a header frame to it's same address to verify that no one
    // ACKs the frame (ie. the logical address is available)
    GenStartSeqFrame();
    GenHeaderFrame(HdmiCec::DevAddress_Tuner1, HdmiCec::DevAddress_Tuner1, true, false );

    // Tuner1 reports physical address to bcast
    AdvanceRand(5.0f, 10.0f);
    GenStartSeqFrame();
    GenHeaderFrame(HdmiCec::DevAddress_Tuner1, HdmiCec::DevAddress_UnregBcast, false, false);
    AdvanceRand(0.2f, 0.8f);
    GenDataFrame(HdmiCec::OpCode_ReportPhysicalAddress, false, false);
    AdvanceRand(0.2f, 0.8f);
    GenDataFrame(0x10, false, false);
    GenDataFrame(0x00, false, false);
    GenDataFrame(0x03, true, false);
}


//
// Frame and bit generation
//

void HdmiCecSimulationDataGenerator::GenStartSeqFrame()
{
    // The bus must be in high
    mCecSimulationData.TransitionIfNeeded( BIT_HIGH );

    // Timing values from CEC 1.3a section 5.2.1 "Start Bit Timing"
    mCecSimulationData.Transition(); // HIGH to LOW
    Advance( 3.7f );

    mCecSimulationData.Transition(); // LOW to HIGH
    Advance( 0.8f );
}

void HdmiCecSimulationDataGenerator::GenHeaderFrame( U8 src, U8 dst, bool eom, bool ack )
{
    const U8 data = ((src & 0xF) << 4) | (dst & 0xF);
    GenDataFrame( data, eom, ack );
}

void HdmiCecSimulationDataGenerator::GenDataFrame( U8 data, bool eom, bool ack )
{
    for( int i=7; i>=0; i-- )
        GenBit( (data >> i) & 0x1 );

    GenBit( eom );
    GenBit( ack, true );
}

void HdmiCecSimulationDataGenerator::GenBit( bool value, bool ackBit )
{
    // Timing values are inverted for the follower-asserted ACK bit
    if(ackBit) value= !value;
    // Timing values from CEC 1.3a section 5.2.2 "Data Bit Timing"
    const float risingTime= value ? 0.6f : 1.5f;
    const float totalTime= 2.4f;

    // We should be in low
    mCecSimulationData.TransitionIfNeeded( BIT_LOW );

    Advance( risingTime );
    mCecSimulationData.Transition(); // LOW to HIGH

    Advance( totalTime - risingTime );
}


void HdmiCecSimulationDataGenerator::Advance(float msecs)
{
    mCecSimulationData.Advance( mClockGenerator.AdvanceByTimeS( msecs / 1000.0 ) );
}

void HdmiCecSimulationDataGenerator::AdvanceRand(float minMsecs, float maxMsecs)
{
    // Get a random number from 0 to 1
    float r = static_cast<float>(rand()) / RAND_MAX;
    // Use r in a weighted sum to obtain a random number from minMsecs to maxMsecs
    Advance( (1-r) * minMsecs + r * maxMsecs );
}
