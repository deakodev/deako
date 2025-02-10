#ifndef DEAKO_HEADER
#define DEAKO_HEADER

#include "deako_config.h"
#include "logger/log.h"

#include <magic_memory.h>

#include <stdbool.h>
#include <stdint.h>

typedef struct {
	Magic_Arena arena;
	bool initialized;
} Deako;

typedef struct
{
	const char* app_name;
	const char* working_dir;
	//glm::vec2 window_size;
} Deako_Config;

extern Deako* deako_init(Deako_Config* config);
extern void deako_shutdown(void);


#endif // DEAKO_HEADER