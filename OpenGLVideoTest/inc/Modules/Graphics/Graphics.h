#pragma once
#include <Modules/Module.h>

#include <memory>

struct GLFWwindow;
struct ImVec4;
class Renderer;

class Graphics : public Module
{
private:
	GLFWwindow* m_Window = nullptr;

	std::shared_ptr<Renderer> m_Renderer = nullptr;
	std::shared_ptr<ImVec4> m_ClearColour;

	bool m_Init = false;

	float m_ColourR = 1.0f;
	float m_ColourG = 0.0f;
	float m_ColourB = 0.0f;
	float m_ColourA = 1.0f;

public:
	int Start() override;
	int Tick() override;
	int End() override;

protected:

	void SetupParts() override;

};

