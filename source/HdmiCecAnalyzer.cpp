#include "HdmiCecAnalyzer.h"

#include <algorithm>

#include "HdmiCecAnalyzerSettings.h"
#include <AnalyzerChannelData.h>
#include "HdmiCecProtocol.h"

#include <iostream>
using namespace std;

HdmiCecAnalyzer::HdmiCecAnalyzer()
:	Analyzer(),
    mSettings( new HdmiCecAnalyzerSettings() ),
    mSimulationInitilized( false )
{
    SetAnalyzerSettings( mSettings.get() );
}

HdmiCecAnalyzer::~HdmiCecAnalyzer()
{
    KillThread();
}

void HdmiCecAnalyzer::WorkerThread()
{
    mResults.reset( new HdmiCecAnalyzerResults( this, mSettings.get() ) );
    SetAnalyzerResults( mResults.get() );
    mResults->AddChannelBubblesWillAppearOn( mSettings->mCecChannel );

    mCec = GetAnalyzerChannelData( mSettings->mCecChannel );

    while(true)
    {
        // Read the start sequence
        Frame startSeqFrame;
        if( !readStartSequence(startSeqFrame) )
            continue;
        mResults->AddFrame( startSeqFrame );
        mResults->CommitResults();
        ReportProgress( startSeqFrame.mEndingSampleInclusive );

        int frameCount= 0;
        bool eom;

        // Read all the frames in the message until the End of Message
        do {
            Frame frame;
            if( !readFrame( frameCount, frame ) )
                break;
            mResults->AddFrame( frame );
            mResults->CommitResults();
            ReportProgress( frame.mEndingSampleInclusive );

            frameCount++;
            eom= frame.mFlags & HdmiCec::FrameFlag_EOM;
        } while( !eom );
    }
}

bool HdmiCecAnalyzer::NeedsRerun()
{
    return false;
}

U32 HdmiCecAnalyzer::GenerateSimulationData( U64 minimum_sample_index, U32 device_sample_rate, SimulationChannelDescriptor** simulation_channels )
{
    if( mSimulationInitilized == false )
    {
        mSimulationDataGenerator.Initialize( GetSimulationSampleRate(), mSettings.get() );
        mSimulationInitilized = true;
    }

    return mSimulationDataGenerator.GenerateSimulationData( minimum_sample_index, device_sample_rate, simulation_channels );
}

U32 HdmiCecAnalyzer::GetMinimumSampleRateHz()
{
    // The min. sampling rate is 25 kHz
    return std::max(HdmiCec::MinSampleRate, static_cast<U32>(25000));
}

const char* HdmiCecAnalyzer::GetAnalyzerName() const
{
    return HdmiCec::AnalyzerName;
}

const char* GetAnalyzerName()
{
    return HdmiCec::AnalyzerName;
}

Analyzer* CreateAnalyzer()
{
    return new HdmiCecAnalyzer();
}

void DestroyAnalyzer( Analyzer* analyzer )
{
    delete analyzer;
}

float HdmiCecAnalyzer::timeSince(U64 sample)
{
    // Clamp sample difference to 0 in case sample < currentSample
    const U64 sampleDiff= std::max(mCec->GetSampleNumber()-sample, static_cast<U64>(0));
    return sampleDiff * 1000.0 / GetSampleRate();
}

bool HdmiCecAnalyzer::readStartSequence(Frame& frame)
{
    // All timing values are taken from the CEC spec, section 5.2.1 "Start Bit Timing"

    // Wait until the bus is in HIGH
    if( mCec->GetBitState() == BIT_LOW )
        mCec->AdvanceToNextEdge();

    frame.mType= HdmiCec::FrameType_StartSeq;
    frame.mFlags= 0;
    frame.mStartingSampleInclusive= mCec->GetSampleNumber();

    // Advance to the next falling edge
    mCec->AdvanceToNextEdge(); // HIGH to LOW
    U64 startingSample= mCec->GetSampleNumber();
    float elapsed= 0;

    // Next edge should be between 3.5 and 3.9ms since START_0A
    mCec->AdvanceToNextEdge(); // LOW to HIGH
    elapsed = timeSince(startingSample);
    cerr << "Elapsed1 " << elapsed << endl;
    // Check that START_0A ends in the correct time (3.7ms is the ideal)
    if( elapsed < 3.5f || elapsed > 3.9f )
        return false;

    // Next edge should be between 4.3 and 4.7ms since START_0A
    mCec->AdvanceToNextEdge(); // HIGH to LOW
    elapsed = timeSince(startingSample);
    cerr << "Elapsed2 " << elapsed << endl;
    // Check that START_1 ends in the correct time (4.5ms is the ideal)
    if( elapsed < 4.3f || elapsed > 4.7f )
        return false;

    // The last sample is the sample just before the edge
    frame.mEndingSampleInclusive= mCec->GetSampleNumber()-1;
    return true;
}

bool HdmiCecAnalyzer::readFrame(int frameIndex, Frame& frame)
{
    // Depending on the position on the message set frame type
    if( !frameIndex )
        frame.mType= HdmiCec::FrameType_Header;
    else if( frameIndex == 1 )
        frame.mType= HdmiCec::FrameType_OpCode;
    else if( frameIndex < HdmiCec::MaxMessageFrames )
        frame.mType= HdmiCec::FrameType_Operand;
    else
        return false;

    frame.mStartingSampleInclusive= mCec->GetSampleNumber();
    cerr << "Start frame " << frame.mStartingSampleInclusive << endl;

    // Read frame byte and End of Message bit
    U8 data;
    bool eom;
    if(!readByteEOM(data, eom))
        return false;
    cerr << "Data " << (int)data << " eom " << (int)eom << endl;

    // Read frame ACK
    bool ack;
    U64 sample = mCec->GetSampleNumber();
    mCec->AdvanceToNextEdge(); // LOW to HIGH
    float elapsed= timeSince(sample);
    cerr << "Elapsed5 " << elapsed << endl;
    ack= elapsed > 1.3f && elapsed < 1.7f;

    // The frame just before the edge where the bus returns to high
    frame.mEndingSampleInclusive= mCec->GetSampleNumber()-1;
    cerr << "Ack " << ack << endl;

    // Store frame data and flags
    frame.mData1 = data;
    frame.mFlags = 0;
    if( eom ) frame.mFlags |= HdmiCec::FrameFlag_EOM;
    if( ack ) frame.mFlags |= HdmiCec::FrameFlag_ACK;
}

bool HdmiCecAnalyzer::readByteEOM(U8& data, bool& eom)
{
    // All timing values are taken from the CEC spec, section 5.2.2 "Data Bit Timing"

    // Wait until the bus is in LOW
    if( mCec->GetBitState() == BIT_HIGH )
        mCec->AdvanceToNextEdge();

    // Reset data
    data= 0;

    // Read from the MSB to the LSB, then read the EOM bit ("bit -1")
    for(int bit=7; bit>=-1; bit--) {
        U64 firstSample= mCec->GetSampleNumber();

        mCec->AdvanceToNextEdge(); // LOW to HIGH
        float elapsed= timeSince(firstSample);
        cerr << "Elapsed3 " << elapsed << endl;

        bool value;
        if(elapsed > 0.4f && elapsed < 0.8f)
            value= true;  // Logical 1
        else if(elapsed > 1.3f && elapsed < 1.7f)
            value= false; // Logical 0
        else
            return false;

        mCec->AdvanceToNextEdge(); // HIGH to LOW
        elapsed= timeSince(firstSample);
        cerr << "Elapsed4 " << elapsed << " bit " << bit << endl;
        if(elapsed < 2.05f || elapsed > 2.75f)
            return false;

        // Bits 7..0 are written to data, and the extra bit is written to eom
        if(bit >= 0)
            data |= value << bit;
        else
            eom= value;
    }
    cerr << "Done byte" << endl;
    return true;
}
