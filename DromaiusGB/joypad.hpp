#pragma once

#include "types.hpp"
#include "addressable.hpp"
#include "interrupts.hpp"

namespace dromaiusgb
{

	enum class JoypadButton
	{
		Right,
		Left,
		Up,
		Down,
		A,
		B,
		Select,
		Start
	};

	union joypad_control_t
	{
		byte value;
		struct
		{
			byte right_or_a : 1;
			byte left_or_b : 1;
			byte up_or_select : 1;
			byte down_or_start : 1;
			byte direction_select : 1;
			byte button_select : 1;
			byte : 2;
		};

		joypad_control_t() : value(0) {}
		joypad_control_t(int i) : value(i) {}
		operator byte() const { return value; }
	};

	class Joypad : public Addressable
	{
	private:
		bool button_states[8];
		InterruptController &interrupt_controller;
		joypad_control_t joypad_flags;

	public:
		Joypad(Bus &, InterruptController &);

		void Set(bus_address_t, byte);
		byte Get(bus_address_t) const;

		void SetButtonState(JoypadButton, bool);
	};
}