#define DEAKO_IMPLEMENT_MAIN
#include "deako.h"

#include "deako_editor.h"

static dk_layer_t layers[] = {
	{ "GUI", dk_editor_gui_on_update, dk_editor_gui_on_request },
	{ "VIEWPORT", dk_editor_viewport_on_update, dk_editor_viewport_on_request }
};

dk_config_t dk_configure(void)
{
	return (dk_config_t) {
		.app_name = "Deako Editor", .app_layers = layers,
			.app_layer_count = 2, .window_width = 1200, .window_height = 900
	};
}