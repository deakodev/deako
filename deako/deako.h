#pragma once

#include "deako_config.h"
#include "app/deako_app.h"
#include "app/deako_core.h"
#include "app/deako_debug.h"

#include <renderer/Renderer.h>

namespace Deako {

	struct ContextConfig
	{
		const char* AppName;
		const char* WorkingDir;
		glm::vec2 WindowSize;
	};

	struct Context
	{
		Scope<Application> Application;
		Scope<Renderer> Renderer;

		bool Initialized = false;
	};

	Context& CreateContext(ContextConfig& config);
	void DestroyContext();

}