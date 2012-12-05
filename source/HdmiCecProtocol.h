#ifndef HDMICEC_PROTOCOL
#define HDMICEC_PROTOCOL

//
// HDMI CIC Protocol enums and parameters
//
// Reference: HDMI CEC spec. version 1.3a

#include <LogicPublicTypes.h>

// Avoid namespace polution using a protocol-specific namespace
namespace HdmiCec
{

static const char* ProtocolName = "HDMI CEC";
static const char* ProtocolFullName = "HDMI Consumer Electronics Control (CEC)";
static const char* Channel1Name = "CEC";

// The minimum pulse width is 0.4ms (for an "initiator asserted" logical 1).
// Reference: Section 5.2.2 "Data Bit Timing"
// Max frequency is 1000ms / 0.4ms= 2.5 kHz.
static const U32 MaxFrequency = 2500;

// Minimum recommended sample rate: mMaxBitRate * 4 = 10 kHz
static const U32 MinSampleRate = MaxFrequency * 4;

// Maximum number of operands in a message
static const U16 MaxMessageOperands = 14;
// Maximum number of frames in a message (header frame + op frame + 0..14 operand frames)
static const U16 MaxMessageFrames = 2 + MaxMessageOperands;

//
// Analyzer implementation
//

enum FrameFlags
{
    FrameFlag_EOM = 0x1,
    FrameFlag_ACK = 0x2
};

enum FrameTypes
{
    FrameType_StartSeq, // CEC start sequence
    FrameType_Header,   // Header frame containing SRC and DST addresses
    FrameType_OpCode,   // Frame containing an opcode
    FrameType_Operand   // Frame containing a single operand
};


} // namespace HdmiCec


#endif // HDMICEC_PROTOCOL
