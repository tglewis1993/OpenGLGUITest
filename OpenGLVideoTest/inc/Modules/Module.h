#pragma once

#include <vector>
#include <memory>

class ModulePart;

class Module
{
protected:
	std::vector<std::shared_ptr<ModulePart>> m_Parts;

public:

	int virtual Start() = 0;
	int virtual Tick() = 0;
	int virtual End() = 0;


protected:
	void virtual SetupParts() = 0;
};

