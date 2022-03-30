#include "Modules/Graphics/Graphics.h"

#include <imgui\imgui.h>
#include <imgui\imgui_impl_glfw.h>
#include <imgui\imgui_impl_opengl3.h>
#include <imgui\imgui_internal.h>

#include <glew/glew.h>
#include <GLFW/glfw3.h>

#include <Modules/Graphics/Renderer.h>

#include <iostream>
#include <string>

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

		static float f = 0.0f;
		static int counter = 0;

		static bool closeShown = false;
		static bool closeRequested = false;

		static bool showStats = false;

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
			//ImGui::OpenPopup("Exiting Application...");


			if (mainWindow != nullptr)
			{
				ImGui::SetNextWindowPos(mainWindowMid, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			}
		}

		if (ImGui::BeginPopupModal("Exiting Application...", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Are you sure you want to exit?\n\n");
			ImGui::Separator();
			ImGui::BeginGroup();
			{

				if (ImGui::Button("Yes", ImVec2(120.f, 0.f)))
				{
					closeRequested = true;
				}
				ImGui::SameLine();
				if (ImGui::Button("No", ImVec2(120.f, 0.f)))
				{
					ImGui::CloseCurrentPopup();
				}
			}

			ImGui::EndPopup();
		}

		ImGui::TextWrapped("The contents of this box are a few examples of simple functionality provided by the ImGui library!");

		if (showStats)
			RenderStats();

		//ImGui::SetNextWindowSize(ImVec2(m_WindowSize->x / 4.f, m_WindowSize->y / 4.f), ImGuiCond_Appearing);


		ImGui::Render();

		int display_w, display_h;
		glfwGetFramebufferSize(m_Window, &display_w, &display_h);

		glViewport(0, 0, display_w, display_h);

		glClearColor(m_ClearColour->x * m_ClearColour->w, m_ClearColour->y * m_ClearColour->w, m_ClearColour->z * m_ClearColour->w, m_ClearColour->w);
		glClear(GL_COLOR_BUFFER_BIT);

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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

void Graphics::RenderStats()
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
			values[values_offset] = 1000.0f / ImGui::GetIO().Framerate;
			values_offset = (values_offset + 1) % IM_ARRAYSIZE(values);
			refresh_time += 1.0f / 60.0f;
		}

		// set graph overlay text
		{
			char avg[64];

			sprintf_s(avg, "Average Frame Time: %.3f (FPS: %.1f)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

			//std::string overlay_text = "Average Frame Time: " + std::to_chars(1000.0f / ImGui::GetIO().Framerate) + "ms (FPS: " + std::to_string(ImGui::GetIO().Framerate) + ")";

			ImGui::PlotLines("", values, IM_ARRAYSIZE(values), values_offset, avg, 0.0f, 0.5f, ImVec2(0, 80.0f));
		}
	}
}
