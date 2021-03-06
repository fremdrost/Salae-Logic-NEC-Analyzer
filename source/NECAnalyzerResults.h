#ifndef NEC_ANALYZER_RESULTS
#define NEC_ANALYZER_RESULTS

#include <AnalyzerResults.h>

class NECAnalyzer;
class NECAnalyzerSettings;

class NECAnalyzerResults : public AnalyzerResults
{
public:
	NECAnalyzerResults( NECAnalyzer* analyzer, NECAnalyzerSettings* settings );
	virtual ~NECAnalyzerResults();

	virtual void GenerateBubbleText( U64 frame_index, Channel& channel, DisplayBase display_base );
	virtual void GenerateExportFile( const char* file, DisplayBase display_base, U32 export_type_user_id );

	virtual void GenerateFrameTabularText(U64 frame_index, DisplayBase display_base );
	virtual void GeneratePacketTabularText( U64 packet_id, DisplayBase display_base );
	virtual void GenerateTransactionTabularText( U64 transaction_id, DisplayBase display_base );

	enum FrameType { AddressFrame, ExtAddressFrame, CommandFrame, RepeatFrame, AgcFrame};

protected: //functions

protected:  //vars
	NECAnalyzerSettings* mSettings;
	NECAnalyzer* mAnalyzer;
};

#endif //NEC_ANALYZER_RESULTS
