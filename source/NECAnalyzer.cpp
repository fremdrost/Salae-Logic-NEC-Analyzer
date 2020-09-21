#include "NECAnalyzer.h"
#include "NECAnalyzerSettings.h"
#include <AnalyzerChannelData.h>

NECAnalyzer::NECAnalyzer()
:	Analyzer2(),  
	mSettings( new NECAnalyzerSettings() ),
	mSimulationInitilized( false )
{
	SetAnalyzerSettings( mSettings.get() );
}

NECAnalyzer::~NECAnalyzer()
{
	KillThread();
}

void NECAnalyzer::SetupResults()
{
	mResults.reset( new NECAnalyzerResults( this, mSettings.get() ) );
	SetAnalyzerResults( mResults.get() );
	mResults->AddChannelBubblesWillAppearOn( mSettings->mInputChannel );
}

void NECAnalyzer::WorkerThread()
{
	mSampleRateHz = GetSampleRate();

	mNEC = GetAnalyzerChannelData( mSettings->mInputChannel );

	//init parametrs
	mTAGCMark = mSettings->mPreTimeMark;
	mTAGCSpace = mSettings->mPreTimeSpace;
	mTRepeatMark = mSettings->mRepeatTimeMark;
	mTRepeatSpace = mSettings->mRepeatTimeSpace;
	mTMark = mSettings->mMark;
	mTSpace0 = mSettings->mZeroSpace;
	mTSpace1 = mSettings->mOneSpace;
	mTError = mSettings->mError;
	mSynchronised = false;

	mBitsForNextByte.clear();

	U64 edge_location = 0, frame_start_location, frame_end_location;
	for( ; ; )
	{
		//find first AGC pulse
		mNEC->AdvanceToNextEdge(); //falling

		ReportProgress(mNEC->GetSampleNumber());
		CheckIfThreadShouldExit();

		// expect falling
		if (mNEC->GetBitState() == BIT_LOW)
		{
			edge_location = mNEC->GetSampleNumber();
		}
		else {
			// sync to falling edge
			continue;
		}

		U64 next_edge_location = mNEC->GetSampleOfNextEdge(); //peak for rising
		U64 edge_distance = next_edge_location - edge_location;
		if ((edge_distance < UsToSample((U64)mTAGCMark + mTError)) && (edge_distance > UsToSample(mTAGCMark - mTError)))
		{
			mNEC->AdvanceToNextEdge();	//rising

			next_edge_location = mNEC->GetSampleOfNextEdge(); //peak for falling
			edge_distance = next_edge_location - edge_location;

			//mResults->CancelPacketAndStartNewPacket();

			// Repeat frame
			if ((edge_distance < UsToSample((U64)mTRepeatMark + mTRepeatSpace + mTError))
				&& (edge_distance > UsToSample((U64)mTRepeatMark + mTRepeatSpace - mTError)))
			{
				Frame frame;
				frame.mStartingSampleInclusive = edge_location;
				frame.mEndingSampleInclusive = next_edge_location;
				frame.mType = NECAnalyzerResults::RepeatFrame;
				mResults->AddFrame(frame);

				//mResults->CommitPacketAndStartNewPacket();
				mResults->CommitResults();
				continue; // next frame
			} else
			// Command frame
			if ((edge_distance < UsToSample((U64)mTAGCMark + mTAGCSpace + mTError))
				&& (edge_distance > UsToSample((U64)mTAGCMark + mTAGCSpace - mTError)))
			{
				mSynchronised = true;
				Frame frame;

				frame_start_location = next_edge_location;
				// mark AGC pulse
				mResults->AddMarker(edge_location, AnalyzerResults::Start, mSettings->mInputChannel);
				mResults->AddMarker(next_edge_location, AnalyzerResults::Stop, mSettings->mInputChannel);

				frame.mStartingSampleInclusive = edge_location;
				frame.mEndingSampleInclusive = next_edge_location;
				frame.mType = NECAnalyzerResults::AgcFrame;
				mResults->AddFrame(frame);

				mNEC->AdvanceToNextEdge();

				U8 low, high;
				// receive address
				if (!GetNextByte(low)) {
					mResults->AddMarker(mNEC->GetSampleNumber(), AnalyzerResults::ErrorX, mSettings->mInputChannel);
					mResults->CommitResults();
					continue;
				}
				if (!GetNextByte(high)) {
					mResults->AddMarker(mNEC->GetSampleNumber(), AnalyzerResults::ErrorSquare, mSettings->mInputChannel);
					mResults->CommitResults();
					continue;
				}
				if (low ^ (0xFF ^ high)) {
					frame.mData1 = high;
					frame.mData2 = low;
					frame.mType = NECAnalyzerResults::ExtAddressFrame;
				}
				else {
					frame.mData1 = low;
					frame.mType = NECAnalyzerResults::AddressFrame;
				}
				frame.mStartingSampleInclusive = frame_start_location;
				frame.mEndingSampleInclusive = frame_end_location = mNEC->GetSampleNumber();
				mResults->AddFrame(frame);

				// receive command
				if (!GetNextByte(high)) {
					mResults->AddMarker(mNEC->GetSampleNumber(), AnalyzerResults::ErrorSquare, mSettings->mInputChannel);
					mResults->CommitResults();
					continue;
				}
				if (!GetNextByte(low)) {
					mResults->AddMarker(mNEC->GetSampleNumber(), AnalyzerResults::ErrorX, mSettings->mInputChannel);
					mResults->CommitResults();
					continue;
				}
				frame.mData1 = (U16)high;
				frame.mData2 = (U16)low;
				frame.mStartingSampleInclusive = frame_end_location;
				frame.mEndingSampleInclusive = mNEC->GetSampleNumber();
				frame.mType = NECAnalyzerResults::CommandFrame;
				mResults->AddFrame(frame);

				//mResults->CommitPacketAndStartNewPacket();
				mResults->CommitResults();
			}
			else {
				continue; // error
			}
		}
		else {
			continue; // error
		}
	}
}

U64 NECAnalyzer::UsToSample(U64 us)
{
	return (mSampleRateHz * us) / 1000000;
}

U64 NECAnalyzer::SamplesToUs(U64 samples)
{
	return(samples * 1000000) / mSampleRateHz;
}

bool NECAnalyzer::GetNextByte(U8& byte) {
	bool bit;
	byte = 0;
	while (mSynchronised) {
		mNEC->AdvanceToNextEdge();

		if (!GetNextBit(bit)) {
			mSynchronised = false;
			mBitsForNextByte.clear();
			mNEC->AdvanceToNextEdge();
			return false;
		}
		else {
			mNEC->AdvanceToNextEdge();
			mBitsForNextByte.push_back(bit);
		}
		if (mBitsForNextByte.size() == 8) {
			for(U8 i = 0; i<8 ; i++)
				byte |= (mBitsForNextByte[i] << i);
			mBitsForNextByte.clear();
			return true;
		}
	}
	mBitsForNextByte.clear();
	return false;
}

bool NECAnalyzer::GetNextBit(bool& bit) {
	bit = 0;
	U64 edge_location = mNEC->GetSampleNumber();
	U64 next_edge_location = mNEC->GetSampleOfNextEdge();
	U64 edge_distance = next_edge_location - edge_location;
	if ((edge_distance < UsToSample((U64)mTSpace0 + mTError))
		&& (edge_distance > UsToSample((U64)mTSpace0 - mTError))) {
		bit = 0;
		return true;
	}
	else if ((edge_distance < UsToSample((U64)mTSpace1 + mTError))
		&& (edge_distance > UsToSample((U64)mTSpace1 - mTError))) {
		bit = 1;
		return true;
	}
	else {
		return false;
	}
}

bool NECAnalyzer::NeedsRerun()
{
	return false;
}

U32 NECAnalyzer::GenerateSimulationData( U64 minimum_sample_index, U32 device_sample_rate, SimulationChannelDescriptor** simulation_channels )
{
	if( mSimulationInitilized == false )
	{
		mSimulationDataGenerator.Initialize( GetSimulationSampleRate(), mSettings.get() );
		mSimulationInitilized = true;
	}

	return mSimulationDataGenerator.GenerateSimulationData( minimum_sample_index, device_sample_rate, simulation_channels );
}

U32 NECAnalyzer::GetMinimumSampleRateHz()
{
	return 3500;	//3.5kHz
}

const char* NECAnalyzer::GetAnalyzerName() const
{
	return "NEC Analyzer";
}

const char* GetAnalyzerName()
{
	return "NEC Analyzer";
}

Analyzer* CreateAnalyzer()
{
	return new NECAnalyzer();
}

void DestroyAnalyzer( Analyzer* analyzer )
{
	delete analyzer;
}