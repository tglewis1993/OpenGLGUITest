#pragma once
#include <Modules/Module.h>

#include <memory>
#include <array>
#include <vector>
#include <string>
#include <future>

struct GLFWwindow;
struct ImVec4;
struct ImVec2;
struct ImGuiContext;
struct FIBITMAP;
class Renderer;

class Graphics : public Module
{
private:
	GLFWwindow* m_Window = nullptr;

	std::shared_ptr<Renderer> m_Renderer = nullptr;
	std::shared_ptr<ImGuiContext> m_ImGUIContext = nullptr;
	std::shared_ptr<ImVec4> m_ClearColour;
	std::shared_ptr<ImVec2> m_WindowSize;

	bool m_Init = false;

	char m_RenderFileName[32]{};
	char m_RenderFilePath[128]{};
	int m_RenderFileFPS = 30;
	int m_RenderFileTime = 10;
	int m_RenderFileDelay = 1;

	double m_RecordRefreshTime = 0.0;
	float m_RecordDelayTime = 0.0f;
	float m_RecordTime = 0.0f;

	bool m_Saving = false;
	bool m_Recording = false;

	//std::string m_PrintFilePathBase = "";

	char m_PrintFilePathBase[256]{0};
	char m_RenderFilePathBase[256]{0};

	unsigned int m_PixelBuffers[2]{};
	int m_ReadIndex = 0;
	int m_WriteIndex = 1;

	double m_CumulativeFrameTime = 0.0;

	unsigned int m_BufferSize = 0;

	std::vector<void*> m_StoredFrames{};

	std::future<void> m_VideoWriteTask;

public:
	Graphics() = default;

	int Start() override;
	int Tick() override;
	int End() override;

protected:

	void SetupParts() override;

private:
	int SetupGLFW();
	int SetupImGUI();

	void ShowMenuBar(bool& beginRendering, bool& showRenderScreen, bool& showCurrentlyRenderingScreen, bool& showPrintedScreen, bool& showStats, bool& closeShown);
	void ShowStatsWindow();
	int ShowRenderToFileWindow();
	int ShowExitWindow();
	int ShowOverwriteWindow();
	int ShowPrintCompleteWindow();
	int ShowCurrentlyRenderingWindow();
	void ShowRecordingStatus(const bool& isRecording, const bool& isWriting, const bool& isDelayed, const float& renderDelay);
	void ShowUsageTable();
	void ShowKnownIssuesTable();

	void PrintOpenGLInfo();
	void PrintGLFWInfo();
	void PrintGLEWInfo();
	void PrintSomethingFun();

};

