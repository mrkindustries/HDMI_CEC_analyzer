#include "HdmiCecAnalyzerSettings.h"
#include <AnalyzerHelpers.h>

#include "HdmiCecProtocol.h"

HdmiCecAnalyzerSettings::HdmiCecAnalyzerSettings()
:	mCecChannel( UNDEFINED_CHANNEL )
{
    mCecChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
    mCecChannelInterface->SetTitleAndTooltip( HdmiCec::Channel1Name, HdmiCec::ProtocolName );
    mCecChannelInterface->SetChannel( mCecChannel );

    AddInterface( mCecChannelInterface.get() );

    AddExportOption( 0, "Export as text/csv file" );
    AddExportExtension( 0, "text", "txt" );
    AddExportExtension( 0, "csv", "csv" );

    ClearChannels();
    AddChannel( mCecChannel, HdmiCec::ProtocolName, false );
}

HdmiCecAnalyzerSettings::~HdmiCecAnalyzerSettings()
{
}

bool HdmiCecAnalyzerSettings::SetSettingsFromInterfaces()
{
    mCecChannel = mCecChannelInterface->GetChannel();

    ClearChannels();
    AddChannel( mCecChannel, HdmiCec::ProtocolName, true );

    return true;
}

void HdmiCecAnalyzerSettings::UpdateInterfacesFromSettings()
{
    mCecChannelInterface->SetChannel( mCecChannel );
}

void HdmiCecAnalyzerSettings::LoadSettings( const char* settings )
{
    SimpleArchive text_archive;
    text_archive.SetString( settings );

    text_archive >> mCecChannel;

    ClearChannels();
    AddChannel( mCecChannel, HdmiCec::ProtocolName, true );

    UpdateInterfacesFromSettings();
}

const char* HdmiCecAnalyzerSettings::SaveSettings()
{
    SimpleArchive text_archive;

    text_archive << mCecChannel;

    return SetReturnString( text_archive.GetString() );
}
