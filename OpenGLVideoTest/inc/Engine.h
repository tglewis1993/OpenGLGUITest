#pragma once
#include <memory>

class Graphics;

class Engine
{
private:

	int m_FrameCount = 0;
	std::shared_ptr<Graphics> m_GraphicsModule = nullptr;

	int m_GraphicsStatus = 0;

public:

	int Start();
	int Tick();
	int End();

};

