#include "HdmiCecAnalyzerResults.h"
#include <AnalyzerHelpers.h>
#include "HdmiCecAnalyzer.h"
#include "HdmiCecAnalyzerSettings.h"
#include "HdmiCecProtocol.h"

#include <fstream>
#include <sstream>

HdmiCecAnalyzerResults::HdmiCecAnalyzerResults( HdmiCecAnalyzer* analyzer, HdmiCecAnalyzerSettings* settings )
:	AnalyzerResults(),
    mSettings( settings ),
    mAnalyzer( analyzer )
{
}

HdmiCecAnalyzerResults::~HdmiCecAnalyzerResults()
{
}

void HdmiCecAnalyzerResults::GenerateBubbleText( U64 frame_index, Channel& /*channel*/, DisplayBase display_base )
{
    GenBubbleText( frame_index, display_base, false );
}

void HdmiCecAnalyzerResults::GenerateFrameTabularText( U64 frame_index, DisplayBase display_base )
{
    GenBubbleText( frame_index, display_base, true );
}

void HdmiCecAnalyzerResults::GeneratePacketTabularText( U64 /*packet_id*/, DisplayBase /*display_base*/ )
{
    ClearResultStrings();
    AddResult( "Not supported" );
}

void HdmiCecAnalyzerResults::GenerateTransactionTabularText( U64 /*transaction_id*/, DisplayBase /*display_base*/ )
{
    ClearResultStrings();
    AddResult( "Not supported" );
}

void HdmiCecAnalyzerResults::GenerateExportFile( const char* file, DisplayBase display_base, U32 /*export_type_user_id*/ )
{
    std::ofstream file_stream( file, std::ios::out );

    mDisplayBase = display_base;
    U64 trigger_sample = mAnalyzer->GetTriggerSample();
    U32 sample_rate = mAnalyzer->GetSampleRate();

    file_stream << "Time [s],Block ID,Type,Data Desc,Data,Flags" << std::endl;

    // String to fill "not applicable" field, maybe an empty string would be
    // easier to parse
    const std::string naString = "N/A";
    // If this showStartSeqs is set to false, start sequence "blocks" won't be listed
    const bool showStartSeqs = true;

    U32 blockId = 0;
    const U64 blockCount = GetNumFrames();
    for( U64 i=0; i < blockCount; i++ )
    {
        Frame block = GetFrame( i );
        HdmiCec::BlockType blockType= static_cast<HdmiCec::BlockType>( block.mType );
        bool showBlock = true;

        const int strLen = 128;
        char timeStr[strLen];
        AnalyzerHelpers::GetTimeString( block.mStartingSampleInclusive, trigger_sample, sample_rate, timeStr, strLen );

        std::stringstream ss;
        ss << blockId;
        std::string blockIdStr = ss.str();

        std::string typeDesc = HdmiCec::GetBlockTypeString( blockType );

        std::string flags = ( block.mFlags & HdmiCec::BlockFlag_ACK ) ? "ACK" : "NACK";
        if( block.mFlags & HdmiCec::BlockFlag_EOM )
            flags += " EOM";

        std::string dataCode = GetNumberString( block.mData1, 8 );
        std::string dataDesc = naString;
        if( blockType == HdmiCec::BlockType_Header )
        {
            const U8 src = ( block.mData1 >> 4 ) & 0xF;
            const U8 dst = ( block.mData1 >> 0 ) & 0xF;
            std::string srcStr = GetNumberString( src, 4 );
            std::string dstStr = GetNumberString( dst, 4 );
            std::string srcName = HdmiCec::GetDevAddressString( static_cast<HdmiCec::DevAddress>( src ) );
            std::string dstName = HdmiCec::GetDevAddressString( static_cast<HdmiCec::DevAddress>( dst ) );
            dataDesc = "SRC=" + srcStr + " (" + srcName + ") DST=" + dstStr + " ("+dstName+")";
        }
        else if( blockType == HdmiCec::BlockType_OpCode )
        {
            HdmiCec::OpCode opCode = static_cast<HdmiCec::OpCode>( block.mData1 );
            dataDesc = HdmiCec::GetOpCodeString( opCode );
        }
        else if( blockType == HdmiCec::BlockType_StartSeq )
        {
            // The start sequence is not really a block
            blockIdStr = naString;
            flags = naString;
            dataCode = naString;
            showBlock = showStartSeqs;
        }

        if( showBlock )
        {
            file_stream << timeStr << "," << blockIdStr << "," << typeDesc << "," <<
                        dataDesc << "," << dataCode << "," << flags << std::endl;
        }

        // Don't count the start sequence "blocks"
        if( blockType != HdmiCec::BlockType_StartSeq )
            blockId++;

        if( UpdateExportProgressAndCheckForCancel( i, blockCount ) )
        {
            file_stream.close();
            return;
        }
    }

    file_stream.close();
}

//
// Bubble generation
//

void HdmiCecAnalyzerResults::GenBubbleText( U64 frame_index, DisplayBase display_base, bool tabular )
{
    ClearResultStrings();
    Frame frame = GetFrame( frame_index );
    mDisplayBase = display_base;
    mTabular = tabular;

    switch( frame.mType )
    {
        case HdmiCec::BlockType_StartSeq:
            GenStartSeqBubble();
            break;
        case HdmiCec::BlockType_Header:
            GenHeaderBubble( frame );
            break;
        case HdmiCec::BlockType_OpCode:
            GenOpCodeBubble( frame );
            break;
        case HdmiCec::BlockType_Operand:
            GenOperandBubble( frame );
            break;
        default:
            break;
    }
}

void HdmiCecAnalyzerResults::GenStartSeqBubble()
{
    if( !mTabular )
    {
        AddResult( "S" );
        AddResult( "Start" );
        AddResult( "Start Seq." );
    }
    AddResult( "Start Sequence" );
}

void HdmiCecAnalyzerResults::GenHeaderBubble( const Frame& block )
{
    const U8 src = ( block.mData1 >> 4 ) & 0xF;
    const U8 dst = ( block.mData1 >> 0 ) & 0xF;
    std::string srcStr = GetNumberString( src, 4 );
    std::string dstStr = GetNumberString( dst, 4 );

    if( !mTabular )
    {
        AddResult( "H" );
        AddResult( "H " + srcStr + " to " + dstStr );
        AddResult( "Header SRC=" + srcStr + ", DST=" + dstStr );
        AddResult( "Header SRC=" + srcStr + ", DST=" + dstStr, block );
    }

    std::string srcName = HdmiCec::GetDevAddressString(
                static_cast<HdmiCec::DevAddress>( src) );
    std::string dstName = HdmiCec::GetDevAddressString(
                static_cast<HdmiCec::DevAddress>( dst ) );

    AddResult( "Header SRC=" + srcStr + " (" + srcName + "), DST=" + dstStr + " ("+dstName+")", block );
}

void HdmiCecAnalyzerResults::GenOpCodeBubble( const Frame& block )
{
    HdmiCec::OpCode opCode = static_cast<HdmiCec::OpCode>( block.mData1 );
    std::string opCodeStr = GetNumberString( block.mData1, 8 );
    std::string opCodeText = HdmiCec::GetOpCodeString( opCode );

    if( !mTabular )
    {
        AddResult( "O" );
        AddResult( "Op. " + opCodeStr );
        AddResult( "Opcode " + opCodeStr );
        AddResult( "Opcode " + opCodeStr, block );
    }
    AddResult( "Opcode " + opCodeStr + " (" + opCodeText + ")", block );
}

void HdmiCecAnalyzerResults::GenOperandBubble( const Frame& block )
{
    std::string dataStr = GetNumberString( block.mData1, 8 );

    if( !mTabular )
    {
        AddResult( "D" );
        AddResult( "Data" );
        AddResult( "Data " + dataStr );
    }
    AddResult( "Data " + dataStr, block );
}

//
// std::string wrappers
//

std::string HdmiCecAnalyzerResults::GetNumberString( U64 number, int bits )
{
    const int strLen = 128;
    char str[strLen];
    AnalyzerHelpers::GetNumberString( number, mDisplayBase, bits, str, strLen );
    return std::string( str );
}

void HdmiCecAnalyzerResults::AddResult( const std::string& str )
{
    AddResultString( str.c_str() );
}

void HdmiCecAnalyzerResults::AddResult( const std::string& str, const Frame& block )
{
    const bool ack = block.mFlags & HdmiCec::BlockFlag_ACK;
    const bool eom = block.mFlags & HdmiCec::BlockFlag_EOM;

    std::string strCopy = str + " |";
    if( eom )
        strCopy += " EOM";
    strCopy += ack ? " ACK" : " NACK";

    AddResultString( strCopy.c_str() );
}
