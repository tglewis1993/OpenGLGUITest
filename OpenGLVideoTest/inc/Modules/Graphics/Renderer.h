#pragma once
#include <Modules/ModulePart.h>

class Renderer : public ModulePart
{
public:
	int Start() override;
	int Tick() override;
	int End() override;
};

