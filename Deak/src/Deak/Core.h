# pragma once

#ifdef DK_PLATFORM_MAC

#define DEAK_API __attribute__((visibility("default")))

#else
#error Deak only supports MacOS

#endif

#define BIT(x) (1 << x)
