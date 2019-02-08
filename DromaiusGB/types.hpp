#pragma once

#include <stdint.h>

namespace dromaiusgb
{
	typedef uint8_t byte;
	typedef uint16_t word;
	typedef uint32_t dword;

	typedef int8_t sbyte;

	typedef uint16_t address_t;

	struct bus_address_t
	{
		address_t address;
		address_t offset;
	};
}