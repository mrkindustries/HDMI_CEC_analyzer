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
        case HdmiCec::FrameType_OpCode:
            GenOpCodeBubble(frame);
            break;
        case HdmiCec::FrameType_Operand:
            GenOperandBubble(frame);
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

    //char number_str[128];
    //AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
    //AddResultString( number_str );
}

void HdmiCecAnalyzerResults::GeneratePacketTabularText( U64 packet_id, DisplayBase display_base )
{
    ClearResultStrings();
    AddResult( "not supported" );
}

void HdmiCecAnalyzerResults::GenerateTransactionTabularText( U64 transaction_id, DisplayBase display_base )
{
    ClearResultStrings();
    AddResult( "not supported" );
}

void HdmiCecAnalyzerResults::GenStartSeqBubble()
{
    AddResult("S");
    AddResult("Start");
    AddResult("Start Seq.");
    AddResult("Start Sequence");
}

void HdmiCecAnalyzerResults::GenHeaderBubble(const Frame& frame)
{
    const U8 src= (frame.mData1 >> 4) & 0xF;
    const U8 dst= (frame.mData1 >> 0) & 0xF;
    std::string srcStr= GetNumberString( src, 4 );
    std::string dstStr= GetNumberString( dst, 4 );

    AddResult( "H" );
    AddResult( "H " + srcStr + " to " + dstStr );
    AddResult( "Header SRC=" + srcStr + ", DST=" + dstStr );
    AddResult( "Header SRC=" + srcStr + ", DST=" + dstStr, frame );

    std::string srcName= HdmiCec::GetDevAddressText(static_cast<HdmiCec::DevAddress>(src));
    std::string dstName= HdmiCec::GetDevAddressText(static_cast<HdmiCec::DevAddress>(dst));
    AddResult( "Header SRC=" + srcStr + " (" + srcName + "), DST=" + dstStr + " ("+dstName+")", frame );
}

void HdmiCecAnalyzerResults::GenOpCodeBubble(const Frame& frame)
{
    HdmiCec::OpCode opCode= static_cast<HdmiCec::OpCode>(frame.mData1);
    std::string opCodeStr= GetNumberString( frame.mData1, 8 );
    std::string opCodeText= HdmiCec::GetOpCodeText(opCode);

    AddResult( "O" );
    AddResult( "Op. " + opCodeStr );
    AddResult( "Opcode " + opCodeStr );
    AddResult( "Opcode " + opCodeStr, frame );
    AddResult( "Opcode " + opCodeStr + " (" + opCodeText + ")", frame );
}

void HdmiCecAnalyzerResults::GenOperandBubble(const Frame& frame)
{
    std::string dataStr= GetNumberString( frame.mData1, 8 );

    AddResult( "D" );
    AddResult( "Data" );
    AddResult( "Data " + dataStr );
    AddResult( "Data " + dataStr, frame );
}

std::string HdmiCecAnalyzerResults::GetNumberString( U64 number, int bits )
{
    const int strLen= 100;
    char str[strLen];
    AnalyzerHelpers::GetNumberString( number, mDisplayBase, bits, str, strLen );
    return std::string( str );
}

void HdmiCecAnalyzerResults::AddResult(const std::string& str)
{
    AddResultString( str.c_str() );
}

void HdmiCecAnalyzerResults::AddResult(const std::string& str, const Frame& frame)
{
    const bool ack = frame.mFlags & HdmiCec::FrameFlag_ACK;
    const bool eom = frame.mFlags & HdmiCec::FrameFlag_EOM;

    std::string strCopy= str + " |";
    if( eom )
        strCopy += " EOM";
    strCopy += ack ? " ACK" : " NACK";

    AddResultString( strCopy.c_str() );
}
