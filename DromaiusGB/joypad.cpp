#include "joypad.hpp"


namespace dromaiusgb
{

	Joypad::Joypad(Bus &bus, InterruptController &ic) : Addressable(bus), interrupt_controller(ic)
	{
		for (bool &b : button_states) {
			b = false;
		}
	}

	void Joypad::Set(bus_address_t addr, byte val)
	{
		joypad_flags = val;
	}

	byte Joypad::Get(bus_address_t) const
	{
		joypad_control_t r = joypad_flags;

		if (!joypad_flags.direction_select) {
			r.right_or_a = (!button_states[(int)JoypadButton::Right]);
			r.left_or_b = (!button_states[(int)JoypadButton::Left]);
			r.up_or_select = (!button_states[(int)JoypadButton::Up]);
			r.down_or_start = (!button_states[(int)JoypadButton::Down]);
		}

		if (!joypad_flags.button_select) {
			r.right_or_a = (!button_states[(int)JoypadButton::A]);
			r.left_or_b = (!button_states[(int)JoypadButton::B]);
			r.up_or_select = (!button_states[(int)JoypadButton::Select]);
			r.down_or_start = (!button_states[(int)JoypadButton::Start]);
		}

		return r;
	}

	void Joypad::SetButtonState(JoypadButton button, bool pressed)
	{
		if (!button_states[(int)button] && pressed)
			interrupt_controller.RequestInterrupt(InterruptFlags::Joypad);

		button_states[(int)button] = pressed;
	}
}