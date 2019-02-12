#pragma once

#include "types.hpp"
#include "addressable.hpp"
#include "bus.hpp"

namespace dromaiusgb
{

	enum class InterruptFlags
	{
		VBlank = 1,
		LCDStat = 2,
		Timer = 4,
		Serial = 8,
		Joypad = 16,
	};

	union interrupt_flags_t
	{
		byte value;
		struct
		{
			byte vblank : 1;
			byte lcd_stat : 1;
			byte timer : 1;
			byte serial : 1;
			byte joypad : 1;
			byte : 3;
		};

		interrupt_flags_t() : value(0) {}
		interrupt_flags_t(int i) : value(i) {}
		interrupt_flags_t(InterruptFlags i) : value((int)i) {}
		operator byte() const { return value; }
	};

	class InterruptController : public Addressable
	{
	public:
		interrupt_flags_t interrupt_enable;
		interrupt_flags_t interrupt_flags;

	public:
		InterruptController(Bus &);

		void Set(bus_address_t, byte);
		byte Get(bus_address_t) const;

		void RequestInterrupt(InterruptFlags);
	};
}