#pragma once

#include <iostream>
#include <istream>
#include "types.hpp"
#include "imbc.hpp"

namespace dromaiusgb
{
	template <word ROMSize, word RAMSize>
	class MBC0 : public MBC
	{
	private:
		byte rom[ROMSize];
		byte ram[RAMSize];

	public:

		void Set(bus_address_t addr, byte val)
		{
			if (addr.address >= 0xA000 && addr.address <= 0xBFFF)
				ram[addr.offset] = val;
		}

		byte Get(bus_address_t addr) const
		{
			if (addr.address >= 0x0000 && addr.address <= 0x7FFF)
				return rom[addr.offset];

			if (addr.address >= 0xA000 && addr.address <= 0xBFFF)
				return ram[addr.offset];

			return 0xFF;
		}

		byte *GetBlock(bus_address_t addr)
		{
			if (addr.address >= 0x0000 && addr.address <= 0x7FFF)
				return &rom[addr.offset];

			if (addr.address >= 0xA000 && addr.address <= 0xBFFF)
				return &ram[addr.offset];

			return nullptr;
		}

		void LoadROM(std::istream &input)
		{
			input.read((char *)rom, ROMSize);
		}
	};
}