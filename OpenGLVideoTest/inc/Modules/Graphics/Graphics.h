#pragma once
#include <Modules/Module.h>

#include <memory>

struct GLFWwindow;
struct ImVec4;
struct ImVec2;
struct ImGuiContext;
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

public:
	int Start() override;
	int Tick() override;
	int End() override;

protected:

	void SetupParts() override;

private:
	int SetupGLFW();
	int SetupImGUI();

	void RenderStats();

};

