#pragma once

#include <string>
#include <fstream>
#include "types.hpp"
#include "addressable.hpp"

namespace dromaiusgb
{
	template <word ROMSize, byte ROMBanks, word RAMSize, byte RAMBanks>
	class MBC1 : public Addressable
	{
	private:
		byte rom[ROMBanks][ROMSize];
		byte ram[RAMBanks][RAMSize];
		byte rom_bank = 0;
		byte ram_bank = 0;
		byte ram_enabled = 0;
		byte mode_select = 0;

	public:
		MBC1(Bus &) : Addressable(bus) {};

		bool Enabled() const
		{
			return true;
		};

		void Set(bus_address_t addr, byte val) 
		{
			byte reg = (addr.address >> 13) & 0x07;

			switch (reg)
			{
				// ram enable/disable
				case 0:
				{
					ram_enabled = ((val & 0x0F) == 0x0A);
					break;
				}
			
				// rom bank number select (lower 5 bits)
				case 1:
				{
					rom_bank = (rom_bank & 0x60) | (val & 0x1F);
					break;
				}

				// ram bank select or upper 2 bits of rom bank select
				case 2:
				{
					if (mode_select) // RAM mode, select ram bank
						ram_bank = val & 0x03;
					else // ROM mode, select upper 2 bits of rom bank
						rom_bank = ((val & 0x03) << 5) | (rom_bank & 0x1F);
					break;
				}

				// rom/ram mode select
				case 3:
				{
					mode_select = val;

					if (mode_select) // RAM mode, clear the upper 2 bits of rom_bank
						rom_bank &= 0x1F; 
					else // ROM mode, only allow selection of RAM bank 0
						ram_bank = 0; 

					break;
				}

				// Write to RAM bank
				case 5:
				{
					if (ram_enabled) {
						ram[ram_bank][addr.address - 0xA000] = val;
					}

					break;
				}
			}			
		}

		byte Get(bus_address_t addr) const
		{
			if (addr.address >= 0x000 && addr.address <= 0x3FFF)
				return rom[0][addr.address - 0x000];

			if (addr.address >= 0x4000 && addr.address <= 0x7FFF)
			{
				byte corrected_rom_bank = rom_bank;
				if ((corrected_rom_bank & 0x1F) == 0x0) // if the lower 5 bits are all zero, add one
					corrected_rom_bank += 1;

				return rom[corrected_rom_bank][addr.address - 0x4000];
			}

			if (addr.address >= 0xA000 && addr.address <= 0xBFFF && ram_enabled)
				return ram[ram_bank][addr.address - 0xA000];

			return 0xFF;
		}

		byte *GetBlock(bus_address_t addr)
		{
			return nullptr; // Getting a block of memory from memory banked rom/ram not implemented
		}

		void Load(std::string name)
		{
			std::ifstream input(name, std::ios::binary);

			if (!input)
				throw std::runtime_error("failed to load rom: " + name);

			input.read((char *)rom, ROMSize * ROMBanks);
		}
	};
}