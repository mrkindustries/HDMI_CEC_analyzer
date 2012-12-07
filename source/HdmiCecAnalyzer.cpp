#include "HdmiCecAnalyzer.h"
#include "HdmiCecAnalyzerSettings.h"
#include <AnalyzerChannelData.h>

#include "HdmiCecProtocol.h"

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

    // For each successful iteration of this loop, we add a new packet and one or
    // more frames. A packet reperesents a "CEC frame" that may contain one or more
    // CEC blocks.
    while( true )
    {
        // Read the start sequence
        Frame startSeqBlock;
        if( !ReadStartSequence( startSeqBlock ) )
        {
            MarkErrorPosition();
            // On error, cancel the packet and look for another start sequence
            mResults->CancelPacketAndStartNewPacket();
            continue;
        }
        mResults->AddFrame( startSeqBlock );
        mResults->CommitResults();
        ReportProgress( startSeqBlock.mEndingSampleInclusive );

        int blockPosition = 0;
        bool eom = false;

        // Read all the blocks in the message until the End of Message
        while( !eom )
        {
            Frame block;
            if( !ReadBlock( blockPosition, block ) )
            {
                MarkErrorPosition();
                // On error, cancel the packet and look for another start sequence
                mResults->CancelPacketAndStartNewPacket();
                break;
            }

            mResults->AddFrame( block );
            mResults->CommitResults();
            ReportProgress( block.mEndingSampleInclusive );

            blockPosition++;
            eom = block.mFlags & HdmiCec::BlockFlag_EOM;
        }

        // On the end of a successfully parsed message, insert an End marker
        // and commit the "packet"
        if( eom ) {
            mResults->CommitPacketAndStartNewPacket();
            mResults->AddMarker( mCec->GetSampleNumber(), AnalyzerResults::Stop, mSettings->mCecChannel );
        }

        mResults->CommitResults();
        ReportProgress( mCec->GetSampleNumber() );
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
    return HdmiCec::MinSampleRateHz;
}

const char* HdmiCecAnalyzer::GetAnalyzerName() const
{
    return HdmiCec::GetProtocolName();
}

const char* GetAnalyzerName()
{
    return HdmiCec::GetProtocolName();
}

Analyzer* CreateAnalyzer()
{
    return new HdmiCecAnalyzer();
}

void DestroyAnalyzer( Analyzer* analyzer )
{
    delete analyzer;
}

bool HdmiCecAnalyzer::ReadStartSequence( Frame& block )
{
    // All timing values are taken from the CEC spec, section 5.2.1 "Start Bit Timing"

    // Wait until the bus is in HIGH
    if( mCec->GetBitState() == BIT_LOW )
        mCec->AdvanceToNextEdge();

    // Advance to the next falling edge
    mCec->AdvanceToNextEdge(); // HIGH to LOW
    U64 startingSample = mCec->GetSampleNumber();
    float elapsed = 0;

    block.mType = HdmiCec::BlockType_StartSeq;
    block.mFlags = 0;
    block.mData1 = 0;
    block.mStartingSampleInclusive = mCec->GetSampleNumber();

    // Advance until the end of the "a" pulse
    mCec->AdvanceToNextEdge(); // LOW to HIGH
    elapsed = TimeSince( startingSample );
    // Check that pulse ends in the correct time
    if( elapsed < HdmiCec::Tim_Start_AMin || elapsed > HdmiCec::Tim_Start_AMax )
        return false;

    // Advance until the end of the "b" pulse
    mCec->AdvanceToNextEdge(); // HIGH to LOW
    elapsed = TimeSince( startingSample );
    // Check that the pulse ends in the correct time
    if( elapsed < HdmiCec::Tim_Start_BMin || elapsed > HdmiCec::Tim_Start_BMax )
        return false;

    // Add start marker on beginning sequence
    mResults->AddMarker( block.mStartingSampleInclusive, AnalyzerResults::Start, mSettings->mCecChannel );

    // The last sample is the sample just before the edge
    block.mEndingSampleInclusive = mCec->GetSampleNumber()-1;
    return true;
}

bool HdmiCecAnalyzer::ReadBlock( int blockIndex, Frame& block )
{
    // Wait until the bus is in LOW
    if( mCec->GetBitState() == BIT_HIGH )
        mCec->AdvanceToNextEdge();

    // Reset flags
    block.mFlags = 0;

    // Depending on the position on the message set block type
    if( !blockIndex )
        block.mType = HdmiCec::BlockType_Header;
    else if( blockIndex == 1 )
        block.mType = HdmiCec::BlockType_OpCode;
    else if( blockIndex < HdmiCec::MaxMessageBlocks )
        block.mType = HdmiCec::BlockType_Operand;
    else
        return false;

    block.mStartingSampleInclusive = mCec->GetSampleNumber();

    // Read block byte and End of Message bit
    U8 data;
    bool eom;
    if( !ReadByteEOM( data, eom ) )
        return false;

    // Read block ACK
    bool ack;
    // ReadByteEOM quits just after the falling edge (we are in LOW)
    U64 ackStartSample = mCec->GetSampleNumber();
    mCec->AdvanceToNextEdge(); // LOW to HIGH
    float elapsed = TimeSince( ackStartSample );
    // Logic values are inverted for ACK
    ack = elapsed > HdmiCec::Tim_Bit_ZeroMin && elapsed < HdmiCec::Tim_Bit_ZeroMax;
    // Ack rising edge should happen before the earliest time for the start of the next bit
    if( elapsed >= HdmiCec::Tim_Bit_LenMin )
        return false;

    // Mark ACK bit
    mResults->AddMarker( mCec->GetSampleNumber()-1, ack ? AnalyzerResults::One : AnalyzerResults::Zero,
                         mSettings->mCecChannel );

    // Advance to the end of the data bit
    // The bus should stay in HIGH at least until HdmiCec::Tim_Bit_LenMin
    U32 samplesToAdvance1 = ( HdmiCec::Tim_Bit_LenMin - elapsed ) * GetSampleRate() / 1000.0;
    if( mCec->WouldAdvancingCauseTransition( samplesToAdvance1 ) )
        return false;
    // If by the nominal data bit period there is no rising edge, move there,
    // else move to the minimum data bit period
    U32 samplesToAdvance2 = ( HdmiCec::Tim_Bit_Len - elapsed ) * GetSampleRate() / 1000.0;
    if( mCec->WouldAdvancingCauseTransition( samplesToAdvance2 ) )
        mCec->Advance( samplesToAdvance1 );
    else
        mCec->Advance( samplesToAdvance2 );

    // The block ends just before the edge where the bus returns to high
    block.mEndingSampleInclusive = mCec->GetSampleNumber()-1;

    // Store block data and flags
    block.mData1 = data;
    if( eom ) block.mFlags |= HdmiCec::BlockFlag_EOM;
    if( ack ) block.mFlags |= HdmiCec::BlockFlag_ACK;

    return true;
}

bool HdmiCecAnalyzer::ReadByteEOM( U8& data, bool& eom )
{
    // All timing values are taken from the CEC spec, section 5.2.2 "Data Bit Timing"

    // Wait until the bus is in LOW
    if( mCec->GetBitState() == BIT_HIGH )
        mCec->AdvanceToNextEdge();

    // Reset data
    data= 0;

    // Read from the MSB to the LSB, then read the EOM bit ("bit -1")
    for( int bit=7; bit>=-1; bit-- )
    {
        U64 firstSample = mCec->GetSampleNumber();

        mCec->AdvanceToNextEdge(); // LOW to HIGH
        float elapsed = TimeSince( firstSample );

        bool value;
        if( elapsed > HdmiCec::Tim_Bit_OneMin && elapsed < HdmiCec::Tim_Bit_OneMax )
            value = true;  // Logical 1
        else if( elapsed > HdmiCec::Tim_Bit_ZeroMin && elapsed < HdmiCec::Tim_Bit_ZeroMax )
            value = false; // Logical 0
        else
            return false;
        // Mark bit
        mResults->AddMarker( mCec->GetSampleNumber(), value ? AnalyzerResults::One : AnalyzerResults::Zero,
                             mSettings->mCecChannel );

        mCec->AdvanceToNextEdge(); // HIGH to LOW
        elapsed = TimeSince( firstSample );
        // Check overall bit period
        if( elapsed < HdmiCec::Tim_Bit_LenMin || elapsed > HdmiCec::Tim_Bit_LenMax )
            return false;

        // Bits 7..0 are written to data, and the extra bit is written to eom
        if( bit >= 0 )
            data |= value << bit;
        else
            eom = value;
    }
    return true;
}

float HdmiCecAnalyzer::TimeSince( U64 sample )
{
    const S64 sampleDiff = mCec->GetSampleNumber() - sample;
    return sampleDiff * 1000.0 / GetSampleRate();
}

void HdmiCecAnalyzer::MarkErrorPosition()
{
    mResults->AddMarker( mCec->GetSampleNumber(), AnalyzerResults::ErrorDot, mSettings->mCecChannel );
}
