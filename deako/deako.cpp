#include "deako_pch.h"
#include "deako.h"

namespace Deako {

	static Context* Gdeako = nullptr;

	Context& CreateContext(ContextConfig& config)
	{
		Gdeako = new Context();
		Context& deako = *Gdeako;

		DK_CORE_ASSERT(!deako.Initialized, "Deako Context already initialized!");

		deako.Window = CreateScope<Window>(Gdeako);
		deako.Application = CreateScope<Application>(Gdeako, deako.Window.get(), config.AppName);
		deako.Input = CreateScope<Input>(Gdeako, deako.Window.get());

		deako.Initialized = true;

		return deako;
	}

	void DestroyContext()
	{
		delete Gdeako;
		Gdeako = nullptr;
	}

}
