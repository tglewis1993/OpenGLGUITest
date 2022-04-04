#include "Modules\Graphics\VideoWriter.h"

#include <functional>
#include <iostream>

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>

template <class T> void SafeRelease(T** ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

int FailInitSafely(const char* message, const HRESULT& code, IMFSinkWriter* sinkWriter, IMFMediaType* mediaTypeOut, IMFMediaType* mediaTypeIn)
{
	std::cout << "VideoWriter Init Failed - " << message << "(Error code: " << code << ")" << std::endl;

	SafeRelease(&sinkWriter);
	SafeRelease(&mediaTypeOut);
	SafeRelease(&mediaTypeIn);

	return -1;
}

int FailWriteFrameSafely(const char* message, const HRESULT& code, IMFSinkWriter* sinkWriter, IMFMediaType* mediaTypeOut, IMFMediaType* mediaTypeIn)
{
	std::cout << "VideoWriter Init Failed - " << message << "(Error code: " << code << ")" << std::endl;

	SafeRelease(&sinkWriter);
	SafeRelease(&mediaTypeOut);
	SafeRelease(&mediaTypeIn);

	return -1;
}

int VideoWriter::Start()
{
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	if (SUCCEEDED(hr))
	{
		hr = MFStartup(MF_VERSION);
		if (SUCCEEDED(hr))
		{
			m_Started = true;
		}
	}

	return 0;
}

int VideoWriter::Tick()
{
	return 0;
}

int VideoWriter::End()
{
	if (m_Started)
	{
		SafeRelease(&m_SinkWriter);
		MFShutdown();

		CoUninitialize();

		m_Started = false;
	}

	return 0;
}

int VideoWriter::Init(const char* filePath, const int& frameWidth, const int& frameHeight, const int& frameRate, const int& dur, const int& bitRate)
{
	SafeRelease(&m_SinkWriter);

	bool success = true;

	SetFilePath(filePath);

	m_FrameRate = frameRate;
	m_FrameWidth = frameWidth;
	m_FrameHeight = frameHeight;
	m_FrameDur = 10 * 1000 * 1000 / m_FrameRate;
	m_BitRate = bitRate;
	m_VideoEncFormat = MFVideoFormat_WMV3;
	m_VideoInFormat = MFVideoFormat_RGB32;
	m_VideoArea = m_FrameWidth * m_FrameHeight;

	IMFSinkWriter* sinkWriter = NULL;
	IMFMediaType* mediaTypeOut = NULL;
	IMFMediaType* mediaTypeIn = NULL;
	DWORD streamIndex;

	HRESULT res = MFCreateSinkWriterFromURL(m_FilePath.c_str(), NULL, NULL, &sinkWriter);

	if (SUCCEEDED(res))
	{
		res = MFCreateMediaType(&mediaTypeOut);
	}
	else
	{
		return FailInitSafely("Couldn't create Sink Writer!", res, sinkWriter, mediaTypeOut, mediaTypeIn);
	}

	if (SUCCEEDED(res))
	{
		res = mediaTypeOut->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
	}
	else
	{
		return FailInitSafely("Couldn't create out media type!", res, sinkWriter, mediaTypeOut, mediaTypeIn);
	}

	// SET VIDEO ENCODING FORMAT 
	if (SUCCEEDED(res))
	{
		res = mediaTypeOut->SetGUID(MF_MT_SUBTYPE, m_VideoEncFormat);
	}
	else
	{
		return FailInitSafely("Couldn't set out media major type!", res, sinkWriter, mediaTypeOut, mediaTypeIn);
	}

	// SET BITRATE
	if (SUCCEEDED(res))
	{
		res = mediaTypeOut->SetUINT32(MF_MT_AVG_BITRATE, m_BitRate);
	}
	else
	{
		return FailInitSafely("Couldn't set out media video encoding format!", res, sinkWriter, mediaTypeOut, mediaTypeIn);
	}

	if (SUCCEEDED(res))
	{
		res = mediaTypeOut->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
	}
	else
	{
		return FailInitSafely("Couldn't set bitrate!", res, sinkWriter, mediaTypeOut, mediaTypeIn);
	}

	if (SUCCEEDED(res))
	{
		res = MFSetAttributeSize(mediaTypeOut, MF_MT_FRAME_SIZE, m_FrameWidth, m_FrameHeight);
	}
	else
	{
		return FailInitSafely("Couldn't set interlace mode!", res, sinkWriter, mediaTypeOut, mediaTypeIn);
	}

	if (SUCCEEDED(res))
	{
		res = MFSetAttributeRatio(mediaTypeOut, MF_MT_FRAME_RATE, m_FrameRate, 1);
	}
	else
	{
		return FailInitSafely("Couldn't set frame size!", res, sinkWriter, mediaTypeOut, mediaTypeIn);
	}

	if (SUCCEEDED(res))
	{
		res = MFSetAttributeRatio(mediaTypeOut, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
	}
	else
	{
		return FailInitSafely("Couldn't set frame rate!", res, sinkWriter, mediaTypeOut, mediaTypeIn);
	}

	if (SUCCEEDED(res))
	{
		res = sinkWriter->AddStream(mediaTypeOut, &streamIndex);
	}
	else
	{
		return FailInitSafely("Couldn't set aspect ratio!", res, sinkWriter, mediaTypeOut, mediaTypeIn);
	}

	// Set the input media type.
	if (SUCCEEDED(res))
	{
		res = MFCreateMediaType(&mediaTypeIn);
	}
	else
	{
		return FailInitSafely("Couldn't add stream to sink writer!", res, sinkWriter, mediaTypeOut, mediaTypeIn);
	}

	if (SUCCEEDED(res))
	{
		res = mediaTypeIn->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
	}
	else
	{
		return FailInitSafely("Couldn't create in media type!", res, sinkWriter, mediaTypeOut, mediaTypeIn);
	}

	if (SUCCEEDED(res))
	{
		res = mediaTypeIn->SetGUID(MF_MT_SUBTYPE, m_VideoInFormat);
	}
	else
	{
		return FailInitSafely("Couldn't set in media major type!", res, sinkWriter, mediaTypeOut, mediaTypeIn);
	}

	if (SUCCEEDED(res))
	{
		res = mediaTypeIn->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
	}
	else
	{
		return FailInitSafely("Couldn't set in media format", res, sinkWriter, mediaTypeOut, mediaTypeIn);
	}

	if (SUCCEEDED(res))
	{
		res = MFSetAttributeSize(mediaTypeIn, MF_MT_FRAME_SIZE, m_FrameWidth, m_FrameHeight);
	}
	else
	{
		return FailInitSafely("Couldn't set in media interlace mode!", res, sinkWriter, mediaTypeOut, mediaTypeIn);
	}

	if (SUCCEEDED(res))
	{
		res = MFSetAttributeRatio(mediaTypeIn, MF_MT_FRAME_RATE, m_FrameRate, 1);
	}
	else
	{
		return FailInitSafely("Couldn't set in media frame size!", res, sinkWriter, mediaTypeOut, mediaTypeIn);
	}

	if (SUCCEEDED(res))
	{
		res = MFSetAttributeRatio(mediaTypeIn, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
	}
	else
	{
		return FailInitSafely("Couldn't set in media frame rate!", res, sinkWriter, mediaTypeOut, mediaTypeIn);
	}

	if (SUCCEEDED(res))
	{
		res = sinkWriter->SetInputMediaType(streamIndex, mediaTypeIn, NULL);
	}
	else
	{
		return FailInitSafely("Couldn't set in media aspect ratio!", res, sinkWriter, mediaTypeOut, mediaTypeIn);
	}

	// Tell the sink writer to start accepting data.
	if (SUCCEEDED(res))
	{
		res = sinkWriter->BeginWriting();
	}
	else
	{
		return FailInitSafely("Couldn't set sink writer input media type!", res, sinkWriter, mediaTypeOut, mediaTypeIn);
	}

	// Return the pointer to the caller.
	if (SUCCEEDED(res))
	{
		m_SinkWriter = sinkWriter;
		m_SinkWriter->AddRef();
		m_StreamIndex = std::make_shared<DWORD>(streamIndex);

		m_Initialised = true;
	}
	else
	{
		return FailInitSafely("Couldn't begin writing!", res, sinkWriter, mediaTypeOut, mediaTypeIn);
	}

	SafeRelease(&sinkWriter);
	SafeRelease(&mediaTypeOut);
	SafeRelease(&mediaTypeIn);

	return 0;
}

bool VideoWriter::WriteFrame(void* frameData, const long long& timestamp)
{
	bool success = true;

	if (m_Initialised)
	{
		IMFSample* pSample = NULL;
		IMFMediaBuffer* frameBuffer = NULL;

		const LONG stride = 4 * m_FrameWidth;
		const DWORD bufferLength = stride * m_FrameHeight;

		BYTE* destBuffer = NULL;

		// Create a new memory buffer.
		HRESULT hr = MFCreateMemoryBuffer(bufferLength, &frameBuffer);

		// Lock the buffer and copy the video frame to the buffer.
		if (SUCCEEDED(hr) && success)
		{
			hr = frameBuffer->Lock(&destBuffer, NULL, NULL);
		}
		else
		{
			std::cout << "Video Writer - WriteFrame failed! Error creating frame buffer (err code: " << std::to_string(hr) << ")" << std::endl;
			success = false;
		}

		if (SUCCEEDED(hr) && success)
		{
			hr = MFCopyImage(
				destBuffer,
				stride,
				(BYTE*)frameData + (m_FrameHeight - 1) * stride,    // First row in source image.
				stride * -1,                    // Source stride.
				stride,                    // Image width in bytes.
				m_FrameHeight                // Image height in pixels.
			);
		}
		else
		{
			std::cout << "Video Writer - WriteFrame failed! Error locking frame buffer (err code: " << std::to_string(hr) << ")" << std::endl;
			success = false;
		}

		if (frameBuffer)
		{
			frameBuffer->Unlock();
		}

		// Set the data length of the buffer.
		if (SUCCEEDED(hr) && success)
		{
			hr = frameBuffer->SetCurrentLength(bufferLength);
		}
		else
		{
			std::cout << "Video Writer - WriteFrame failed! Error copying frame (err code: " << std::to_string(hr) << ")" << std::endl;
			success = false;
		}

		// Create a media sample and add the buffer to the sample.
		if (SUCCEEDED(hr) && success)
		{
			hr = MFCreateSample(&pSample);
		}
		else
		{
			std::cout << "Video Writer - WriteFrame failed! Error setting frame buffer length (err code: " << std::to_string(hr) << ")" << std::endl;
			success = false;
		}

		if (SUCCEEDED(hr) && success)
		{
			hr = pSample->AddBuffer(frameBuffer);
		}
		else
		{
			std::cout << "Video Writer - WriteFrame failed! Error creating sample (err code: " << std::to_string(hr) << ")" << std::endl;
			success = false;
		}

		// Set the time stamp and the duration.
		if (SUCCEEDED(hr) && success)
		{
			hr = pSample->SetSampleTime(timestamp);
		}
		else
		{
			std::cout << "Video Writer - WriteFrame failed! Error adding frame buffer to sample (err code: " << std::to_string(hr) << ")" << std::endl;
			success = false;
		}

		if (SUCCEEDED(hr) && success)
		{
			hr = pSample->SetSampleDuration(m_FrameDur);
		}
		else
		{
			std::cout << "Video Writer - WriteFrame failed! Error setting sample timestamp (err code: " << std::to_string(hr) << ")" << std::endl;
			success = false;
		}

		// Send the sample to the Sink Writer.
		if (SUCCEEDED(hr) && success)
		{
			hr = m_SinkWriter->WriteSample(*m_StreamIndex, pSample);
		}
		else
		{
			std::cout << "Video Writer - WriteFrame failed! Error setting sample duration (err code: " << std::to_string(hr) << ")" << std::endl;
			success = false;
		}

		if (!SUCCEEDED(hr) && success)
		{
			std::cout << "Video Writer - WriteFrame failed! Error writing sample (err code: " << std::to_string(hr) << ")" << std::endl;
			success = false;
		}

		SafeRelease(&pSample);
		SafeRelease(&frameBuffer);
	}
	else
	{
		std::cout << "VideoWriter - Not initialised!" << std::endl;
		success = false;
	}

	return success;
}

void VideoWriter::SetFilePath(const char* filePath)
{
	std::string stemp = filePath;

	m_FilePath = std::wstring(stemp.begin(), stemp.end());
}

void VideoWriter::WriteAllFrames(std::vector<void*> frames)
{
	if (!m_Writing)
	{
		m_Writing = true;

		bool success = true;
		long long timestamp = 0;

		m_FrameCount = frames.size();

		if (m_FrameCount == 0)
		{
			std::cout << "Input frame container is empty!" << std::endl;
		}

		// foreach frame, write
		for (size_t i = 0; i < m_FrameCount; ++i)
		{
			if (frames[i] != nullptr)
			{
				if (!WriteFrame(frames[i], timestamp))
				{
					std::cout << "Failed to write frame " << i << "!" << std::endl;

					success = false;

					break;
				}

				timestamp += m_FrameDur;
			}
		}

		if (success)
		{
			m_SinkWriter->Finalize();
		}

		// must reinit
		m_Initialised = false;
		m_Writing = false;
	}
}

VideoWriter::~VideoWriter()
{
	//if (m_Started)
	//{
	//	SafeRelease(&m_SinkWriter);
	//	MFShutdown();

	//	CoUninitialize();

	//	m_Started = false;
	//}
}
