#include "deako_pch.h"
#include "deako.h"

namespace Deako {

	static Context* Gdeako = nullptr;

	Context& CreateContext(ContextConfig& config)
	{
		Gdeako = new Context();
		Context& deako = *Gdeako;

		DK_CORE_ASSERT(!deako.Initialized, "Deako Context already initialized!");

		deako.Application = CreateScope<Application>(Gdeako, config.AppName, config.WindowSize);
		deako.Renderer = CreateScope<Renderer>(Gdeako, deako.Application->GetWindow());

		deako.Initialized = true;

		return deako;
	}

	void DestroyContext()
	{
		delete Gdeako;
		Gdeako = nullptr;
	}

}
