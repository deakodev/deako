#include <deako.h>

#include "layers/SceneLayer.h"
#include "layers/GuiLayer.h"

using namespace Editor;

int main(int argc, char** argv)
{
	Deako::DebugProvider::Init();

	std::string zach = "zach";

	Deako::Arena arena{ 1024 * 1024 * 1024 };

	DK_INFO("Arena space: {}", arena.RemainingSpace());

	Deako::ArenaBlock* block = arena.AddBlock(1024);

	DK_INFO("Arena space: {}", arena.RemainingSpace());
	DK_INFO("Block space: {}", block->RemainingSpace());

	block->Push(zach);

	DK_INFO("Block space: {}", block->RemainingSpace());




	/*Deako::ContextConfig config{};
	config.AppName = "deako_editor";
	config.WorkingDir = "../deako_editor";
	config.WindowSize = { 1600, 900 };

	Deako::Context& deako = Deako::CreateContext(config);

	deako.Application->CreateLayer<SceneLayer>("scene_layer");
	deako.Application->CreateLayer<GuiLayer>("gui_layer");

	deako.Application->Run();

	Deako::DestroyContext();*/
}