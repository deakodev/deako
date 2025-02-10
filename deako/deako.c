#include "deako_pch.h"
#include "deako.h"

#include <malloc.h>
#include <stdbool.h>
#include <assert.h>

static Deako* g_deako = NULL;

Deako* deako_init(Deako_Config* config)
{
	Magic_Arena arena = { 0 };
	int status = magic_arena_allocate(&arena, sizeof(*g_deako));
	if (status < 0)
	{
		DK_LOG_ERROR("Failed to allocate arena.\n");
		return NULL;
	}

	status = magic_arena_write(&arena, (void**)&g_deako, sizeof(*g_deako));
	if (status < 0)
	{
		DK_LOG_ERROR("Failed to write to arena.\n");
		return NULL;
	}

	if (g_deako == NULL)
	{
		return NULL;
	}

	g_deako->arena = arena;
	g_deako->initialized = true;

	return g_deako;
}

void deako_shutdown(void)
{
	/*delete g_deako;
	g_deako = nullptr;*/
}
