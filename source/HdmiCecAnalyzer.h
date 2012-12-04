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

protected: //vars
    std::auto_ptr< HdmiCecAnalyzerSettings > mSettings;
    std::auto_ptr< HdmiCecAnalyzerResults > mResults;
    AnalyzerChannelData* mCec;

    HdmiCecSimulationDataGenerator mSimulationDataGenerator;
    bool mSimulationInitilized;

    //Serial analysis vars:
    U32 mSampleRateHz;
    U32 mStartOfStopBitOffset;
    U32 mEndOfStopBitOffset;
};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer( );
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer( Analyzer* analyzer );

#endif //HDMICEC_ANALYZER_H
