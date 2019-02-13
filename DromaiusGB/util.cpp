#include "util.hpp"


namespace dromaiusgb
{
	namespace util
	{
		byte get_immediate_byte(word &pc, Bus &mem)
		{
			byte r = mem.Get(pc);
			pc += 1;

			return r;
		}

		word get_immediate_word(word &pc, Bus &mem)
		{
			byte l = get_immediate_byte(pc, mem);
			byte m = get_immediate_byte(pc, mem);
			
			word r = l | (m << 8);
			return r;
		}

		sbyte get_immediate_sbyte(word &pc, Bus &mem)
		{
			byte r = mem.Get(pc);
			pc += 1;

			return r;
		}

		word add_word_and_sbyte(word a, sbyte b, flags_t &flags)
		{
			word r;
			if (b < 0)
				r = sub_words(a, -b, flags);
			else
				r = add_words(a, b, flags);
			
			flags.zf = 0;
			flags.n = 0;
			return r;
		}

		word add_words(word a, word b, flags_t &flags)
		{
			byte zf = flags.zf;
			byte rlo = add_with_carry(a & 0xFF, b & 0xFF, 0, flags);
			byte rhi = add_with_carry(a >> 8 & 0xFF, b >> 8 & 0xFF, flags.cy, flags);
			flags.zf = zf;

			word r = (word)rlo | (word)(rhi << 8);
			return r;
		}

		word sub_words(word a, word b, flags_t &flags)
		{
			byte zf = flags.zf;
			byte rlo = sub_with_carry(a & 0xFF, b & 0xFF, 0, flags);
			byte rhi = sub_with_carry(a >> 8 & 0xFF, b >> 8 & 0xFF, flags.cy, flags);
			flags.zf = zf;

			word r = (word)rlo | (word)(rhi << 8);
			return r;
		}

		byte add_with_carry(byte a, byte b, byte carry, flags_t &flags)
		{
			word r = a + b + carry;
			
			flags.n = 0;
			flags.h = (((a & 0x0F) + (b & 0x0F) + carry) & 0x10) >> 4;
			flags.cy = (r > 0xFF);

			r = r & 0xFF;
			flags.zf = (r == 0);
			return (byte)r;
		}

		byte sub_with_carry(byte a, byte b, byte carry, flags_t &flags)
		{
			word r = a - b - carry;

			flags.n = 1;
			flags.h = (((a & 0x0F) - (b & 0x0F) - carry) & 0x10) >> 4;
			flags.cy = (r > 0xFF);

			r = r & 0xFF;
			flags.zf = (r == 0);
			return (byte)r;
		}

		byte logical_and(byte a, byte b, flags_t &flags)
		{
			byte r = a & b;
			
			flags.zf = (r == 0);
			flags.n = 0;
			flags.h = 1;
			flags.cy = 0;

			return r;
		}

		byte logical_xor(byte a, byte b, flags_t &flags)
		{
			byte r = a ^ b;

			flags.zf = (r == 0);
			flags.n = 0;
			flags.h = 0;
			flags.cy = 0;

			return r;
		}

		byte logical_or(byte a, byte b, flags_t &flags)
		{
			byte r = a | b;

			flags.zf = (r == 0);
			flags.n = 0;
			flags.h = 0;
			flags.cy = 0;

			return r;
		}

		void compare(byte a, byte b, flags_t &flags)
		{
			sub_with_carry(a, b, 0, flags);
		}

		byte increment_byte(byte w, flags_t &flags)
		{
			word r = w + 1;
			
			flags.n = 0;
			flags.h = ((r & 0x0F) < (w & 0x0F));

			w = (byte)r & 0xFF;
			flags.zf = (w == 0);

			return w;
		}

		word increment_word(word w)
		{
			return w + 1;
		}

		byte decrement_byte(byte w, flags_t &flags)
		{
			word r = w - 1;

			flags.n = 1;
			flags.h = ((r & 0x0F) > (w & 0x0F));

			w = (byte)r & 0xFF;
			flags.zf = (w == 0);

			return w;
		}

		word decrement_word(word w)
		{
			return w - 1;
		}

		void push(word w, word &sp, Bus &mem)
		{
			sp -= 2;
			mem.Set(sp, w & 0xFF);
			mem.Set(sp + 1, (w >> 8) & 0xFF);
		}

		word pop(word &sp, Bus &mem)
		{
			byte l = mem.Get(sp);
			byte m = mem.Get(sp + 1);
			sp += 2;

			word r = l | (m << 8);
			return r;
		}

		void call(word addr, word &sp, word &pc, Bus &mem)
		{
			push(pc, sp, mem);
			pc = addr;
		}

		void ret(word &pc, word &sp, Bus &mem)
		{
			pc = util::pop(sp, mem);
		}

		byte bcd_correction(byte a, flags_t &flags)
		{
			if (!flags.n) {
				if (flags.cy || a > 0x99) {
					a += 0x60; 
					flags.cy = 1;
				}

				if (flags.h || (a & 0x0F) > 0x09) {
					a += 0x06;
				}
			} else {
				if (flags.cy) {
					a -= 0x60;
				}

				if (flags.h) {
					a -= 0x06;
				}
			}

			flags.zf = (a == 0);
			flags.h = 0;

			return a;
		}

		void test_bit(byte v, byte bit, flags_t &flags)
		{
			flags.zf = ((v & (1 << bit)) == 0);
			flags.n = 0;
			flags.h = 1;
		}

		byte set_bit(byte v, byte bit)
		{
			v |= (1 << bit);
			return v;
		}

		byte reset_bit(byte v, byte bit)
		{
			v &= ~(1 << bit);
			return v;
		}

		byte rotate_left(byte v, flags_t &flags)
		{
			byte carry = flags.cy;
			flags.cy = (v >> 7) & 0x01;
			v <<= 1;
			v |= carry;

			flags.zf = (v == 0);
			flags.n = 0;
			flags.h = 0;

			return v;
		}

		byte rotate_right(byte v, flags_t &flags)
		{
			byte carry = flags.cy;
			flags.cy = v & 0x01;
			v >>= 1;
			v |= (carry << 7);

			flags.zf = (v == 0);
			flags.n = 0;
			flags.h = 0;

			return v;
		}

		byte rotate_left_through_carry(byte v, flags_t &flags)
		{
			flags.cy = (v >> 7) & 0x01;
			v <<= 1;
			v |= (flags.cy);

			flags.zf = (v == 0);
			flags.n = 0;
			flags.h = 0;

			return v;
		}

		byte rotate_right_through_carry(byte v, flags_t &flags)
		{
			flags.cy = v & 0x01;
			v >>= 1;
			v |= (flags.cy << 7);

			flags.zf = (v == 0);
			flags.n = 0;
			flags.h = 0;

			return v;
		}

		byte shift_left_arithmetic(byte v, flags_t &flags)
		{
			flags.cy = (v >> 0x07) & 0x01;
			v <<= 1;

			flags.zf = (v == 0);
			flags.n = 0;
			flags.h = 0;

			return v;
		}

		byte shift_right_arithmetic(byte v, flags_t &flags)
		{
			flags.cy = v & 0x01;
			v >>= 1;
			v |= (v & 0x40) << 1;

			flags.zf = (v == 0);
			flags.n = 0;
			flags.h = 0;

			return v;
		}

		byte shift_right_logical(byte v, flags_t &flags)
		{
			flags.cy = v & 0x01;
			v >>= 1;

			flags.zf = (v == 0);
			flags.n = 0;
			flags.h = 0;

			return v;
		}

		byte swap_nibbles(byte v, flags_t &flags)
		{
			v = ((v << 4) & 0xF0) | ((v >> 4) & 0x0F);

			flags.zf = (v == 0);
			flags.n = 0;
			flags.h = 0;
			flags.cy = 0;

			return v;
		}

		word mask_AF(word af)
		{
			return af & 0xFFF0;
		}
	}
}