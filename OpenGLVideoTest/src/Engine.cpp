#include <Modules/Graphics/Graphics.h>
#include <Engine.h>

#include <iostream>


int Engine::Start()
{
	std::cout << "Engine is cranking up!" << std::endl;

	m_GraphicsModule = std::make_shared<Graphics>(Graphics());

	if (m_GraphicsModule != nullptr)
	{
		m_GraphicsModule->Start();
	}
	else
	{
		return -1;
	}

	return 0;
}

int Engine::Tick()
{
	std::cout << "*CHUGGA CHUGGA* (Frame: " << m_FrameCount << ")" << std::endl;

	m_FrameCount++;

	if (m_GraphicsModule != nullptr)
	{
		m_GraphicsStatus = m_GraphicsModule->Tick();

		if (m_GraphicsStatus != 0)
		{
			return m_GraphicsStatus;
		}
	}
	else
	{
		std::cout << "Graphics Module failed! err: " << m_GraphicsStatus << " (Frame: " << m_FrameCount << ")" << std::endl;
		return -1;
	}

	return 0;
}

int Engine::End()
{
	std::cout << "Engine is stopping!" << std::endl;

	if (m_GraphicsModule != nullptr)
	{
		m_GraphicsModule->End();
	}

	return 0;
}
