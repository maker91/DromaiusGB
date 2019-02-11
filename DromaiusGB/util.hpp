#pragma once

#include "types.hpp"
#include "bus.hpp"
#include "cpu.hpp"

namespace dromaiusgb
{
	namespace util
	{
		byte get_immediate_byte(word &pc, Bus &mem);
		word get_immediate_word(word &pc, Bus &mem);
		sbyte get_immediate_sbyte(word &pc, Bus &mem);

		word add_word_and_sbyte(word a, sbyte b, flags_t &flags);
		word add_words(word a, word b, flags_t &flags);
		byte add_with_carry(byte a, byte b, byte carry, flags_t &flags);
		byte sub_with_carry(byte a, byte b, byte carry, flags_t &flags);
		byte logical_and(byte a, byte b, flags_t &flags);
		byte logical_xor(byte a, byte b, flags_t &flags);
		byte logical_or(byte a, byte b, flags_t &flags);
		void compare(byte a, byte b, flags_t &flags);

		byte increment_byte(byte w, flags_t &flags);
		word increment_word(word w);
		byte decrement_byte(byte w, flags_t &flags);
		word decrement_word(word w);

		void push(word w, word &sp, Bus &mem);
		word pop(word &sp, Bus &mem);
		void call(word addr, word &sp, word &pc, Bus &mem);
		void ret(word &pc, word &sp, Bus &mem);

		byte bcd_correction(byte a, flags_t &flags);

		void test_bit(byte v, byte bit, flags_t &flags);
		byte set_bit(byte v, byte bit);
		byte reset_bit(byte v, byte bit);
		byte rotate_left(byte v, flags_t &flags);
		byte rotate_right(byte v, flags_t &flags);
		byte rotate_left_through_carry(byte v, flags_t &flags);
		byte rotate_right_through_carry(byte v, flags_t &flags);
		byte shift_left_arithmetic(byte v, flags_t &flags);
		byte shift_right_arithmetic(byte v, flags_t &flags);
		byte shift_right_logical(byte v, flags_t &flags);
		byte swap_nibbles(byte v, flags_t &flags);

		word mask_AF(word af);
	}
}