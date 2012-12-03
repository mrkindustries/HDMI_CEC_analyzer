#include "HdmiCecAnalyzerSettings.h"
#include <AnalyzerHelpers.h>

const std::string HdmiCecAnalyzerSettings::mCecChannelName = "HDMI CEC";

HdmiCecAnalyzerSettings::HdmiCecAnalyzerSettings()
:	mCecChannel( UNDEFINED_CHANNEL )
{
    mCecChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
    mCecChannelInterface->SetTitleAndTooltip( "CEC", "HDMI Consumer Electronics Control (CEC)" );
    mCecChannelInterface->SetChannel( mCecChannel );

    AddInterface( mCecChannelInterface.get() );

    AddExportOption( 0, "Export as text/csv file" );
    AddExportExtension( 0, "text", "txt" );
    AddExportExtension( 0, "csv", "csv" );

    ClearChannels();
    AddChannel( mCecChannel, mCecChannelName, false );
}

HdmiCecAnalyzerSettings::~HdmiCecAnalyzerSettings()
{
}

bool HdmiCecAnalyzerSettings::SetSettingsFromInterfaces()
{
    mCecChannel = mCecChannelInterface->GetChannel();

    ClearChannels();
    AddChannel( mCecChannel, mCecChannelName, true );

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
    AddChannel( mCecChannel, mCecChannelName, true );

    UpdateInterfacesFromSettings();
}

const char* HdmiCecAnalyzerSettings::SaveSettings()
{
    SimpleArchive text_archive;

    text_archive << mCecChannel;

    return SetReturnString( text_archive.GetString() );
}
