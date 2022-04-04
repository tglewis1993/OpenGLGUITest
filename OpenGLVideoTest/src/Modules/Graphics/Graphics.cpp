#include "Modules/Graphics/Graphics.h"

#include <imgui\imgui.h>
#include <imgui\imgui_impl_glfw.h>
#include <imgui\imgui_impl_opengl3.h>
#include <imgui\imgui_internal.h>

#include <glew/glew.h>
#include <GLFW/glfw3.h>

#include <Modules/Graphics/Renderer.h>
#include <Modules/Graphics/VideoWriter.h>

#include <iostream>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <future>

static void GLFW_ERROR_LOG(int error, const char* description)
{
	std::cout << "GLFW Error - " << description << "(" << error << ")" << std::endl;
}

int Graphics::Start()
{
	sprintf_s(m_RenderFilePathBase, "%s\\Videos", std::filesystem::current_path().string().c_str());
	sprintf_s(m_PrintFilePathBase, "%s\\Logs", std::filesystem::current_path().string().c_str());

	if (!std::filesystem::exists(m_RenderFilePathBase))
	{
		std::filesystem::create_directory(m_RenderFilePathBase);
	}

	if (!std::filesystem::exists(m_PrintFilePathBase))
	{
		std::filesystem::create_directory(m_PrintFilePathBase);
	}

	m_ClearColour = std::make_shared<ImVec4>(ImVec4(0.45f, 0.55f, 0.60f, 1.00f));
	m_WindowSize = std::make_shared<ImVec2>(ImVec2(1600.0f, 900.0f));

	m_BufferSize = m_WindowSize->x * m_WindowSize->y * 4;

	int err = SetupGLFW();

	if (err != 0)
	{
		return err;
	}

	SetupParts();

	err = SetupImGUI();

	if (err != 0)
	{
		return err;
	}

	ImGui::GetIO().MouseDrawCursor = true;

	glGenBuffers(2, m_PixelBuffers);

	glBindBuffer(GL_PIXEL_PACK_BUFFER, m_PixelBuffers[0]);
	glBufferData(GL_PIXEL_PACK_BUFFER, m_BufferSize, 0, GL_STREAM_READ);

	glBindBuffer(GL_PIXEL_PACK_BUFFER, m_PixelBuffers[1]);
	glBufferData(GL_PIXEL_PACK_BUFFER, m_BufferSize, 0, GL_STREAM_READ);

	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

	m_Init = true;

	return 0;
}



int Graphics::Tick()
{
	if (m_Init)
	{
		VideoWriter* vidWrite = (VideoWriter*)m_Parts[1].get();

		if (glfwWindowShouldClose(m_Window))
		{
			std::cout << "Graphics Module - Window close requested!" << std::endl;

			return 1;
		}

		glfwPollEvents();

		if (m_Parts.size() > 0)
		{
			for (std::shared_ptr<ModulePart> part : m_Parts)
			{
				part->Tick();
			}
		}

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		static bool closeShown = false;
		static bool closeRequested = false;

		static bool showStats = false;
		static bool showRenderScreen = false;

		static bool showOverwriteScreen = false;

		static bool showPrintedScreen = false;

		static bool showCurrentlyRenderingScreen = false;

		static bool beginRendering = false;
		static bool isDelayed = false;

		ImVec2 centre = ImGui::GetMainViewport()->GetWorkCenter();

		ImGui::SetNextWindowPos(centre, ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(ImVec2(1280, 640), ImGuiCond_FirstUseEver);
		ImGui::Begin("ImGui In Action!", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar);

		ImGuiWindow* mainWindow = ImGui::GetCurrentWindow();

		ImVec2 mainWindowMid = ImVec2(mainWindow->Size.x / 2.f, mainWindow->Size.y / 2.f);

		ShowMenuBar(beginRendering, showRenderScreen, showCurrentlyRenderingScreen, showPrintedScreen, showStats, closeShown);

		if (closeShown)
		{
			closeShown = false;

			if (mainWindow != nullptr)
			{
				ImGui::SetNextWindowPos(mainWindowMid, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			}

			ImGui::OpenPopup("Exiting Application...");
		}

		if (showRenderScreen)
		{
			showRenderScreen = false;

			if (mainWindow != nullptr)
			{
				ImGui::SetNextWindowPos(mainWindowMid, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			}

			ImGui::OpenPopup("Render To File...");
		}

		if (showOverwriteScreen)
		{
			showOverwriteScreen = false;

			if (mainWindow != nullptr)
			{
				ImGui::SetNextWindowPos(mainWindowMid, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			}

			ImGui::OpenPopup("Overwite existing file?");
		}

		if (showCurrentlyRenderingScreen)
		{
			showCurrentlyRenderingScreen = false;

			if (mainWindow != nullptr)
			{
				ImGui::SetNextWindowPos(mainWindowMid, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			}

			ImGui::OpenPopup("Rendering still in progress!");
		}

		if (showPrintedScreen)
		{
			showPrintedScreen = false;

			if (mainWindow != nullptr)
			{
				ImGui::SetNextWindowPos(mainWindowMid, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			}

			ImGui::OpenPopup("Print Complete!");
		}

		if (ShowExitWindow() == 1)
		{
			closeRequested = true;
		}

		if (ShowRenderToFileWindow() == 1)
		{
			if (std::filesystem::exists(m_RenderFilePath))
			{
				showOverwriteScreen = true;
			}
			else
			{
				beginRendering = true;
			}
		}

		if (ShowOverwriteWindow() == 1)
		{
			if (std::remove(m_RenderFilePath) == 0)
			{
				beginRendering = true;
			}
			else
			{
				std::cout << "Could not overwrite '" << m_RenderFilePath << "'!" << std::endl;
			}
		}

		ShowCurrentlyRenderingWindow();

		ShowPrintCompleteWindow();

		if (showStats)
			ShowStatsWindow();

		ShowRecordingStatus(m_Recording, m_Saving, isDelayed, m_RecordDelayTime);

		ShowUsageTable();

		ShowKnownIssuesTable();

		ImGui::Render();

		int display_w, display_h;
		glfwGetFramebufferSize(m_Window, &display_w, &display_h);

		glViewport(0, 0, display_w, display_h);

		glClearColor(m_ClearColour->x * m_ClearColour->w, m_ClearColour->y * m_ClearColour->w, m_ClearColour->z * m_ClearColour->w, m_ClearColour->w);
		glClear(GL_COLOR_BUFFER_BIT);

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (beginRendering)
		{
			if (m_RecordDelayTime < m_RenderFileDelay)
			{
				isDelayed = true;

				m_RecordDelayTime += ImGui::GetIO().DeltaTime;
			}
			else
			{
				isDelayed = false;

				m_RecordTime += ImGui::GetIO().DeltaTime;

				if (m_RecordTime < m_RenderFileTime)
				{
					m_Recording = true;

					if (m_RecordRefreshTime == 0.0)
						m_RecordRefreshTime = ImGui::GetTime();

					// render at desired frame rate
					while (m_RecordRefreshTime < ImGui::GetTime())
					{
						//swap read and write indices
						m_WriteIndex = (m_WriteIndex + 1) % 2;
						m_ReadIndex = (m_WriteIndex + 1) % 2;

						glBindBuffer(GL_PIXEL_PACK_BUFFER, m_PixelBuffers[m_WriteIndex]);
						glReadPixels(0, 0, m_WindowSize->x, m_WindowSize->y, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
						glBindBuffer(GL_PIXEL_PACK_BUFFER, m_PixelBuffers[m_ReadIndex]);

						void* mapData = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);

						if (mapData != nullptr)
						{
							void* frameCopy = malloc(m_BufferSize);

							if (memcpy_s(frameCopy, m_BufferSize, mapData, m_BufferSize) == 0)
							{
								m_StoredFrames.push_back(frameCopy);
							}

							glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
						}

						glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

						m_RecordRefreshTime += 1.0f / m_RenderFileFPS;
					}
				}
				else
				{
					if (vidWrite != nullptr && !m_Saving)
					{
						std::cout << m_StoredFrames.size() << " frames have been recorded, attempting to write to '" << m_RenderFileName << ".wmv'" << std::endl;

						m_Recording = false;

						std::cout << vidWrite << std::endl;

						if (vidWrite->Init(m_RenderFilePath, m_WindowSize->x, m_WindowSize->y, m_RenderFileFPS, m_RenderFileTime, 6000000) == 0)
						{
							m_VideoWriteTask = std::async(std::launch::async, &VideoWriter::WriteAllFrames, *vidWrite, m_StoredFrames);

							m_Saving = true;
						}
					}
					else
					{
						if (m_VideoWriteTask.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
						{
							beginRendering = false;
							m_Saving = false;

							std::cout << "'" << m_RenderFileName << ".wmv' has successfully saved! (Path: " << m_RenderFilePath << ")" << std::endl;

							m_RecordDelayTime = 0;
							m_RecordTime = 0;
							m_RecordRefreshTime = 0.0;

							for (auto frame : m_StoredFrames)
							{
								if (frame != nullptr);
								delete frame;
							}

							m_StoredFrames.clear();
						}
					}
				}
			}
		}

		glfwSwapBuffers(m_Window);

		if (closeRequested)
		{
			return 1;
		}
	}

	return 0;
}

int Graphics::End()
{
	/*if (m_ClearColour != nullptr)
		delete m_ClearColour;*/

	if (m_Parts.size() > 0)
	{
		for (std::shared_ptr<ModulePart> part : m_Parts)
		{
			part->End();
		}
	}

	if (m_Init)
	{
		for (void* frame : m_StoredFrames)
		{
			if (frame != nullptr)
				delete frame;
		}

		m_StoredFrames.clear();

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}

	return 0;
}

void Graphics::SetupParts()
{
	m_Parts = {};

	std::shared_ptr<Renderer> rend = std::make_shared<Renderer>(Renderer());

	if (rend != nullptr)
	{
		rend->Start();

		m_Parts.push_back(rend);
	}

	std::shared_ptr<VideoWriter> vidWriter = std::make_shared<VideoWriter>(VideoWriter());

	if (vidWriter != nullptr)
	{
		vidWriter->Start();

		m_Parts.push_back(vidWriter);
	}

}

int Graphics::SetupGLFW()
{
	glfwSetErrorCallback(GLFW_ERROR_LOG);

	if (!glfwInit())
	{
		std::cout << "Graphics Module - GLFW init failed!" << std::endl;

		return -1;
	}

	m_Window = glfwCreateWindow(m_WindowSize->x, m_WindowSize->y, "OpenGL GUI and Video Renderer", NULL, NULL);

	if (m_Window == nullptr)
	{
		glfwTerminate();

		std::cout << "Graphics Module - GLFW window creation failed!" << std::endl;

		return -2;
	}

	glfwMakeContextCurrent(m_Window);

	// vsync off = 0 , on = 1
	glfwSwapInterval(0);

	std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		std::cout << "Error: %s\n" << (unsigned char*)glewGetErrorString(err) << std::endl;

		return -3;
	}

	return 0;
}

int Graphics::SetupImGUI()
{
	IMGUI_CHECKVERSION();

	auto cont = ImGui::CreateContext();

	if (cont != nullptr)
	{
		ImGui::StyleColorsDark();

		if (!ImGui_ImplGlfw_InitForOpenGL(m_Window, true))
		{
			std::cout << "Graphics Module - ImGui init failed!" << std::endl;

			return -2;
		}

		if (!ImGui_ImplOpenGL3_Init("#version 130"))
		{
			std::cout << "Graphics Module - ImGui renderer backend init failed!" << std::endl;

			return -3;
		}
	}
	else
	{
		std::cout << "Graphics Module - ImGui failed to create a context!" << std::endl;

		return -1;
	}

	return 0;

}

void Graphics::ShowMenuBar(bool& beginRendering, bool& showRenderScreen, bool& showCurrentlyRenderingScreen, bool& showPrintedScreen, bool& showStats, bool& closeShown)
{
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Render To File..."))
			{
				if (!beginRendering)
				{
					showRenderScreen = true;
				}
				else
				{
					showCurrentlyRenderingScreen = true;
				}
			}

			if (ImGui::BeginMenu("Print Info"))
			{
				if (ImGui::MenuItem("Print OpenGL Info"))
				{
					PrintOpenGLInfo();
					showPrintedScreen = true;
				}

				if (ImGui::MenuItem("Print GLFW Info"))
				{
					PrintGLFWInfo();
					showPrintedScreen = true;
				}

				if (ImGui::MenuItem("Print GLEW Info"))
				{
					PrintGLEWInfo();
					showPrintedScreen = true;
				}

				if (ImGui::MenuItem("Print something fun :)"))
				{
					PrintSomethingFun();
					showPrintedScreen = true;
				}

				ImGui::EndMenu();
			}

			if (ImGui::MenuItem("Show Stats", NULL, showStats))
			{
				showStats = !showStats;
			}

			if (ImGui::MenuItem("Exit"))
			{
				closeShown = true;
			}

			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}
}

void Graphics::ShowStatsWindow()
{
	ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);

	if (ImGui::TreeNode("Stats"))
	{
		int currFrame = ImGui::GetFrameCount();

		// avoid divide by zero
		if (currFrame == 0)
			currFrame = 1;

		m_CumulativeFrameTime += ImGui::GetIO().DeltaTime * 1000;

		// Fill an array of contiguous float values to plot
		static float values[90] = {};
		static int values_offset = 0;
		static double refresh_time = 0.0;
		if (refresh_time == 0.0)
			refresh_time = ImGui::GetTime();

		// plot delta time of frame in ms
		while (refresh_time < ImGui::GetTime()) // Create data at fixed 60Hz rate
		{
			values[values_offset] = ImGui::GetIO().DeltaTime * 1000;
			values_offset = (values_offset + 1) % IM_ARRAYSIZE(values);
			refresh_time += 1.0f / 60.0f;
		}

		// set graph overlay text
		{
			char avg[64];

			sprintf_s(avg, "Average Frame Time: %.5fms (FPS: %f)", m_CumulativeFrameTime / ImGui::GetFrameCount(), ImGui::GetIO().Framerate);

			//std::string overlay_text = "Average Frame Time: " + std::to_chars(1000.0f / ImGui::GetIO().Framerate) + "ms (FPS: " + std::to_string(ImGui::GetIO().Framerate) + ")";

			ImGui::PlotLines("", values, IM_ARRAYSIZE(values), values_offset, avg, 0.0f, 5.0f, ImVec2(0, 100.0f));
		}
	}
}

int Graphics::ShowRenderToFileWindow()
{
	if (ImGui::BeginPopupModal("Render To File...", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		static char tmpBuf[16]{ 0 };

		ImGui::InputTextWithHint("File Name", "Please enter file name...", tmpBuf, IM_ARRAYSIZE(tmpBuf));
		ImGui::InputInt("Frame Rate (FPS)", &m_RenderFileFPS);
		ImGui::InputInt("Time (Seconds)", &m_RenderFileTime);
		ImGui::InputInt("Delay (Seconds)", &m_RenderFileDelay);

		if (m_RenderFileFPS < 1)
			m_RenderFileFPS = 1;
		else if (m_RenderFileFPS > 30)
			m_RenderFileFPS = 30;

		if (m_RenderFileTime < 1)
			m_RenderFileTime = 1;
		else if (m_RenderFileTime > 600)
			m_RenderFileTime = 600;

		if (m_RenderFileDelay < 0)
			m_RenderFileDelay = 0;
		else if (m_RenderFileDelay > 5)
			m_RenderFileDelay = 5;

		ImGui::Text("\n\n\n\n");

		ImGui::Separator();
		ImGui::BeginGroup();
		{
			if (ImGui::Button("Render", ImVec2(200.f, 0.f)))
			{
				if (tmpBuf[0] != 0)
				{
					std::copy_n(tmpBuf, 16, m_RenderFileName);

					sprintf_s(m_RenderFilePath, "%s\\Videos\\%s.wmv", std::filesystem::current_path().string().c_str(), m_RenderFileName);

					ImGui::CloseCurrentPopup();

					return 1;
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(200.f, 0.f)))
			{
				ImGui::CloseCurrentPopup();
				return -1;
			}
		}

		ImGui::EndPopup();

		return 0;
	}


}

int Graphics::ShowExitWindow()
{
	if (ImGui::BeginPopupModal("Exiting Application...", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Are you sure you want to exit?\n\n");
		ImGui::Separator();
		ImGui::BeginGroup();
		{
			if (ImGui::Button("Yes", ImVec2(120.f, 0.f)))
			{
				ImGui::CloseCurrentPopup();
				return 1;
			}
			ImGui::SameLine();
			if (ImGui::Button("No", ImVec2(120.f, 0.f)))
			{
				ImGui::CloseCurrentPopup();
				return -1;
			}
		}

		ImGui::EndPopup();

		return 0;
	}


}

int Graphics::ShowOverwriteWindow()
{
	if (ImGui::BeginPopupModal("Overwite existing file?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		char message[128] = "";
		sprintf_s(message, "'%s' already exists, rendering will overwrite this file!\n\n", m_RenderFileName);

		ImGui::TextWrapped(message);
		ImGui::Text("Are you sure you want to continue?\n\n");
		ImGui::Separator();
		ImGui::BeginGroup();
		{
			if (ImGui::Button("Yes", ImVec2(120.f, 0.f)))
			{
				ImGui::CloseCurrentPopup();
				return 1;
			}
			ImGui::SameLine();
			if (ImGui::Button("No", ImVec2(120.f, 0.f)))
			{
				ImGui::CloseCurrentPopup();
				return -1;
			}
		}

		ImGui::EndPopup();

		return 0;
	}
}

int Graphics::ShowPrintCompleteWindow()
{
	if (ImGui::BeginPopupModal("Print Complete!", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::TextWrapped("Printed! Check Logs folder for text file!\n\n");
		ImGui::Separator();
		ImGui::BeginGroup();
		{
			if (ImGui::Button("Okay", ImVec2(260.f, 0.f)))
			{
				ImGui::CloseCurrentPopup();
				return 1;
			}
		}

		ImGui::EndPopup();

		return 0;
	}
}

int Graphics::ShowCurrentlyRenderingWindow()
{
	if (ImGui::BeginPopupModal("Rendering still in progress!", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		char message[128] = "";
		sprintf_s(message, "Cannot start another render yet, render of '%s' still in progress!\n\n", m_RenderFileName);

		ImGui::TextWrapped(message);
		ImGui::Separator();
		ImGui::BeginGroup();
		{
			if (ImGui::Button("Close", ImVec2(260.f, 0.f)))
			{
				ImGui::CloseCurrentPopup();
				return 1;
			}
		}

		ImGui::EndPopup();

		return 0;
	}
}

void Graphics::ShowRecordingStatus(const bool& isRecording, const bool& isWriting, const bool& isDelayed, const float& renderDelay)
{
	if (isRecording)
	{
		ImGui::Text("Recording '%s.wmv'... %d", m_RenderFileName, (int)m_RecordTime);
	}
	else if (isWriting)
	{
		ImGui::Text("Saving '%s.wmv'...", m_RenderFileName);
	}
	else if (isDelayed)
	{
		ImGui::Text("Recording '%s' will begin in %d", m_RenderFileName, (int)m_RenderFileDelay - (int)renderDelay);
	}
}

void Graphics::ShowUsageTable()
{
	if (ImGui::TreeNode("Usage"))
	{
		if (ImGui::BeginTable("Usage Table", 2, ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Hideable))
		{
			ImGui::TableSetupColumn("Menu Option", ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn("Description", ImGuiTableColumnFlags_WidthStretch);

			ImGui::TableHeadersRow();

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("File -> Render To File...");
			ImGui::TableSetColumnIndex(1);
			ImGui::TextWrapped("Here you can 'record' the applications window and render the result to a .WMV file.");
			ImGui::TextWrapped("These video files will be saved in the 'Videos' directory located in the working directory.\n\n");
			ImGui::TextWrapped("Some details on how it works...");
			ImGui::TextWrapped("When we start the 'recording' process, the pixels from the OpenGL Context are being read into a pixel buffer objects, which are then mapped to a byte array.");
			ImGui::TextWrapped("We have two pixel buffer objects, one reads the pixels from the back buffer while the other is being mapped to a byte array.");
			ImGui::TextWrapped("This allows us to get the pixels across to the CPU without needing to wait for the frame to be presented to the screen.");
			ImGui::TextWrapped("Based on the duration specified in the 'Render To File...' popup, these byte arrays will be stored and be used as the frames in our video.\n\n");
			ImGui::TextWrapped("When the frames for the specified duration and frame rate have been collected, they will then be passsed to the VideoWriter.");
			ImGui::TextWrapped("The VideoWriter uses the Microsoft Media Foundation API to take our frame data and convert this into the video file.");
			ImGui::TextWrapped("The frames have to be processed one by one, so asynchronous functionality is used so that the program does not get halted during this time.");

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("File -> Print Info");
			ImGui::TableSetColumnIndex(1);
			ImGui::TextWrapped("Here, there are four options. Each one will print information to a text file.");
			ImGui::TextWrapped("The text files will be saved in the 'Logs' directory in the working directory.\n\n");
			ImGui::TextWrapped("OpenGL Info:");
			ImGui::BulletText("Vendor Name");
			ImGui::BulletText("Version");
			ImGui::BulletText("Renderer");
			ImGui::BulletText("GLSL Version\n\n");
			ImGui::TextWrapped("GLFW Info:");
			ImGui::BulletText("Version\n\n");
			ImGui::TextWrapped("GLEW Info:");
			ImGui::BulletText("Available Extensions\n\n");
			ImGui::TextWrapped("Something fun?:");
			ImGui::BulletText("Just a memento to one of my favourite gaming franchises!");

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("File -> ShowStats");
			ImGui::TableSetColumnIndex(1);
			ImGui::TextWrapped("This will show an animated time series graph.");
			ImGui::TextWrapped("The graph records the frame time in milliseconds of the last 90 frames.");
			ImGui::TextWrapped("The animation runs at a fixed refresh rate of 60hz.");

			ImGui::EndTable();
		}
	}
}

void Graphics::ShowKnownIssuesTable()
{
	if (ImGui::TreeNode("Issues"))
	{
		if (ImGui::BeginTable("Issues Table", 2, ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Hideable))
		{
			ImGui::TableSetupColumn("Issues", ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn("Description", ImGuiTableColumnFlags_WidthStretch);

			ImGui::TableHeadersRow();

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Recording memory usage");
			ImGui::TableSetColumnIndex(1);
			ImGui::TextWrapped("Currently the recording process stores the pixel data of each frame in memory for the desired time before giving it to the video writer.");
			ImGui::TextWrapped("Each pixel in each frame is in RGB32 format, meaning with each frame takes 1600 * 900 * 4 bytes of memory, 5.76MB.");
			ImGui::TextWrapped("Given a 30 second video when 60fps is desired, that's 10.368GB in memory...\n\n");
			ImGui::TextWrapped("In the interest of time, I have limited the videos to be 30 seconds at 30fps maximum to hard cap it going over this already ridiculously large figure.\n\n");
			ImGui::TextWrapped("Solutions:");
			ImGui::TextWrapped("Have each frame encoded as soon as it is read from the pixel buffer object.");
			ImGui::TextWrapped("This would immediately cut the memory usage to single frame figures (could open the door for live video streaming later).\n\n");
			ImGui::TextWrapped("This could introduce significant latency as we'd be sampling each frame of the video while we are simultaneously reading the next frame from the GPU");
			ImGui::TextWrapped("Care would need to be taken to make sure we are only processing complete frames.\n\n");
			ImGui::TextWrapped("Half the render resolution.");
			ImGui::TextWrapped("This would cut the memory footprint in half and the drawback of reduced video quality.");

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Windows reliance");
			ImGui::TableSetColumnIndex(1);
			ImGui::TextWrapped("I have used Windows Media Foundation as a means to encode my frames to video. This is not a cross platform solution.");
			ImGui::TextWrapped("If I were to take this further, I would investaigate into writing my own wrapper for avcodec as a means to provide a cross platform solution.");

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("OpenGL only");
			ImGui::TableSetColumnIndex(1);
			ImGui::TextWrapped("This demo only uses OpenGL 4.6 and has not looked into DirectX or Vulkan APIs.");
			ImGui::TextWrapped("Fortunately ImGui supports a heap of different platforms, so it would only be a matter of having the time to add that explicit support.");

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Styling");
			ImGui::TableSetColumnIndex(1);
			ImGui::TextWrapped("The GUI on display here is pretty barebones in terms of colours and styles.");
			ImGui::TextWrapped("If I were to take this further I would make a html/css wrapper for the ImGui style settings to allow us to set window styles.");

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("S P A G H E T T I");
			ImGui::TableSetColumnIndex(1);
			ImGui::TextWrapped("The code here is quite untidy in spots and could definitely do with a spring clean.\n\n");
			ImGui::TextWrapped("As I am using OpenGL 4.6, I could have made use of DSA (Direct State Access) which makes programming with OpenGL a bit closer to OOP.");

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Irrelevant 'engine' code");
			ImGui::TableSetColumnIndex(1);
			ImGui::TextWrapped("Tried to get too fancy early on setting up a whole engine, some engine structure left in after employing KISS.");

			ImGui::EndTable();
		}
	}
}

void Graphics::PrintOpenGLInfo()
{
	std::ofstream file;

	char filePath[128] = "";
	sprintf_s(filePath, "%s\\opengl_info.txt", m_PrintFilePathBase);

	std::cout << "Print Opengl " << filePath << std::endl;

	file.open(filePath, std::ios::out);

	if (file.is_open())
	{
		file << "OpenGL Information\n";
		file << "-----------------------------------------\n";
		file << "Vendor: " << glGetString(GL_VENDOR) << "\n";
		file << "OpenGL Version: " << glGetString(GL_VERSION) << "\n";
		file << "Renderer: " << glGetString(GL_RENDERER) << "\n";
		file << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";


		file.close();
	}
}

void Graphics::PrintGLFWInfo()
{
	std::ofstream file;

	char filePath[128] = "";
	sprintf_s(filePath, "%s\\glfw_info.txt", m_PrintFilePathBase);

	file.open(filePath, std::ios::out);

	if (file.is_open())
	{
		file << "GLFW Information\n";
		file << "-----------------------------------------\n";
		file << "Version: " << glfwGetVersionString() << "\n";


		file.close();
	}
}

void Graphics::PrintGLEWInfo()
{
	GLint n = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &n);

	std::ofstream file;

	char filePath[128] = "";
	sprintf_s(filePath, "%s\\glew_info.txt", m_PrintFilePathBase);

	file.open(filePath, std::ios::out);

	if (file.is_open())
	{
		file << "GLEW Information\n";
		file << "-----------------------------------------\n";
		file << "GLEW Version: " << glewGetString(GLEW_VERSION) << "\n";
		file << "Extensions Available:\n";

		for (size_t i = 0; i < n; ++i)
		{
			file << glGetStringi(GL_EXTENSIONS, i) << "\n";
		}

		file.close();
	}
}

void Graphics::PrintSomethingFun()
{
	std::ofstream file;

	char filePath[128] = "";
	sprintf_s(filePath, "%s\\surprise.txt", m_PrintFilePathBase);

	file.open(filePath, std::ios::out);

	if (file.is_open())
	{
		file << " _______________________________________ \n";
		file << "|\\ ___________________________________ /|\n";
		file << "| | _                               _ | |\n";
		file << "| |(+)        _           _        (+)| |\n";
		file << "| | ~      _--/           \\--_      ~ | |\n";
		file << "| |       /  /             \\  \\       | |\n";
		file << "| |      /  |               |  \\      | |\n";
		file << "| |     /   |               |   \\     | |\n";
		file << "| |     |   |    _______    |   |     | |\n";
		file << "| |     |   |    \\     /    |   |     | |\n";
		file << "| |     \\    \\_   |   |   _/    /     | |\n";
		file << "| |      \\     -__|   |__-     /      | |\n";
		file << "| |       \\_                 _/       | |\n";
		file << "| |         --__         __--         | |\n";
		file << "| |             --|   |--             | |\n";
		file << "| |               |   |               | |\n";
		file << "| |                | |                | |\n";
		file << "| |                 |                 | |\n";
		file << "| |                                   | |\n";
		file << "| |     H A P P Y  F R A G G I N G    | |\n";
		file << "| | _                               _ | |\n";
		file << "| |(+)                             (+)| |\n";
		file << "| | ~                               ~ | |\n";
		file << "|/ ----------------------------------- \\|\n";
		file << " _______________________________________ \n";

		file.close();
	}
}
