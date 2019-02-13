#pragma once

#include "bus.hpp"
#include <istream>

namespace dromaiusgb
{

	class MBC
	{
	public:
		virtual void Set(bus_address_t addr, byte val) = 0;
		virtual byte Get(bus_address_t addr) const = 0;
		virtual byte *GetBlock(bus_address_t addr) = 0;
		virtual void LoadROM(std::istream &) = 0;
	};
}