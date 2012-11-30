#ifndef HDMICEC_ANALYZER_SETTINGS
#define HDMICEC_ANALYZER_SETTINGS

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

class HdmiCecAnalyzerSettings : public AnalyzerSettings
{
public:
	HdmiCecAnalyzerSettings();
	virtual ~HdmiCecAnalyzerSettings();

	virtual bool SetSettingsFromInterfaces();
	void UpdateInterfacesFromSettings();
	virtual void LoadSettings( const char* settings );
	virtual const char* SaveSettings();

	
	Channel mInputChannel;
	U32 mBitRate;

protected:
	std::auto_ptr< AnalyzerSettingInterfaceChannel >	mInputChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceInteger >	mBitRateInterface;
};

#endif //HDMICEC_ANALYZER_SETTINGS
