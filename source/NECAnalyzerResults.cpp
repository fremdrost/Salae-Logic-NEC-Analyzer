#include "NECAnalyzerResults.h"
#include <AnalyzerHelpers.h>
#include "NECAnalyzer.h"
#include "NECAnalyzerSettings.h"
#include <iostream>
#include <fstream>

NECAnalyzerResults::NECAnalyzerResults( NECAnalyzer* analyzer, NECAnalyzerSettings* settings )
:	AnalyzerResults(),
	mSettings( settings ),
	mAnalyzer( analyzer )
{
}

NECAnalyzerResults::~NECAnalyzerResults()
{
}

void NECAnalyzerResults::GenerateBubbleText( U64 frame_index, Channel& channel, DisplayBase display_base )
{
	ClearResultStrings();
	Frame frame = GetFrame( frame_index );
	if (frame.mType == RepeatFrame) {
		AddResultString("Repeat");
		return;
	}
	else if (frame.mType == AgcFrame) {
		AddResultString("AGC");
		return;
	}

	U8 base = 8;
	U64 data = 0;
	char prefix[40];
	char data_str[128];

	switch(frame.mType){
	case ExtAddressFrame:
		if (display_base == Binary) strcpy(prefix, "ExA");
		else strcpy(prefix, "ExAddress");
		base = 16;
		data = (frame.mData1 << 8) | frame.mData2;
		break;
	case AddressFrame:
		if (display_base == Binary) strcpy(prefix, "A");
		else strcpy(prefix, "Address");
		base = 8;
		data = frame.mData1;
		break;
	case CommandFrame:
		if (display_base == Binary) strcpy(prefix, "C");
		else strcpy(prefix, "Command");
		base = 16;
		data = frame.mData1 << 8 | frame.mData2;
		break;
	}

	AnalyzerHelpers::GetNumberString(data, display_base, base, data_str, 128);
	
	AddResultString(prefix, ": ", data_str);
}

void NECAnalyzerResults::GenerateExportFile( const char* file, DisplayBase display_base, U32 export_type_user_id )
{
	std::ofstream file_stream( file, std::ios::out );

	U64 trigger_sample = mAnalyzer->GetTriggerSample();
	U32 sample_rate = mAnalyzer->GetSampleRate();

	file_stream << "Time [s],Address,Command" << std::endl;

	U64 num_frames = GetNumFrames();
	U32 i;
	for(i = 0; i < num_frames; i++ )
	{
		UpdateExportProgressAndCheckForCancel(i, num_frames);

		Frame frame = GetFrame( i );
		if (frame.mType != ExtAddressFrame && frame.mType != AddressFrame) {
			continue;
		}

		Frame frameNext;
		if (i + 1 == num_frames || (frameNext = GetFrame(i + 1)).mType != CommandFrame) {
			continue;
		}

		char time_str[128];
		AnalyzerHelpers::GetTimeString( frame.mStartingSampleInclusive, trigger_sample, sample_rate, time_str, 128 );

		U8 base = 8;
		U64 data = 0;
		char address_str[128];
		char command_str[128];

		switch (frame.mType) {
		case ExtAddressFrame:
			base = 16;
			data = frame.mData1 << 8 | frame.mData2;
			break;
		case AddressFrame:
			base = 8;
			data = frame.mData1;
			break;
		}

		AnalyzerHelpers::GetNumberString(data, display_base, base, address_str, 128);
		AnalyzerHelpers::GetNumberString(frameNext.mData1, display_base, 16, command_str, 128);

		file_stream << time_str << "," << address_str << "," << command_str << std::endl;

		if( UpdateExportProgressAndCheckForCancel( i, num_frames) == true )
		{
			file_stream.close();
			return;
		}
	}
	
	file_stream.close();
}

void NECAnalyzerResults::GenerateFrameTabularText( U64 frame_index, DisplayBase display_base )
{
#ifdef SUPPORTS_PROTOCOL_SEARCH
	ClearTabularText();

	Frame frame = GetFrame(frame_index);
	if (frame.mType == RepeatFrame) {
		AddTabularText("Repeat");
		return;
	}
	else if (frame.mType == AgcFrame) {
		AddTabularText("AGC");
		return;
	}

	U8 base = 8;
	U64 data = 0;
	char prefix[40];
	char data_str[128];

	switch (frame.mType) {
	case ExtAddressFrame:
		if (display_base == Binary) strcpy(prefix, "ExA");
		else strcpy(prefix, "ExAddress");
		base = 16;
		data = frame.mData1 << 8 | frame.mData2;
		break;
	case AddressFrame:
		if (display_base == Binary) strcpy(prefix, "A");
		else strcpy(prefix, "Address");
		base = 8;
		data = frame.mData1;
		break;
	case CommandFrame:
		if (display_base == Binary) strcpy(prefix, "C");
		else strcpy(prefix, "Command");
		base = 16;
		data = frame.mData1 << 8 | frame.mData2;
		break;
	}

	AnalyzerHelpers::GetNumberString(data, display_base, base, data_str, 128);

	AddTabularText(prefix, ": ", data_str);
	//AddTabularText( number_str );
#endif
}

void NECAnalyzerResults::GeneratePacketTabularText( U64 packet_id, DisplayBase display_base )
{
	//not supported
}

void NECAnalyzerResults::GenerateTransactionTabularText( U64 transaction_id, DisplayBase display_base )
{
	//not supported
}