#pragma once

#include <string>
#include "types.hpp"
#include "addressable.hpp"

namespace dromaiusgb
{

	template <word Size>
	class RAM : public Addressable
	{
	private:
		byte ram[Size];

	public:
		RAM(Bus &) : Addressable(bus) {}

		void Set(bus_address_t addr, byte val)
		{
			ram[addr.offset] = val;
		}

		byte Get(bus_address_t addr) const
		{
			return ram[addr.offset];
		}

		byte *GetBlock(bus_address_t addr)
		{
			return &ram[addr.offset];
		}

	};
}