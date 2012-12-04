#include "HdmiCecAnalyzer.h"

#include <algorithm>

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

    mSampleRateHz = GetSampleRate();

    mCec = GetAnalyzerChannelData( mSettings->mCecChannel );

    if( mCec->GetBitState() == BIT_LOW )
        mCec->AdvanceToNextEdge();

    U32 samples_per_bit = mSampleRateHz / HdmiCec::MaxFrequency;
    U32 samples_to_first_center_of_first_data_bit = U32( 1.5 * double( mSampleRateHz ) / double( HdmiCec::MaxFrequency ) );

    for( ; ; )
    {
        U8 data = 0;
        U8 mask = 1 << 7;

        mCec->AdvanceToNextEdge(); //falling edge -- beginning of the start bit

        U64 starting_sample = mCec->GetSampleNumber();

        mCec->Advance( samples_to_first_center_of_first_data_bit );

        for( U32 i=0; i<8; i++ )
        {
            //let's put a dot exactly where we sample this bit:
            mResults->AddMarker( mCec->GetSampleNumber(), AnalyzerResults::Dot, mSettings->mCecChannel );

            if( mCec->GetBitState() == BIT_HIGH )
                data |= mask;

            mCec->Advance( samples_per_bit );

            mask = mask >> 1;
        }


        //we have a byte to save.
        Frame frame;
        frame.mData1 = data;
        frame.mFlags = 0;
        frame.mStartingSampleInclusive = starting_sample;
        frame.mEndingSampleInclusive = mCec->GetSampleNumber();

        mResults->AddFrame( frame );
        mResults->CommitResults();
        ReportProgress( frame.mEndingSampleInclusive );
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
    return std::max(HdmiCec::MinSampleRate, 25000);
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
