#include "link.hpp"
#include <iostream>

namespace dromaiusgb
{

	LinkPort::LinkPort(Bus &bus, InterruptController &ic) : Addressable(bus), interrupt_controller(ic)
	{

	}

	void LinkPort::Set(bus_address_t addr, byte val)
	{
		switch (addr.address) {
			case 0xFF01:
			{
				transfer_value = val;
				break;
			}

			case 0xFF02:
			{
				transfer_control = val;

				if (transfer_control.transfer_start_flag) {
					std::cout << transfer_value << std::flush;
					transfer_control.transfer_start_flag = 0;
					interrupt_controller.RequestInterrupt(InterruptFlags::Serial);
				}

				break;
			}
		}
	}

	byte LinkPort::Get(bus_address_t addr) const
	{
		switch (addr.address) {
			case 0xFF01: return transfer_value;
			case 0xFF02: return transfer_control;
			default: return 0xFF;
		}
	}
}