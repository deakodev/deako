#pragma once

#include "deako_config.h"
#include "app/deako_app.h"
#include "app/deako_core.h"
#include "app/deako_debug.h"

namespace Deako {

	struct ContextConfig
	{
		const char* AppName;
		const char* WorkingDir;
		glm::vec2 WindowSize;
	};

	struct Context
	{
		Scope<Window> Window;
		Scope<Application> Application;
		Scope<Input> Input;

		bool Initialized = false;
	};

	Context& CreateContext(ContextConfig& config);
	void DestroyContext();

}