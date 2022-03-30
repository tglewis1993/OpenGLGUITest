#include <iostream>
#include <memory>
#include <Engine.h>



int main()
{
	int engineTickCode = 0;

	std::unique_ptr<Engine> engineInstance = std::make_unique<Engine>(Engine());

	if (engineInstance != nullptr)
	{
		engineInstance->Start();

		while (engineTickCode == 0)
		{
			engineTickCode = engineInstance->Tick();
		}

		engineInstance->End();
	}

}
