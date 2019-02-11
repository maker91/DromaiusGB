#pragma once

#include "types.hpp"
#include "addressable.hpp"
#include "interrupts.hpp"


namespace dromaiusgb
{

	union serial_transfer_control_t
	{
		byte value;
		struct
		{
			byte shift_clock : 1;
			byte clock_speed : 1;
			byte unused : 5;
			byte transfer_start_flag: 1;
		};

		serial_transfer_control_t() : value(0) {}
		serial_transfer_control_t(int i) : value(i) {}
		operator byte() const { return value; }
	};

	class LinkPort : public Addressable
	{
	private:
		InterruptController &interrupt_controller;

	private:
		serial_transfer_control_t transfer_control;
		byte transfer_value;

	public:
		LinkPort(Bus &, InterruptController &);

		void Set(bus_address_t, byte);
		byte Get(bus_address_t) const;
	};
}