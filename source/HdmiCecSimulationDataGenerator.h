#ifndef HDMICEC_SIMULATION_DATA_GENERATOR
#define HDMICEC_SIMULATION_DATA_GENERATOR

#include <SimulationChannelDescriptor.h>
#include <string>
class HdmiCecAnalyzerSettings;

class HdmiCecSimulationDataGenerator
{
public:
    HdmiCecSimulationDataGenerator();
    ~HdmiCecSimulationDataGenerator();

    void Initialize( U32 simulation_sample_rate, HdmiCecAnalyzerSettings* settings );
    U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channel );

protected:
    HdmiCecAnalyzerSettings* mSettings;
    U32 mSimulationSampleRateHz;

protected:
    SimulationChannelDescriptor mCecSimulationData;

};
#endif //HDMICEC_SIMULATION_DATA_GENERATOR
