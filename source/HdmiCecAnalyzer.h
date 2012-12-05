#ifndef HDMICEC_ANALYZER_H
#define HDMICEC_ANALYZER_H

#include <Analyzer.h>
#include "HdmiCecAnalyzerResults.h"
#include "HdmiCecSimulationDataGenerator.h"

class HdmiCecAnalyzerSettings;

class ANALYZER_EXPORT HdmiCecAnalyzer : public Analyzer
{
public:
    HdmiCecAnalyzer();
    virtual ~HdmiCecAnalyzer();
    virtual void WorkerThread();

    virtual U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels );
    virtual U32 GetMinimumSampleRateHz();

    virtual const char* GetAnalyzerName() const;
    virtual bool NeedsRerun();

protected:
    // Returns the elapsed time in msecs since a past sample
    float timeSince(U64 sample);
    // Reads the "start CEC frame sequency", returns false on error
    bool readStartSequence(Frame& frame);
    // Read a 10-bit CEC word. Sets frame type depending on the frame order in the message
    // Returns false on error.
    bool readFrame(int frameIndex, Frame& frame);
    // Reads the frame byte plus the EOM bit written by the initiatior.
    // Returns false on error
    bool readByteEOM(U8& data, bool& eom);

    std::auto_ptr< HdmiCecAnalyzerSettings > mSettings;
    std::auto_ptr< HdmiCecAnalyzerResults > mResults;
    AnalyzerChannelData* mCec;

    HdmiCecSimulationDataGenerator mSimulationDataGenerator;
    bool mSimulationInitilized;
};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer( );
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer( Analyzer* analyzer );

#endif //HDMICEC_ANALYZER_H
