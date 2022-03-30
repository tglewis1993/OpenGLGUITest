#include "Modules/Graphics/Graphics.h"

#include <imgui\imgui.h>
#include <imgui\imgui_impl_glfw.h>
#include <imgui\imgui_impl_opengl3.h>

#include <glew/glew.h>
#include <GLFW/glfw3.h>

#include <Modules/Graphics/Renderer.h>

#include <iostream>

static void GLFW_ERROR_LOG(int error, const char* description)
{
	std::cout << "GLFW Error - " << description << "(" << error << ")" << std::endl;
}

int Graphics::Start()
{
	glfwSetErrorCallback(GLFW_ERROR_LOG);

	if (!glfwInit())
	{
		std::cout << "Graphics Module - GLFW init failed!" << std::endl;

		return -1;
	}

	m_Window = glfwCreateWindow(640, 480, "Video Test", NULL, NULL);

	if (m_Window == nullptr)
	{
		glfwTerminate();

		std::cout << "Graphics Module - GLFW window creation failed!" << std::endl;

		return -2;
	}

	glfwMakeContextCurrent(m_Window);

	// vsync off = 0 , on = 1
	glfwSwapInterval(0);

	SetupParts();

	IMGUI_CHECKVERSION();

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
	ImGui_ImplOpenGL3_Init("#version 130");

	m_ClearColour = std::make_shared<ImVec4>(ImVec4(0.45f, 0.55f, 0.60f, 1.00f));

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

		if (m_Parts.size() > 0)
		{
			for (std::shared_ptr<ModulePart> part : m_Parts)
			{
				part->Tick();
			}
		}

		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		//ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
		//ImGui::Checkbox("Another Window", &show_another_window);

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		//ImGui::ColorEdit3("clear color", m_ClearColour); // Edit 3 floats representing a color
		//ImGui::ColorEdit3()


		if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();


		ImGui::Render();

		int display_w, display_h;
		glfwGetFramebufferSize(m_Window, &display_w, &display_h);

		glViewport(0, 0, display_w, display_h);

		glClearColor(m_ClearColour->x * m_ClearColour->w, m_ClearColour->y * m_ClearColour->w, m_ClearColour->z * m_ClearColour->w, m_ClearColour->w);
		glClear(GL_COLOR_BUFFER_BIT);

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(m_Window);
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
