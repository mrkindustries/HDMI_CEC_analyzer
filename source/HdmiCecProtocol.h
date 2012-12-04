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

const char* AnalyzerName = "HDMI CEC";
const char* AnalyzerFullName = "HDMI Consumer Electronics Control (CEC)";

// The minimum pulse width is 0.4ms (for an "initiator asserted" logical 1).
// Reference: Section 5.2.2 "Data Bit Timing"
// Max frequency is 1000ms / 0.4ms= 2.5 kHz.
static const U32 MaxFrequency= 2500;

// Minimum recommended sample rate: mMaxBitRate * 4 = 10 kHz
static const U32 MinSampleRate= MaxFrequency * 4;

} // namespace HdmiCec


#endif // HDMICEC_PROTOCOL
