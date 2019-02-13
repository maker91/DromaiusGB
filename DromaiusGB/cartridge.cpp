#include "cartridge.hpp"
#include "mbc0.hpp"
#include "mbc1.hpp"
#include <iostream>
#include <fstream>


namespace dromaiusgb
{

	Cartridge::Cartridge(Bus &bus) : Addressable(bus)
	{

	}

	void Cartridge::Set(bus_address_t addr, byte val)
	{
		mbc->Set(addr, val);
	}

	byte Cartridge::Get(bus_address_t addr) const
	{
		return mbc->Get(addr);
	}

	byte *Cartridge::GetBlock(bus_address_t addr)
	{
		return mbc->GetBlock(addr);
	}

	void Cartridge::LoadFromFile(std::string filename)
	{
		std::ifstream input(filename, std::ios::binary);

		if (!input)
			throw std::runtime_error("failed to load rom: " + filename);

		// read cartridge header
		input.seekg(0x100);
		input.read((char *)&header, 0x4F);

		std::cout << "===== Cartridge Header =====" << std::endl;
		std::cout << "Title: " << header.title << std::endl;
		std::cout << "Cartidge Type: 0x" << std::hex << (int)header.cartridge_type << std::endl;
		std::cout << "ROM Size: 0x" << std::hex << (int)header.rom_size << std::endl;
		std::cout << "RAM Size: 0x" << std::hex << (int)header.ram_size << std::endl;

		input.seekg(0);

		// create an MBC based on the header
		switch (header.cartridge_type) {

			case 0x00: case 0x08: // basic ROM (+ RAM)
				//mbc = std::make_unique<MBC0<0x8000, 0x2000>>(); break;
				mbc = std::make_unique<MBC1<0x4000, 128, 0x2000, 4>>(); break;

			case 0x01: case 0x02: case 0x03: // MBC1
				mbc = std::make_unique<MBC1<0x4000, 128, 0x2000, 4>>(); break;

			default:
				throw std::runtime_error("cartridge type not implemented");
		}

		// load the ROM in to the MBC
		mbc->LoadROM(input);
	}
}