#include <iostream>
#include <memory>
#include <SFML/Graphics.hpp>

#include "bus.hpp"
#include "rom.hpp"
#include "ram.hpp"
#include "mbc1.hpp"

#include "cpu.hpp"
#include "lcd.hpp"
#include "timer.hpp"
#include "interrupts.hpp"


int main(int argc, const char* argv[]) 
{
	if (argc < 2) {
		return -1;
	}

	// Set up the address bus
	dromaiusgb::Bus address_bus;

	auto boot_rom = std::make_shared<dromaiusgb::ROM<0x100>>(address_bus);
	auto boot_rom_switch = std::make_shared<dromaiusgb::ROMSwitch<0x100>>(address_bus, boot_rom);
	//auto cartridge = std::make_shared<dromaiusgb::ROM<0x8000>>(address_bus);
	auto cartridge = std::make_shared<dromaiusgb::MBC1<0x4000, 128, 0x2000, 4>>(address_bus);
	auto vram = std::make_shared<dromaiusgb::RAM<0x2000>>(address_bus);
	auto wram = std::make_shared<dromaiusgb::RAM<0x2000>>(address_bus);
	auto oam = std::make_shared<dromaiusgb::RAM<0x009F>>(address_bus);
	auto timer = std::make_shared<dromaiusgb::Timer>(address_bus);
	auto interrupt_controller = std::make_shared<dromaiusgb::InterruptController>(address_bus);
	auto lcd = std::make_shared<dromaiusgb::LCD>(address_bus);
	auto hram = std::make_shared<dromaiusgb::RAM<0x007E>>(address_bus);

	address_bus.RegisterAddressSpace(0x0000, 0x0100, boot_rom);
	address_bus.RegisterAddressSpace(0xFF50, 0xFF50, boot_rom_switch);
	address_bus.RegisterAddressSpace(0x0000, 0x7FFF, cartridge); // cartridge rom
	address_bus.RegisterAddressSpace(0x8000, 0x9FFF, vram);
	address_bus.RegisterAddressSpace(0xA000, 0xBFFF, cartridge); // cartridge ram
	address_bus.RegisterAddressSpace(0xC000, 0xDFFF, wram);
	address_bus.RegisterAddressSpace(0xE000, 0xFDFF, wram);
	address_bus.RegisterAddressSpace(0xFE00, 0xFE9F, oam);
	address_bus.RegisterAddressSpace(0xFF04, 0xFF07, timer);
	address_bus.RegisterAddressSpace(0xFF0F, 0xFF0F, interrupt_controller);
	address_bus.RegisterAddressSpace(0xFF40, 0xFF4B, lcd);
	address_bus.RegisterAddressSpace(0xFF80, 0xFFFE, hram);
	address_bus.RegisterAddressSpace(0xFFFF, 0xFFFF, interrupt_controller);

	// Load the boot rom and cartridge
	boot_rom->LoadRom("bootstrap.bin");
	cartridge->LoadRom(argv[1]);

	// start the cpu thread
	dromaiusgb::CPU cpu(address_bus, *lcd, *timer, *interrupt_controller);
	cpu.Start();

	// create a sprite to draw the gameboy screen buffer
	sf::Sprite spr;
	spr.setOrigin(80, 72);
	spr.setPosition(240, 216);
	spr.setScale(2.5f, 2.5f);

	// create a window and poll for events
	sf::RenderWindow window(sf::VideoMode(480, 432), "DromaiusGC", sf::Style::Close);

	while (window.isOpen()) {

		sf::Event ev;
		while (window.pollEvent(ev)) {
			switch (ev.type) {
				case sf::Event::Closed:
				{
					window.close();
					break;
				}
			}
		}

		window.clear(sf::Color::Black);
		spr.setTexture(lcd->GetScreenTexture());
		window.draw(spr);
		window.display();

		std::this_thread::sleep_for(std::chrono::milliseconds(16));
	}

	// stop the cpu thread
	cpu.Stop();
	return 0;
}