#pragma once

#include <string>
#include <fstream>
#include "types.hpp"
#include "addressable.hpp"

namespace dromaiusgb
{
	template <word Size>
	class ROM : public Addressable
	{
	private:
		bool rom_enabled;
		byte rom[Size];

	public:
		ROM(Bus &) : Addressable(bus), rom_enabled(true) {};

		bool Enabled() const 
		{ 
			return rom_enabled; 
		};

		void Set(bus_address_t, byte) { } // read only

		byte Get(bus_address_t addr) const
		{ 
			return rom[addr.offset]; 
		}
		
		byte *GetBlock(bus_address_t addr) 
		{ 
			return &rom[addr.offset];
		}

		void Load(std::string name)
		{
			std::ifstream input(name, std::ios::binary);

			if (!input)
				throw std::runtime_error("failed to load rom: " + name);

			input.read((char *)rom, Size);
		}

		void Disable()
		{
			rom_enabled = false;
		}
	};

	template <word Size>
	class ROMSwitch : public Addressable
	{
	private:
		std::shared_ptr<ROM<Size>> rom;

	public:
		ROMSwitch(Bus &bus, std::shared_ptr<ROM<Size>> rom) : Addressable(bus), rom(rom) {}

		void Set(bus_address_t, byte) { rom->Disable(); }
		byte Get(bus_address_t) const { return 0xFF; }
	};
}