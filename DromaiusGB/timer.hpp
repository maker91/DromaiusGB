#pragma once

#include "types.hpp"
#include "addressable.hpp"


namespace dromaiusgb
{

	union timer_control_t
	{
		byte value;
		struct
		{
			byte clock_select : 2;
			byte timer_enabled : 1;
			byte : 5;
		};

		timer_control_t() : value(0) {}
		timer_control_t(int i) : value(i) {}
		operator byte() const { return value; }
	};

	class Timer : public Addressable
	{
	private:
		const dword cycles_per_div_tick = 256;
		const dword cycles_per_tima_tick_0 = 1024;
		const dword cycles_per_tima_tick_1 = 16;
		const dword cycles_per_tima_tick_2 = 64;
		const dword cycles_per_tima_tick_3 = 256;

	private:
		dword div_cycle;
		dword tima_cycle;

	private:
		byte div;
		byte tima;
		byte tma;
		timer_control_t tac;

	public:
		Timer(Bus &);

		void Set(bus_address_t, byte);
		byte Get(bus_address_t) const;

		void Tick(dword delta_cycle);
	};
}