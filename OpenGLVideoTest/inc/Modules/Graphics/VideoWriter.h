#pragma once
#include <Modules/ModulePart.h>

#include <windows.h>
#include <memory>
#include <guiddef.h>

#include <string>
#include <vector>

struct IMFSinkWriter;

class VideoWriter : public ModulePart
{
public:
	int Start() override;
	int Tick() override;
	int End() override;

private:
	std::wstring m_FilePath = L"";
	int m_FrameRate = 60;
	int m_FrameWidth = 640;
	int m_FrameHeight = 480;
	long m_FrameDur = 10 * 1000 * 1000 / 60;
	int m_BitRate = 80000;
	GUID m_VideoEncFormat;
	GUID m_VideoInFormat;
	unsigned int m_VideoArea = 640 * 480;
	unsigned int m_FrameCount = 20 * 60;

	void* m_FrameData;

	IMFSinkWriter* m_SinkWriter;
	std::shared_ptr<DWORD> m_StreamIndex;

	bool m_Started = false;
	bool m_Initialised = false;
	bool m_Writing = false;
	
	bool WriteFrame(void*, const long long&);

	void SetFilePath(const char* filePath);

public:
	int Init(const char* filePath, const int& frameWidth, const int& frameHeight, const int& frameRate, const int& dur, const int& bitRate);

	void WriteAllFrames(std::vector<void*> frames);

    bool IsInit() { return m_Initialised; }
	bool IsWriting() { return m_Writing; }

	~VideoWriter();
	
};