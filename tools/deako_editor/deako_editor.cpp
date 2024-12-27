#include <deako.h>

#include "layers/SceneLayer.h"
#include "layers/GuiLayer.h"

using namespace Editor;

int main(int argc, char** argv)
{
	Deako::DebugProvider::Init();

	Deako::ContextConfig config{};
	config.AppName = "deako_editor";
	config.WorkingDir = "../deako_editor";
	config.WindowSize = { 1600, 900 };

	Deako::Context& deako = Deako::CreateContext(config);

	deako.Application->CreateLayer<SceneLayer>("scene_layer");
	deako.Application->CreateLayer<GuiLayer>("gui_layer");

	deako.Application->Run();

	Deako::DestroyContext();
}