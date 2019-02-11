#include "interrupts.hpp"
#include "bus.hpp"


namespace dromaiusgb
{

	InterruptController::InterruptController(Bus &bus) : Addressable(bus)
	{

	}

	void InterruptController::Set(bus_address_t addr, byte val)
	{
		switch (addr.address) {
			case 0xFF0F: interrupt_flags = val; break;
			case 0xFFFF: interrupt_enable = val; break;
		}
	}

	byte InterruptController::Get(bus_address_t addr) const
	{
		switch (addr.address) {
			case 0xFF0F: return interrupt_flags;
			case 0xFFFF: return interrupt_enable;
			default: return 0xFF;
		}
	}

	void InterruptController::RequestInterrupt(InterruptFlags interrupt)
	{
		interrupt_flags = interrupt_flags | (byte)interrupt;
	}
}