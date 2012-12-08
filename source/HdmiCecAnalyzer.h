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
    // Reads the start sequence, returns false on error
    bool ReadStartSequence( Frame& block );
    // Reads a 10-bit CEC block. Sets mType depending on the block position
    // in the message (as indicated by blockIndex). Returns false on error.
    bool ReadBlock( int blockIndex, Frame& block );
    // Reads the block data byte plus the EOM bit written by the initiatior.
    // Returns false on error
    bool ReadByteEOM( U8& data, bool& eom );

    // Returns the elapsed time in msecs since another sample.
    // TimeSince will return a negative number if "sample" is in the future.
    float TimeSince( U64 sample );
    // Adds an error marker to the current position
    void MarkErrorPosition();

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
