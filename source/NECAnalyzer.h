#ifndef NEC_ANALYZER_H
#define NEC_ANALYZER_H

#include <Analyzer.h>
#include "NECAnalyzerResults.h"
#include "NECSimulationDataGenerator.h"

class NECAnalyzerSettings;
class ANALYZER_EXPORT NECAnalyzer : public Analyzer2
{
public:
	NECAnalyzer();
	virtual ~NECAnalyzer();

	virtual void SetupResults();
	virtual void WorkerThread();

	virtual U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels );
	virtual U32 GetMinimumSampleRateHz();

	virtual const char* GetAnalyzerName() const;
	virtual bool NeedsRerun();

protected: //func
	U64 UsToSample(U64 us);
	U64 SamplesToUs(U64 samples);

	bool GetNextByte(U8& byte);
	bool GetNextBit(bool& bit);

protected: //vars
	std::auto_ptr< NECAnalyzerSettings > mSettings;
	std::auto_ptr< NECAnalyzerResults > mResults;
	AnalyzerChannelData* mNEC;

	NECSimulationDataGenerator mSimulationDataGenerator;
	bool mSimulationInitilized;

	//NEC analysis vars:
	U32 mSampleRateHz;
	U32 mTRepeatMark;
	U32 mTRepeatSpace;
	U32 mTAGCMark;
	U32 mTAGCSpace;
	// Bit marker
	U32 mTMark;
	// Space for 0 Bit
	U32 mTSpace0;
	// Space for 1 Bit
	U32 mTSpace1;
	U32 mTError;
	bool mSynchronised;
	std::vector<bool> mBitsForNextByte; //value
};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer( );
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer( Analyzer* analyzer );

#endif //NEC_ANALYZER_H


// https://support.saleae.com/protocol-analyzers/unofficially-supported-protocols
// https://github.com/kodizhuk/Salae-Logic-NEC-Analyzer
// https://www.sbprojects.net/knowledge/ir/nec.php