#include "cpu.hpp"
#include "util.hpp"

#include <iostream>

namespace dromaiusgb
{

	CPU::CPU(Bus &bus, LCD &lcd, Timer &timer, InterruptController &interrupt_controller) 
		: bus(bus), lcd(lcd), timer(timer), interrupt_controller(interrupt_controller), running(false), halted(false)
	{
		registers[0] = &BC.hi; // B
		registers[1] = &BC.lo; // C
		registers[2] = &DE.hi; // D
		registers[3] = &DE.lo; // E
		registers[4] = &HL.hi; // H
		registers[5] = &HL.lo; // L
		registers[6] = &AF.lo; // F
		registers[7] = &AF.hi; // A

		wregisters[0] = &BC.value;
		wregisters[1] = &DE.value;
		wregisters[2] = &HL.value;
		wregisters[3] = &SP;

		AF = BC = DE = HL = PC = SP = 0;
		interrupt_master_enable_flag = true;
	};

	dword CPU::HandleCBPrefixOpcode()
	{
		opcode_t opcode = bus.Get(PC);
		PC += 1;

		switch (opcode) {
			case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x07: // rlc reg
			{
				byte &dst = *registers[opcode & 0x07];
				dst = util::rotate_left_through_carry(dst, AF.flags);
				return 8;
			}

			case 0x06: // rlc (HL)
			{
				byte v = util::rotate_left_through_carry(bus.Get(HL), AF.flags);
				bus.Set(HL, v);
				return 16;
			}

			case 0x08: case 0x09: case 0x0A: case 0x0B: case 0x0C: case 0x0D: case 0x0F: // rrc reg
			{
				byte &dst = *registers[opcode & 0x07];
				dst = util::rotate_right_through_carry(dst, AF.flags);
				return 8;
			}

			case 0x0E: // rrc (HL)
			{
				byte v = util::rotate_right_through_carry(bus.Get(HL), AF.flags);
				bus.Set(HL, v);
				return 16;
			}

			case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x17: // rl reg
			{
				byte &dst = *registers[opcode & 0x07];
				dst = util::rotate_left(dst, AF.flags);
				return 8;
			}

			case 0x16: // rl (HL)
			{
				byte v = util::rotate_left(bus.Get(HL), AF.flags);
				bus.Set(HL, v);
				return 16;
			}

			case 0x18: case 0x19: case 0x1A: case 0x1B: case 0x1C: case 0x1D: case 0x1F: // rr reg
			{
				byte &dst = *registers[opcode & 0x07];
				dst = util::rotate_right(dst, AF.flags);
				return 8;
			}

			case 0x1E: // rr (HL)
			{
				byte v = util::rotate_right(bus.Get(HL), AF.flags);
				bus.Set(HL, v);
				return 16;
			}

			case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x27: // sla reg
			{
				byte &dst = *registers[opcode & 0x07];
				dst = util::shift_left_arithmetic(dst, AF.flags);
				return 8;
			}

			case 0x26: // sla (HL)
			{
				byte v = util::shift_left_arithmetic(bus.Get(HL), AF.flags);
				bus.Set(HL, v);
				return 16;
			}

			case 0x28: case 0x29: case 0x2A: case 0x2B: case 0x2C: case 0x2D: case 0x2F: // sra reg
			{
				byte &dst = *registers[opcode & 0x07];
				dst = util::shift_right_arithmetic(dst, AF.flags);
				return 8;
			}

			case 0x2E: // sra (HL)
			{
				byte v = util::shift_right_arithmetic(bus.Get(HL), AF.flags);
				bus.Set(HL, v);
				return 16;
			}

			case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x37: // swap reg
			{
				byte &dst = *registers[opcode & 0x07];
				dst = util::swap_nibbles(dst, AF.flags);
				return 8;
			}

			case 0x36: // swap (HL)
			{
				byte v = util::swap_nibbles(bus.Get(HL), AF.flags);
				bus.Set(HL, v);
				return 16;
				
			}

			case 0x38: case 0x39: case 0x3A: case 0x3B: case 0x3C: case 0x3D: case 0x3F: // srl reg
			{
				byte &dst = *registers[opcode & 0x07];
				dst = util::shift_right_logical(dst, AF.flags);
				return 8;
			}

			case 0x3E: // srl (HL)
			{
				byte v = util::shift_right_logical(bus.Get(HL), AF.flags);
				bus.Set(HL, v);
				return 16;
			}

			case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x47: // bit 0, reg
			case 0x48: case 0x49: case 0x4A: case 0x4B: case 0x4C: case 0x4D: case 0x4F: // bit 1, reg
			case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x57: // bit 2, reg
			case 0x58: case 0x59: case 0x5A: case 0x5B: case 0x5C: case 0x5D: case 0x5F: // bit 3, reg
			case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x67: // bit 4, reg
			case 0x68: case 0x69: case 0x6A: case 0x6B: case 0x6C: case 0x6D: case 0x6F: // bit 5, reg
			case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x77: // bit 6, reg
			case 0x78: case 0x79: case 0x7A: case 0x7B: case 0x7C: case 0x7D: case 0x7F: // bit 7, reg
			{
				byte v = *registers[opcode & 0x07];
				byte bit = opcode >> 3 & 0x07;
				util::test_bit(v, bit, AF.flags);
				return 8;
			}

			case 0x46: case 0x4E: case 0x56: case 0x5E: case 0x66: case 0x6E: case 0x76: case 0x7E: // bit b, (HL)
			{
				byte v = bus.Get(HL);
				byte bit = opcode >> 3 & 0x07;
				util::test_bit(v, bit, AF.flags);
				return 12;
			}

			case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x87: // res 0, reg
			case 0x88: case 0x89: case 0x8A: case 0x8B: case 0x8C: case 0x8D: case 0x8F: // res 1, reg
			case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x97: // res 2, reg
			case 0x98: case 0x99: case 0x9A: case 0x9B: case 0x9C: case 0x9D: case 0x9F: // res 3, reg
			case 0xA0: case 0xA1: case 0xA2: case 0xA3: case 0xA4: case 0xA5: case 0xA7: // res 4, reg
			case 0xA8: case 0xA9: case 0xAA: case 0xAB: case 0xAC: case 0xAD: case 0xAF: // res 5, reg
			case 0xB0: case 0xB1: case 0xB2: case 0xB3: case 0xB4: case 0xB5: case 0xB7: // res 6, reg
			case 0xB8: case 0xB9: case 0xBA: case 0xBB: case 0xBC: case 0xBD: case 0xBF: // res 7, reg
			{
				byte &v = *registers[opcode & 0x07];
				byte bit = opcode >> 3 & 0x07;
				v = util::reset_bit(v, bit);
				return 8;
			}

			case 0x86: case 0x8E: case 0x96: case 0x9E: case 0xA6: case 0xAE: case 0xB6: case 0xBE: // res b, (HL)
			{
				byte v = bus.Get(HL);
				byte bit = opcode >> 3 & 0x07;
				bus.Set(HL, util::reset_bit(v, bit));
				return 16;
			}

			case 0xC0: case 0xC1: case 0xC2: case 0xC3: case 0xC4: case 0xC5: case 0xC7: // set 0, reg
			case 0xC8: case 0xC9: case 0xCA: case 0xCB: case 0xCC: case 0xCD: case 0xCF: // set 1, reg
			case 0xD0: case 0xD1: case 0xD2: case 0xD3: case 0xD4: case 0xD5: case 0xD7: // set 2, reg
			case 0xD8: case 0xD9: case 0xDA: case 0xDB: case 0xDC: case 0xDD: case 0xDF: // set 3, reg
			case 0xE0: case 0xE1: case 0xE2: case 0xE3: case 0xE4: case 0xE5: case 0xE7: // set 4, reg
			case 0xE8: case 0xE9: case 0xEA: case 0xEB: case 0xEC: case 0xED: case 0xEF: // set 5, reg
			case 0xF0: case 0xF1: case 0xF2: case 0xF3: case 0xF4: case 0xF5: case 0xF7: // set 6, reg
			case 0xF8: case 0xF9: case 0xFA: case 0xFB: case 0xFC: case 0xFD: case 0xFF: // set 7, reg
			{
				byte &v = *registers[opcode & 0x07];
				byte bit = opcode >> 3 & 0x07;
				v = util::set_bit(v, bit);
				return 8;
			}

			case 0xC6: case 0xCE: case 0xD6: case 0xDE: case 0xE6: case 0xEE: case 0xF6: case 0xFE: // set b, (HL)
			{
				byte v = bus.Get(HL);
				byte bit = opcode >> 3 & 0x07;
				bus.Set(HL, util::set_bit(v, bit));
				return 16;
			}

			default:
			{
				std::cout << "Undefined opcode: 0xcb " << std::hex << (int)opcode << " at " << PC << std::endl;
				return 0;
			}
		}
	}

	dword CPU::Step()
	{
		opcode_t opcode = bus.Get(PC);
		PC += 1;

		switch (opcode) {
			// ================================================
			//  ================= 8-bit LOADS =================
			// ================================================

			case 0x02: // ld (BC), A
			{
				bus.Set(BC, AF.hi);
				return 8;
			}

			case 0x12: // ld (DE), A
			{
				bus.Set(DE, AF.hi);
				return 8;
			}

			case 0x22: // ldi (HL), A
			{
				bus.Set(HL, AF.hi);
				HL = HL + 1;
				return 8;
			}

			case 0x32: // ldd (HL), A
			{
				bus.Set(HL, AF.hi);
				HL = HL - 1;
				return 8;
			}

			case 0x0A: // ld A, (BC)
			{
				AF.hi = bus.Get(BC);
				return 8;
			}

			case 0x1A: // ld A, (DE)
			{
				AF.hi = bus.Get(DE);
				return 8;
			}

			case 0x2A: // ldi A, (HL)
			{
				AF.hi = bus.Get(HL);
				HL = HL + 1;
				return 8;
			}

			case 0x3A: // ldd A, (HL)
			{
				AF.hi = bus.Get(HL);
				HL = HL - 1;
				return 8;
			}

			case 0x06: case 0x0E: case 0x16: case 0x1E: case 0x26: case 0x2E: case 0x3E: // ld reg, n
			{
				byte &dst = *registers[opcode >> 3 & 0x07];
				dst = util::get_immediate_byte(PC, bus);
				return 8;
			}

			case 0x36: //ld (HL), n
			{
				bus.Set(HL, util::get_immediate_byte(PC, bus));
				return 12;
			}

			case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x47: // ld b,reg
			case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4f: // ld c,reg
			case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x57: // ld d,reg
			case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5f: // ld e,reg
			case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x67: // ld h,reg
			case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6f: // ld l,reg
			case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7f: // ld a,reg
			{
				byte &dst = *registers[opcode >> 3 & 0x07];
				byte src = *registers[opcode & 0x07];
				dst = src;
				return 4;
			}

			case 0x46: case 0x4E: case 0x56: case 0x5E: case 0x66: case 0x6E: case 0x7E: // ld reg,(HL)
			{
				byte &dst = *registers[opcode >> 3 & 0x07];
				byte src = bus.Get(HL);
				dst = src;
				return 8;
			}

			case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x77: // ld (HL),reg
			{
				byte src = *registers[opcode & 0x07];
				bus.Set(HL, src);
				return 8;
			}

			case 0xE0: // ld ($FF00 + n), A
			{
				address_t addr = 0xFF00 + util::get_immediate_byte(PC, bus);
				bus.Set(addr, AF.hi);
				return 12;
			}

			case 0xF0: // ld A, ($FF00 + n)
			{
				address_t addr = 0xFF00 + util::get_immediate_byte(PC, bus);
				AF.hi = bus.Get(addr);
				return 12;
			}

			case 0xE2: // ld ($FF00 + C), A
			{
				bus.Set(0xFF00 + BC.lo, AF.hi);
				return 8;
			}

			case 0xF2: // ld A, ($FF00 + C)
			{
				AF.hi = bus.Get(0xFF00 + BC.lo);
				return 8;
			}

			case 0xEA: // ld (nn), A
			{
				word immediate = util::get_immediate_word(PC, bus);
				bus.Set(immediate, AF.hi);
				return 16;
			}

			case 0xFA: // ld A, (nn)
			{
				word immediate = util::get_immediate_word(PC, bus);
				AF.hi = bus.Get(immediate);
				return 16;
			}

			// ================================================
			// ================= 16-bit LOADS =================
			// ================================================

			case 0x01: case 0x11: case 0x21: case 0x31: // ld wreg, nn
			{
				word &dst = *wregisters[opcode >> 4 & 0x03];
				dst = util::get_immediate_word(PC, bus);
				return 12;
			}

			case 0x08: // ld (nn), SP
			{
				word immediate = util::get_immediate_word(PC, bus);
				bus.Set(immediate, SP & 0xFF);
				bus.Set(immediate + 1, (SP >> 8) & 0xFF);
				return 20;
			}

			case 0xF8: // ld HL, SP+r8
			{
				sbyte immediate = util::get_immediate_sbyte(PC, bus);
				HL = util::add_word_and_sbyte(SP, immediate, AF.flags);
				return 12;
			}

			case 0xF9: // ld SP, HL
			{
				SP = HL;
				return 8;
			}

			case 0xC1: case 0xD1: case 0xE1: // POP wreg (excluding AF)
			{
				word &dst = *wregisters[opcode >> 4 & 0x03];
				dst = util::pop(SP, bus);
				return 12;
			}

			case 0xF1: // POP AF
			{
				word v = util::pop(SP, bus);
				AF = util::mask_AF(v);
				return 12;
			}

			case 0xC5: case 0xD5: case 0xE5: // PUSH wreg (excluding AF)
			{
				word val = *wregisters[opcode >> 4 & 0x03];
				util::push(val, SP, bus);
				return 16;
			}

			case 0xF5: // PUSH AF
			{
				util::push(AF, SP, bus);
				return 16;
			}

			// ================================================
			// =========== 8-bit arithmetic/logical ===========
			// ================================================

			case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x87: // add a,reg
			case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8f: // adc a,reg
			{
				byte carry = (opcode >> 3 & 0x01) & AF.flags.cy;
				AF.hi = util::add_with_carry(AF.hi, *registers[opcode & 0x07], carry, AF.flags);
				return 4;
			}

			case 0x86: // add a,(HL)
			case 0x8E: // adc a,(HL)
			{
				byte carry = (opcode >> 3 & 0x01) & AF.flags.cy;
				AF.hi = util::add_with_carry(AF.hi, bus.Get(HL), carry, AF.flags);
				return 8;
			}

			case 0xC6: // add a,n
			case 0xCE: // adc a,n
			{
				byte carry = (opcode >> 3 & 0x01) & AF.flags.cy;
				AF.hi = util::add_with_carry(AF.hi, util::get_immediate_byte(PC, bus), carry, AF.flags);
				return 8;
			}

			case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x97: // sub a,reg
			case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9f: // sbc a,reg
			{
				byte carry = (opcode >> 3 & 0x01) & AF.flags.cy;
				AF.hi = util::sub_with_carry(AF.hi, *registers[opcode & 0x07], carry, AF.flags);
				return 4;
			}

			case 0x96: // sub a,(HL)
			case 0x9E: // sbc a,(HL)
			{
				byte carry = (opcode >> 3 & 0x01) & AF.flags.cy;
				AF.hi = util::sub_with_carry(AF.hi, bus.Get(HL), carry, AF.flags);
				return 8;
			}

			case 0xD6: // sub a,n
			case 0xDE: // sbc a,n
			{
				byte carry = (opcode >> 3 & 0x01) & AF.flags.cy;
				AF.hi = util::sub_with_carry(AF.hi, util::get_immediate_byte(PC, bus), carry, AF.flags);
				return 8;
			}

			case 0xA0: case 0xA1: case 0xA2: case 0xA3: case 0xA4: case 0xA5: case 0xA7: // and a,reg
			{
				byte r = *registers[opcode & 0x07];
				AF.hi = util::logical_and(AF.hi, r, AF.flags);
				return 4;
			}

			case 0xA6: // and a,(HL)
			{
				AF.hi = util::logical_and(AF.hi, bus.Get(HL), AF.flags);
				return 8;
			}

			case 0xE6: // and a,n
			{
				AF.hi = util::logical_and(AF.hi, util::get_immediate_byte(PC, bus), AF.flags);
				return 8;
			}

			case 0xA8: case 0xA9: case 0xAA: case 0xAB: case 0xAC: case 0xAD: case 0xAF: // xor a,reg
			{
				byte r = *registers[opcode & 0x07];
				AF.hi = util::logical_xor(AF.hi, r, AF.flags);
				return 4;
			}

			case 0xAE: // xor a,(HL)
			{
				AF.hi = util::logical_xor(AF.hi, bus.Get(HL), AF.flags);
				return 8;
			}

			case 0xEE: // xor a,n
			{
				AF.hi = util::logical_xor(AF.hi, util::get_immediate_byte(PC, bus), AF.flags);
				return 8;
			}

			case 0xB0: case 0xB1: case 0xB2: case 0xB3: case 0xB4: case 0xB5: case 0xB7: // or a,reg
			{
				byte r = *registers[opcode & 0x07];
				AF.hi = util::logical_or(AF.hi, r, AF.flags);
				return 4;
			}

			case 0xB6: // or a,(HL)
			{
				AF.hi = util::logical_or(AF.hi, bus.Get(HL), AF.flags);
				return 8;
			}

			case 0xF6: // or a,n
			{
				AF.hi = util::logical_or(AF.hi, util::get_immediate_byte(PC, bus), AF.flags);
				return 8;
			}

			case 0xB8: case 0xB9: case 0xBA: case 0xBB: case 0xBC: case 0xBD: case 0xBF: // cp reg
			{
				byte r = *registers[opcode & 0x07];
				util::compare(AF.hi, r, AF.flags);
				return 4;
			}

			case 0xBE: // cp (HL)
			{
				util::compare(AF.hi, bus.Get(HL), AF.flags);
				return 8;
			}

			case 0xFE: // cp n
			{
				util::compare(AF.hi, util::get_immediate_byte(PC, bus), AF.flags);
				return 8;
			}

			case 0x04: case 0x0C: case 0x14: case 0x1C: case 0x24: case 0x2C: case 0x3C: // inc r
			{
				byte &r = *registers[opcode >> 3 & 0x07];
				r = util::increment_byte(r, AF.flags);
				return 4;
			}

			case 0x34: // inc (HL)
			{
				bus.Set(HL, util::increment_byte(bus.Get(HL), AF.flags));
				return 12;
			}

			case 0x05: case 0x0D: case 0x15: case 0x1D: case 0x25: case 0x2D: case 0x3D: // dec r
			{
				byte &r = *registers[opcode >> 3 & 0x07];
				r = util::decrement_byte(r, AF.flags);
				return 4;
			}

			case 0x35: // dec (HL)
			{
				bus.Set(HL, util::decrement_byte(bus.Get(HL), AF.flags));
				return 12;
			}

			case 0x27: // daa
			{
				AF.hi = util::bcd_correction(AF.hi, AF.flags);
				return 4;
			}

			case 0x37: // scf
			{
				AF.flags.cy = 1;
				AF.flags.h = 0;
				AF.flags.n = 0;
				return 4;
			}

			case 0x2F: // cpl
			{
				AF.hi = AF.hi ^ 0xFF;
				AF.flags.n = 1;
				AF.flags.h = 1;
				return 4;
			}

			case 0x3F: // ccf
			{
				AF.flags.cy = AF.flags.cy ^ 0x01;
				AF.flags.h = 0;
				AF.flags.n = 0;
				return 4;
			}

			// ================================================
			// ========== 16-bit arithmetic/logical ===========
			// ================================================

			case 0x03: case 0x13: case 0x23: case 0x33: // inc wreg
			{
				word &dst = *wregisters[opcode >> 4 & 0x03];
				dst = util::increment_word(dst);
				return 8;
			}

			case 0x0B: case 0x1B: case 0x2B: case 0x3B: // dec wreg
			{
				word &dst = *wregisters[opcode >> 4 & 0x03];
				dst = util::decrement_word(dst);
				return 8;
			}

			case 0x09: case 0x19: case 0x29: case 0x39: // add HL, wreg
			{
				word b = *wregisters[opcode >> 4 & 0x03];
				HL = util::add_words(HL, b, AF.flags);
				return 8;
			}

			case 0xE8: // add SP, r8
			{
				sbyte immediate = util::get_immediate_sbyte(PC, bus);
				SP = util::add_word_and_sbyte(SP, immediate, AF.flags);
				return 16;
			}

			// ================================================
			// =============== Jumps and Calls ================
			// ================================================

			case 0x18: // jp r8
			{
				sbyte offset = util::get_immediate_sbyte(PC, bus);
				PC += offset;
				return 12;
			}

			case 0x20: case 0x28: // jp NZ/Z r8
			{
				byte zero_check = opcode >> 3 & 0x01;
				sbyte immediate = util::get_immediate_sbyte(PC, bus);
				if (AF.flags.zf == zero_check) {
					PC += immediate;
					return 12;
				}
				return 8;
			}

			case 0x30: case 0x38: // jp NC/C r8
			{
				byte carry_check = opcode >> 3 & 0x01;
				sbyte immediate = util::get_immediate_sbyte(PC, bus);
				if (AF.flags.cy == carry_check) {
					PC += immediate;
					return 12;
				}
				return 8;
			}

			case 0xC0: case 0xC8: // ret NZ/Z
			{
				byte zero_check = opcode >> 3 & 0x01;
				if (AF.flags.zf == zero_check) {
					util::ret(PC, SP, bus);
					return 20;
				}
				return 8;
			}

			case 0xD0: case 0xD8: // ret NC/C
			{
				byte carry_check = opcode >> 3 & 0x01;
				if (AF.flags.cy == carry_check) {
					util::ret(PC, SP, bus);
					return 20;
				}
				return 8;
			}

			case 0xC2: case 0xCA: // JP NZ/Z, a16
			{
				byte zero_check = opcode >> 3 & 0x01;
				word immediate = util::get_immediate_word(PC, bus);
				if (AF.flags.zf == zero_check) {
					PC = immediate;
					return 16;
				}
				return 12;
			}

			case 0xD2: case 0xDA: // JP NC/C, a16
			{
				byte carry_check = opcode >> 3 & 0x01;
				word immediate = util::get_immediate_word(PC, bus);
				if (AF.flags.cy == carry_check) {
					PC = immediate;
					return 16;
				}
				return 12;
			}

			case 0xC3: // jp a16
			{
				word immediate = util::get_immediate_word(PC, bus);
				PC = immediate;
				return 16;
			}

			case 0xC4: case 0xCC: // call NZ/Z, a16
			{
				byte zero_check = opcode >> 3 & 0x01;
				word immediate = util::get_immediate_word(PC, bus);
				if (AF.flags.zf == zero_check) {
					util::call(immediate, SP, PC, bus);
					return 24;
				}
				return 12;
			}

			case 0xD4: case 0xDC: // call NC/C, a16
			{
				byte carry_check = opcode >> 3 & 0x01;
				word immediate = util::get_immediate_word(PC, bus);
				if (AF.flags.cy == carry_check) {
					util::call(immediate, SP, PC, bus);
					return 24;
				}
				return 12;
			}

			case 0xC7: case 0xD7: case 0xE7: case 0xF7: case 0xCF: case 0xDF: case 0xEF: case 0xFF: // rst
			{
				word addr = opcode & 0x30;
				byte offset = opcode & 0x08;
				util::call(addr + offset, SP, PC, bus);
				return 16;
			}

			case 0xC9: // ret
			{
				util::ret(PC, SP, bus);
				return 16;
			}

			case 0xD9: // reti
			{
				util::ret(PC, SP, bus);
				interrupt_master_enable_flag = true;
				return 16;
			}

			case 0xE9: // jp (HL)
			{
				PC = HL;
				return 4;
			}

			case 0xCD: // call addr
			{
				word addr = util::get_immediate_word(PC, bus);
				util::call(addr, SP, PC, bus);
				return 24;
			}

			// ================================================
			// =========== 8-bit Bit Instructions =============
			// ================================================

			case 0x07: // rlca
			{
				AF.hi = util::rotate_left_through_carry(AF.hi, AF.flags);
				AF.flags.zf = 0;
				return 4;
			}

			case 0x0F: // rrca
			{
				AF.hi = util::rotate_right_through_carry(AF.hi, AF.flags);
				AF.flags.zf = 0;
				return 4;
			}

			case 0x17: // rla
			{
				AF.hi = util::rotate_left(AF.hi, AF.flags);
				AF.flags.zf = 0;
				return 4;
			}

			case 0x1F: // rra
			{
				AF.hi = util::rotate_right(AF.hi, AF.flags);
				AF.flags.zf = 0;
				return 4;
			}

			// ================================================
			// =================== Control ====================
			// ================================================

			case 0x00: // noop
			{
				return 4;
			}

			case 0x76: case 0x10: // halt/stop
			{
				halted = true;
				return 4;
			}

			case 0xF3: case 0xFB: // DI/EI
			{
				interrupt_master_enable_flag = (opcode >> 3) & 0x01;
				return 4;
			}

			case 0xCB: // prefix CB
			{
				return HandleCBPrefixOpcode();
			}

			default:
			{
				std::cout << "Undefined opcode: " << std::hex << (int)opcode << " at " << PC << std::endl;
				system("pause");
				return 0;
			}
		}

		return true;
	}

	dword CPU::HandleInterrupts()
	{
		if (halted && interrupt_controller.interrupt_flags != 0)
			halted = false;

		if (!interrupt_master_enable_flag)
			return 0;

		interrupt_flags_t interrupt_enable = interrupt_controller.interrupt_enable;
		interrupt_flags_t &interrupt_flags = interrupt_controller.interrupt_flags;
		interrupt_flags_t interrupts_to_execute = interrupt_enable & interrupt_flags;

		if (!interrupts_to_execute)
			return 0;

		// check V-Blank
		if (interrupts_to_execute.vblank)
		{
			interrupt_flags.vblank = 0;
			interrupt_master_enable_flag = false;

			util::call(0x40, SP, PC, bus);
			return 5;
		}

		// check LCD STAT
		else if (interrupts_to_execute.lcd_stat)
		{
			interrupt_flags.lcd_stat = 0;
			interrupt_master_enable_flag = false;

			util::call(0x48, SP, PC, bus);
			return 5;
		}

		// check Timer
		else if (interrupts_to_execute.timer)
		{
			interrupt_flags.timer = 0;
			interrupt_master_enable_flag = false;

			util::call(0x50, SP, PC, bus);
			return 5;
		}

		// check Serial
		else if (interrupts_to_execute.serial)
		{
			interrupt_flags.serial = 0;
			interrupt_master_enable_flag = false;

			util::call(0x58, SP, PC, bus);
			return 5;
		}

		// check Joypad
		else if (interrupts_to_execute.joypad)
		{
			interrupt_flags.joypad = 0;
			interrupt_master_enable_flag = false;

			util::call(0x60, SP, PC, bus);
			return 5;
		}

		return 0;
	}

	void CPU::Start()
	{
		if (running)
			return;

		running = true;

		// start a thread to Step the cpu
		thread = std::thread([&] {
			while (running) {
				dword cycles = HandleInterrupts();

				if (halted)
					cycles += 4;
				else
					cycles += Step();

				// tick the internal timer
				timer.Tick(cycles);

				// tick LCD driver
				lcd.Tick(cycles);
			}
		});
	}

	void CPU::Stop()
	{
		if (!running)
			return;

		running = false;
		thread.join();
	}

	void CPU::Toggle()
	{
		if (running)
			Stop();
		else
			Start();
	}
}