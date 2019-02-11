#include "timer.hpp"
#include "bus.hpp"
#include "interrupts.hpp"

#include <iostream>

namespace dromaiusgb
{
	Timer::Timer(Bus &bus, InterruptController &ic) : Addressable(bus), interrupt_controller(ic), div_cycle(0), tima_cycle(0),
		div(0), tima(0), tma(0)
	{

	}

	void Timer::Set(bus_address_t addr, byte val)
	{
		switch (addr.address)
		{
		case 0xFF04: div = val; break;
		case 0xFF05: tima = val; break;
		case 0xFF06: tma = val; break;
		case 0xFF07: tac = val; break;
		}
	}

	byte Timer::Get(bus_address_t addr) const
	{
		switch (addr.address)
		{
		case 0xFF04: return div;
		case 0xFF05: return tima;
		case 0xFF06: return tma;
		case 0xFF07: return tac;
		default: return 0xFF;
		}
	}

	void Timer::Tick(dword delta_cycle)
	{
		div_cycle += delta_cycle;

		if (div_cycle >= cycles_per_div_tick) {
			div_cycle -= cycles_per_div_tick;
			div += 1;
		}

		if (tac.timer_enabled) {
			tima_cycle += delta_cycle;

			dword cycles_per_tima_tick = 0;
			switch (tac.clock_select) {
			case 0: cycles_per_tima_tick = cycles_per_tima_tick_0; break;
			case 1: cycles_per_tima_tick = cycles_per_tima_tick_1; break;
			case 2: cycles_per_tima_tick = cycles_per_tima_tick_2; break;
			case 3: cycles_per_tima_tick = cycles_per_tima_tick_3; break;
			};

			while (tima_cycle >= cycles_per_tima_tick) {
				tima_cycle -= cycles_per_tima_tick;
				tima += 1;

				if (tima == 0) {
					// tima overflow, reset and request interrupt
					tima = tma;
					interrupt_controller.RequestInterrupt(InterruptFlags::Timer);
				}
			}
		}
	}
}