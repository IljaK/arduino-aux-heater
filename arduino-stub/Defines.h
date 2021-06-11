#pragma once
#include <stdint.h>
#ifdef _LINUX_
#include <unistd.h>
#else
#include <windows.h>
#endif
#include <ctype.h>
#include <time.h>
#include "Defines.h"
typedef uint8_t pin_size_t;