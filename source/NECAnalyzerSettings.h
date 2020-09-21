#ifndef NEC_ANALYZER_SETTINGS
#define NEC_ANALYZER_SETTINGS

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

class NECAnalyzerSettings : public AnalyzerSettings
{
public:
	NECAnalyzerSettings();
	virtual ~NECAnalyzerSettings();

	virtual bool SetSettingsFromInterfaces();
	void UpdateInterfacesFromSettings();
	virtual void LoadSettings( const char* settings );
	virtual const char* SaveSettings();

	
	Channel mInputChannel;
	U32 mPreTimeMark;
	U32 mPreTimeSpace;
	U32 mRepeatTimeMark;
	U32 mRepeatTimeSpace;
	U32 mMark;
	U32 mOneSpace;
	U32 mZeroSpace;
	U32 mError;

protected:
	std::auto_ptr< AnalyzerSettingInterfaceChannel >	mInputChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceInteger >	mPreTimeMarkInterface;
	std::auto_ptr< AnalyzerSettingInterfaceInteger >	mPreTimeSpaceInterface;
	std::auto_ptr< AnalyzerSettingInterfaceInteger >	mRepeatTimeMarkInterface;
	std::auto_ptr< AnalyzerSettingInterfaceInteger >	mRepeatTimeSpaceInterface;
	std::auto_ptr< AnalyzerSettingInterfaceInteger >	mMarkInterface;
	std::auto_ptr< AnalyzerSettingInterfaceInteger >	mOneSpaceInterface;
	std::auto_ptr< AnalyzerSettingInterfaceInteger >	mZeroSpaceInterface;
	std::auto_ptr< AnalyzerSettingInterfaceInteger >	mErrorInterface;
};

#endif //NEC_ANALYZER_SETTINGS
