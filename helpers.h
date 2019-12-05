#pragma once

#include <string.h>
#include "allocator.h"

#define HEADER_BYTE 'H'
#define ALLOCED_BYTE 'A'
#define EMPTY_BYTE 'E'

void memDump();

void printBeforeStart();

void printPageIds();
