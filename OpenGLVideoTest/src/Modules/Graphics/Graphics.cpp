#include "Modules/Graphics/Graphics.h"

#include <imgui\imgui.h>
#include <imgui\imgui_impl_glfw.h>
#include <imgui\imgui_impl_opengl3.h>
#include <imgui\imgui_internal.h>

#include <glew/glew.h>
#include <GLFW/glfw3.h>

#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>

#include <FI/FreeImage.h>

#include <Modules/Graphics/Renderer.h>

#include <iostream>
#include <string>
#include <algorithm>
//#include <wingdi.h>

static void GLFW_ERROR_LOG(int error, const char* description)
{
	std::cout << "GLFW Error - " << description << "(" << error << ")" << std::endl;
}

int Graphics::Start()
{
	m_ClearColour = std::make_shared<ImVec4>(ImVec4(0.45f, 0.55f, 0.60f, 1.00f));
	m_WindowSize = std::make_shared<ImVec2>(ImVec2(1600.0f, 900.0f));

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

	m_Init = true;

	return 0;
}



int Graphics::Tick()
{
	if (m_Init)
	{
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

		static bool beginRendering = false;

		ImVec2 centre = ImGui::GetMainViewport()->GetWorkCenter();

		ImGui::SetNextWindowPos(centre, ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(ImVec2(m_WindowSize->x / 2.f, m_WindowSize->y / 2.f), ImGuiCond_FirstUseEver);
		ImGui::Begin("ImGui In Action!", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar);

		ImGuiWindow* mainWindow = ImGui::GetCurrentWindow();

		ImVec2 mainWindowMid = ImVec2(mainWindow->Size.x / 2.f, mainWindow->Size.y / 2.f);

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Render To File..."))
				{
					showRenderScreen = true;
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


		if (closeShown)
		{
			closeShown = false;
			ImGui::OpenPopup("Exiting Application...");

			if (mainWindow != nullptr)
			{
				ImGui::SetNextWindowPos(mainWindowMid, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			}
		}

		if (showRenderScreen)
		{
			showRenderScreen = false;
			ImGui::OpenPopup("Render To File...");

			if (mainWindow != nullptr)
			{
				ImGui::SetNextWindowPos(mainWindowMid, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			}
		}

		if (ShowExitWindow() == 1)
		{
			closeRequested = true;
		}

		if (ShowRenderToFileWindow() == 1)
		{
			beginRendering = true;
		}

		if (showStats)
			ShowStatsWindow();

		ImGui::Render();

		int display_w, display_h;
		glfwGetFramebufferSize(m_Window, &display_w, &display_h);

		glViewport(0, 0, display_w, display_h);

		glClearColor(m_ClearColour->x * m_ClearColour->w, m_ClearColour->y * m_ClearColour->w, m_ClearColour->z * m_ClearColour->w, m_ClearColour->w);
		glClear(GL_COLOR_BUFFER_BIT);

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (beginRendering)
		{
			static float delay = 0.f;
			static float renderTime = 0.f;
			static int frames = 0;
			static int renderedFrames = 0;

			if (delay < m_RenderFileDelay)
			{
				delay += ImGui::GetIO().DeltaTime;
			}
			else
			{
				renderTime += ImGui::GetIO().DeltaTime;

				if (renderTime < m_RenderFileTime)
				{
					static double refresh_time = 0.0;
					if (refresh_time == 0.0)
						refresh_time = ImGui::GetTime();

					// render at desired frame rate
					while (refresh_time < ImGui::GetTime())
					{
						GLubyte* pixels = new GLubyte[3 * display_w * display_h];

						glReadPixels(0, 0, display_w, display_h, GL_BGR, GL_UNSIGNED_BYTE, pixels);

						GLenum err = glGetError();

						if (err != GL_NO_ERROR)
						{
							std::cout << "Couldn't read frame buffer pixels! (" << err << ")" << std::endl;
						}
						else
						{
							char fileName[64] = "";

							sprintf_s(fileName, "C:\\RenderImages\\%s_%d.bmp", m_RenderFileName, renderedFrames);

							FIBITMAP* image = FreeImage_ConvertFromRawBits(pixels, display_w, display_h, 3 * display_w, 24, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, FALSE);

							SaveBufferToBMP(pixels, renderedFrames);

							if (FreeImage_Save(FIF_BMP, image, fileName, 0))
								std::cout << "Successfully saved!" << std::endl;
							else
								std::cout << "Failed to save!" << std::endl;

							FreeImage_Unload(image);
						}

						delete pixels;

						++renderedFrames;

						refresh_time += 1.0f / m_RenderFileFPS;
					}

					++frames;
				}
				else
				{
					std::cout << "Rendering of '" << m_RenderFileName << "' complete! (Frames: " << std::to_string(renderedFrames) << ")" << std::endl;

					beginRendering = false;
					delay = 0;
					renderTime = 0;
					frames = 0;
					renderedFrames = 0;
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
}

int Graphics::SetupGLFW()
{
	glfwSetErrorCallback(GLFW_ERROR_LOG);

	if (!glfwInit())
	{
		std::cout << "Graphics Module - GLFW init failed!" << std::endl;

		return -1;
	}

	m_Window = glfwCreateWindow(m_WindowSize->x, m_WindowSize->y, "Opengl/ImGui Test", NULL, NULL);

	if (m_Window == nullptr)
	{
		glfwTerminate();

		std::cout << "Graphics Module - GLFW window creation failed!" << std::endl;

		return -2;
	}

	glfwMakeContextCurrent(m_Window);

	// vsync off = 0 , on = 1
	glfwSwapInterval(0);

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

int Graphics::SetupDevIL()
{
	ilInit();
	iluInit();
	ilutRenderer(ILUT_OPENGL);

	return 0;
}

void Graphics::ShowStatsWindow()
{
	ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);

	if (ImGui::TreeNode("Stats"))
	{
		// Fill an array of contiguous float values to plot
		// Tip: If your float aren't contiguous but part of a structure, you can pass a pointer to your first float
		// and the sizeof() of your structure in the "stride" parameter.
		static float values[90] = {};
		static int values_offset = 0;
		static double refresh_time = 0.0;
		if (refresh_time == 0.0)
			refresh_time = ImGui::GetTime();

		// plot points into 'values' array
		while (refresh_time < ImGui::GetTime()) // Create data at fixed 60 Hz rate
		{
			values[values_offset] = ImGui::GetIO().DeltaTime;
			values_offset = (values_offset + 1) % IM_ARRAYSIZE(values);
			refresh_time += 1.0f / 60.0f;
		}

		// set graph overlay text
		{
			char avg[64];

			sprintf_s(avg, "Average Frame Time: %.5f (FPS: %f)", ImGui::GetIO().DeltaTime, ImGui::GetIO().Framerate);

			//std::string overlay_text = "Average Frame Time: " + std::to_chars(1000.0f / ImGui::GetIO().Framerate) + "ms (FPS: " + std::to_string(ImGui::GetIO().Framerate) + ")";

			ImGui::PlotLines("", values, IM_ARRAYSIZE(values), values_offset, avg, 0.0f, 0.5f, ImVec2(0, 100.0f));
		}
	}
}

int Graphics::ShowRenderToFileWindow()
{
	if (ImGui::BeginPopupModal("Render To File...", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		static char tmpBuf[32] = "";

		ImGui::InputTextWithHint("File Name", "Please enter file name...", tmpBuf, IM_ARRAYSIZE(tmpBuf));
		ImGui::InputInt("Frame Rate (FPS)", &m_RenderFileFPS);
		ImGui::InputInt("Time (Seconds)", &m_RenderFileTime);
		ImGui::InputInt("Delay (Seconds)", &m_RenderFileDelay);

		if (m_RenderFileFPS < 1)
			m_RenderFileFPS = 1;
		else if (m_RenderFileFPS > 60)
			m_RenderFileFPS = 60;

		if (m_RenderFileTime < 1)
			m_RenderFileTime = 1;
		else if (m_RenderFileTime > 30)
			m_RenderFileTime = 30;

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
				std::copy_n(tmpBuf, 32, m_RenderFileName);

				std::cout << "new: " << m_RenderFileName << "\nold: " << tmpBuf << std::endl;

				ImGui::CloseCurrentPopup();
				return 1;
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

void Graphics::SaveBufferToBMP(const unsigned char* buf, const int& frameNumber) const
{
	char fileName[64] = "";

	sprintf_s(fileName, "C:\\RenderImages\\%s_%d.bmp", m_RenderFileName, frameNumber);

	std::cout << fileName << std::endl;


	/*FILE* file;
	unsigned long imageSize = 3* m_WindowSize->x* m_WindowSize->y;
	GLbyte* data = NULL;
	GLenum lastBuffer;
	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER bmih;

	bmfh.bfType = 'MB';
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfOffBits = 54;
	bmih.biSize = 40;

	bmih.biWidth = m_WindowSize->x;
	bmih.biHeight = m_WindowSize->y;
	bmih.biPlanes = 1;
	bmih.biBitCount = 24;
	bmih.biCompression = 0;
	bmih.biSizeImage = imageSize;
	bmih.biXPelsPerMeter = 45089;
	bmih.biYPelsPerMeter = 45089;
	bmih.biClrUsed = 0;
	bmih.biClrImportant = 0;

	bmfh.bfSize = imageSize + sizeof(bmfh) + sizeof(bmih);

	file = fopen(m_RenderFileName, "wb");

	fwrite(&bmfh, sizeof(bmfh), 1, file);
	fwrite(&bmih, sizeof(bmih), 1, file);
	fwrite(data, imageSize, 1, file);
	free(data);
	fclose(file);*/



}
