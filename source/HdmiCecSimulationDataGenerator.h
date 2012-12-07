#ifndef HDMICEC_SIMULATION_DATA_GENERATOR
#define HDMICEC_SIMULATION_DATA_GENERATOR

#include <SimulationChannelDescriptor.h>
#include <AnalyzerHelpers.h>
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

    SimulationChannelDescriptor mCecSimulationData;
    ClockGenerator mClockGenerator;

    // Generates a GetCecVersion/CecVersion request/answer transaction
    void GenVersionTransaction();
    // Generates a Standby transaction
    void GetStandbyTransaction();
    // Generates an Init transaction: checks that the address is not taken then
    // reports physical address
    void GetInitTransaction();


    void GenStartSeqFrame();
    void GenHeaderFrame(U8 src, U8 dst, bool eom = false, bool ack= true);
    void GenDataFrame(U8 data, bool eom, bool ack);
    // Generates a transition representing a single bit, if ackBit is true
    // the the timings are inverted as indicated by the spec
    void GenBit(bool value, bool ackBit = false);

    // Advance a time period in the current state
    void Advance(float msecs);
    // Advance a random time period from minMsecs to maxMsecs in the current state
    void AdvanceRand(float minMsecs, float maxMsecs);
};
#endif //HDMICEC_SIMULATION_DATA_GENERATOR
