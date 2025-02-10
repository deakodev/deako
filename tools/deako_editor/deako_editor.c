#include <deako.h>

int main(int argc, char** argv)
{
	Deako_Config config;
	Deako* deako = deako_init(&config);

	DK_LOG_DEBUG("Deako initialized? %s\n", deako->initialized ? "true" : "false");
	DK_LOG_DEBUG("Deako arena write marker: %d\n", deako->arena.write_marker);
	DK_LOG_DEBUG("Deako arena end marker: %d\n", deako->arena.end_marker);
} // End of main