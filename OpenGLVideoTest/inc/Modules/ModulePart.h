#pragma once
class ModulePart
{
public:
	int virtual Start() = 0;
	int virtual Tick() = 0;
	int virtual End() = 0;
};

