#include "HdmiCecAnalyzerResults.h"
#include <AnalyzerHelpers.h>
#include "HdmiCecAnalyzer.h"
#include "HdmiCecAnalyzerSettings.h"
#include "HdmiCecProtocol.h"
#include <iostream>
#include <fstream>

HdmiCecAnalyzerResults::HdmiCecAnalyzerResults( HdmiCecAnalyzer* analyzer, HdmiCecAnalyzerSettings* settings )
:	AnalyzerResults(),
    mSettings( settings ),
    mAnalyzer( analyzer )
{
}

HdmiCecAnalyzerResults::~HdmiCecAnalyzerResults()
{
}

void HdmiCecAnalyzerResults::GenerateBubbleText( U64 frame_index, Channel& channel, DisplayBase display_base )
{
    ClearResultStrings();
    Frame frame = GetFrame( frame_index );
    mDisplayBase= display_base;

    switch( frame.mType )
    {
        case HdmiCec::FrameType_StartSeq:
            GenStartSeqBubble();
            break;
        case HdmiCec::FrameType_Header:
            GenHeaderBubble(frame);
            break;
        default:
            break;
    }
}

void HdmiCecAnalyzerResults::GenerateExportFile( const char* file, DisplayBase display_base, U32 export_type_user_id )
{
    std::ofstream file_stream( file, std::ios::out );

    U64 trigger_sample = mAnalyzer->GetTriggerSample();
    U32 sample_rate = mAnalyzer->GetSampleRate();

    file_stream << "Time [s],Value" << std::endl;

    U64 num_frames = GetNumFrames();
    for( U32 i=0; i < num_frames; i++ )
    {
        Frame frame = GetFrame( i );

        char time_str[128];
        AnalyzerHelpers::GetTimeString( frame.mStartingSampleInclusive, trigger_sample, sample_rate, time_str, 128 );

        char number_str[128];
        AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );

        file_stream << time_str << "," << number_str << std::endl;

        if( UpdateExportProgressAndCheckForCancel( i, num_frames ) == true )
        {
            file_stream.close();
            return;
        }
    }

    file_stream.close();
}

void HdmiCecAnalyzerResults::GenerateFrameTabularText( U64 frame_index, DisplayBase display_base )
{
    Frame frame = GetFrame( frame_index );
    ClearResultStrings();

    char number_str[128];
    AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
    AddResultString( number_str );
}

void HdmiCecAnalyzerResults::GeneratePacketTabularText( U64 packet_id, DisplayBase display_base )
{
    ClearResultStrings();
    AddResultString( "not supported" );
}

void HdmiCecAnalyzerResults::GenerateTransactionTabularText( U64 transaction_id, DisplayBase display_base )
{
    ClearResultStrings();
    AddResultString( "not supported" );
}

void HdmiCecAnalyzerResults::GenStartSeqBubble()
{
    AddResultString("S");
    AddResultString("Start");
    AddResultString("Start Seq.");
    AddResultString("Start Sequence");
}

void HdmiCecAnalyzerResults::GenHeaderBubble(const Frame& frame)
{
    const U8 src= (frame.mData1 >> 4) & 0xF;
    const U8 dst= (frame.mData1 >> 0) & 0xF;

    const U32 strLen= 50;
    char srcStr[strLen];
    char dstStr[strLen];
    AnalyzerHelpers::GetNumberString( src, mDisplayBase, 4, srcStr, strLen );
    AnalyzerHelpers::GetNumberString( dst, mDisplayBase, 4, dstStr, strLen );

    AddResultString("H");
    AddResultString("H ", srcStr, " to ", dstStr);
    AddResultString("Header SRC=", srcStr, ", DST=", dstStr);
}
